#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// Include Windows headers before socket headers
#include <Windows.h>
// Include x64dbg SDK
#include "pluginsdk/bridgemain.h"
#include "pluginsdk/_plugins.h"
#include "pluginsdk/_scriptapi_module.h"
#include "pluginsdk/_scriptapi_memory.h"
#include "pluginsdk/_scriptapi_register.h"
#include "pluginsdk/_scriptapi_debug.h"
#include "pluginsdk/_scriptapi_assembler.h"
#include "pluginsdk/_scriptapi_comment.h"
#include "pluginsdk/_scriptapi_label.h"
#include "pluginsdk/_scriptapi_bookmark.h"
#include "pluginsdk/_scriptapi_function.h"
#include "pluginsdk/_scriptapi_argument.h"
#include "pluginsdk/_scriptapi_symbol.h"
#include "pluginsdk/_scriptapi_stack.h"
#include "pluginsdk/_scriptapi_pattern.h"
#include "pluginsdk/_scriptapi_flag.h"
#include "pluginsdk/_scriptapi_gui.h"
#include "pluginsdk/_scriptapi_misc.h"
#include "pluginsdk/_dbgfunctions.h"
#include "ServerLogic.h"
#include <iomanip>  // For std::setw and std::setfill
#include <cstdlib>
#include <cstdio>

// Socket includes - after Windows.h
#include <winsock2.h>
#include <ws2tcpip.h>

// Standard library includes
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <thread>
#include <algorithm>
#include <memory>
#include <fstream>
#include <cctype>
#include <cstring>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <queue>
// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

// Link against correct x64dbg library depending on architecture
#ifdef _WIN64
#pragma comment(lib, "x64dbg.lib")
#else
#pragma comment(lib, "x32dbg.lib")
#endif

// Architecture-aware formatting and register macros
#ifdef _WIN64
#define FMT_DUINT_HEX "0x%llx"
#define FMT_DUINT_DEC "%llu"
#define DUINT_CAST_PRINTF(v) (unsigned long long)(v)
#define DUSIZE_CAST_PRINTF(v) (unsigned long long)(v)
#define REG_IP Script::Register::RIP
#else
#define FMT_DUINT_HEX "0x%08X"
#define FMT_DUINT_DEC "%u"
#define DUINT_CAST_PRINTF(v) (unsigned int)(v)
#define DUSIZE_CAST_PRINTF(v) (unsigned int)(v)
#define REG_IP Script::Register::EIP
#endif

// Plugin information
#define PLUGIN_NAME "x64dbg HTTP Server"
#define PLUGIN_VERSION 1

// Default settings
#define DEFAULT_PORT 8888
static const size_t DEFAULT_MAX_REQUEST_SIZE = 1048576;
static const size_t DEFAULT_MAX_QUEUE_SIZE = 64;
static const int DEFAULT_WORKER_COUNT = 4;
static const size_t DEFAULT_MAX_EXEC_QUEUE = 32;
static const size_t DEFAULT_MAX_EXEC_JOBS = 128;
static const size_t DEFAULT_MAX_EXEC_OUTPUT = 256 * 1024;
static const unsigned long long DEFAULT_EXEC_TTL_MS = 5ULL * 60ULL * 1000ULL;
static const bool DEFAULT_EXEC_REDIRECT = true;
static const bool DEFAULT_EXEC_DIRECT = true;
static const bool DEFAULT_SAFE_MODE = false;
static const size_t DEFAULT_DBG_TASK_QUEUE = 256;

// Global variables
int g_pluginHandle;
HANDLE g_httpServerThread = NULL;
std::atomic<bool> g_httpServerRunning(false);
int g_httpPort = DEFAULT_PORT;
std::mutex g_httpMutex;
SOCKET g_serverSocket = INVALID_SOCKET;
std::mutex g_queueMutex;
std::condition_variable g_queueCv;
std::queue<SOCKET> g_socketQueue;
std::vector<std::thread> g_workerThreads;
std::mutex g_execCommandMutex;
std::mutex g_dbgApiMutex;
size_t g_maxRequestSize = DEFAULT_MAX_REQUEST_SIZE;
size_t g_maxQueueSize = DEFAULT_MAX_QUEUE_SIZE;
int g_workerCount = DEFAULT_WORKER_COUNT;
size_t g_maxExecQueue = DEFAULT_MAX_EXEC_QUEUE;
size_t g_maxExecJobs = DEFAULT_MAX_EXEC_JOBS;
size_t g_maxExecOutput = DEFAULT_MAX_EXEC_OUTPUT;
unsigned long long g_execJobTtlMs = DEFAULT_EXEC_TTL_MS;
bool g_execRedirectEnabled = DEFAULT_EXEC_REDIRECT;
bool g_execDirectEnabled = DEFAULT_EXEC_DIRECT;
bool g_safeModeEnabled = DEFAULT_SAFE_MODE;
size_t g_dbgTaskQueueMax = DEFAULT_DBG_TASK_QUEUE;

struct ExecCommandJob {
    std::string id;
    std::string cmd;
    std::string output;
    std::string error;
    std::string status;
    bool success = false;
    bool truncated = false;
    unsigned long long createdAtMs = 0;
    unsigned long long finishedAtMs = 0;
};

std::mutex g_execJobsMutex;
std::condition_variable g_execQueueCv;
std::queue<std::string> g_execQueue;
std::unordered_map<std::string, ExecCommandJob> g_execJobs;
std::thread g_execWorkerThread;
std::atomic<bool> g_execWorkerRunning(false);
std::atomic<unsigned long long> g_execJobCounter(0);

std::mutex g_dbgTaskMutex;
std::condition_variable g_dbgTaskCv;
std::queue<std::function<void()>> g_dbgTaskQueue;
std::thread g_dbgTaskThread;
std::atomic<bool> g_dbgTaskRunning(false);

// Run-until-user-code state
std::atomic<bool> g_runUntilUserCodeActive(false);
std::atomic<bool> g_runUntilUserCodeUserBpHit(false);
std::atomic<bool> g_runUntilUserCodeOwnsBp(false);
std::atomic<bool> g_runUntilUserCodeAutoResume(true);
duint g_runUntilUserCodeBp = 0;
std::atomic<bool> g_debugStepBusy(false);
std::atomic<bool> g_stepBatchActive(false);
std::atomic<int> g_stepBatchRemaining(0);

// Helpers
static bool isSystemModuleAddr(duint addr) {
    const DBGFUNCTIONS* dbg = DbgFunctions();
    if (!dbg || !dbg->ModBaseFromAddr || !dbg->ModGetParty) {
        return true;
    }
    duint base = dbg->ModBaseFromAddr(addr);
    if (!base) {
        return true;
    }
    return dbg->ModGetParty(base) == mod_system;
}

static bool parseIntMaybeHex(const std::string& text, unsigned long long& valueOut) {
    if (text.empty()) {
        return false;
    }
    int base = 10;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
    } else {
        for (char c : text) {
            if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                base = 16;
                break;
            }
        }
    }
    try {
        size_t idx = 0;
        valueOut = std::stoull(text, &idx, base);
        return idx == text.size();
    } catch (...) {
        return false;
    }
}

static bool parseAddressMaybeHex(const std::string& text, duint& valueOut) {
    unsigned long long parsed = 0;
    if (!parseIntMaybeHex(text, parsed)) {
        return false;
    }
    valueOut = static_cast<duint>(parsed);
    return true;
}

static bool sanitizeHexBytes(const std::string& input, std::string& output) {
    output.clear();
    output.reserve(input.size());
    bool sawPrefix = false;
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (std::isspace(c)) {
            continue;
        }
        if (!sawPrefix && output.empty() && c == '0' && i + 1 < input.size() &&
            (input[i + 1] == 'x' || input[i + 1] == 'X')) {
            sawPrefix = true;
            ++i;
            continue;
        }
        if (!std::isxdigit(c)) {
            return false;
        }
        output.push_back(static_cast<char>(c));
    }
    if (output.empty()) {
        return false;
    }
    return (output.size() % 2) == 0;
}

static bool readMemoryHex(duint addr, duint size, std::string& hexOut, duint& sizeRead, std::string& errorOut) {
    hexOut.clear();
    errorOut.clear();
    sizeRead = 0;

    if (!Script::Memory::IsValidPtr(addr)) {
        errorOut = "Invalid memory address";
        return false;
    }

    std::vector<unsigned char> buffer(size);
    duint initialRead = 0;
    bool fullRead = Script::Memory::Read(addr, buffer.data(), size, &initialRead);
    sizeRead = initialRead;

    if (!fullRead || sizeRead != size) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        size_t pageSize = sysInfo.dwPageSize ? sysInfo.dwPageSize : 0x1000;
        duint remaining = size;
        duint totalRead = 0;
        duint cursor = addr;
        while (remaining > 0) {
            size_t pageOffset = static_cast<size_t>(cursor % pageSize);
            duint chunk = static_cast<duint>(std::min<unsigned long long>(remaining, pageSize - pageOffset));
            duint chunkRead = 0;
            if (!Script::Memory::Read(cursor, buffer.data() + totalRead, chunk, &chunkRead) || chunkRead == 0) {
                break;
            }
            totalRead += chunkRead;
            cursor += chunkRead;
            remaining -= chunkRead;
            if (chunkRead < chunk) {
                break;
            }
        }

        sizeRead = totalRead;
    }

    if (sizeRead == 0) {
        unsigned int protect = Script::Memory::GetProtect(addr);
        std::stringstream err;
        err << "Failed to read memory (protect=0x" << std::hex << protect << ")";
        errorOut = err.str();
        return false;
    }

    std::stringstream ss;
    for (duint i = 0; i < sizeRead; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (int)buffer[i];
    }
    hexOut = ss.str();
    return true;
}

static size_t readSizeEnv(const char* name, size_t defaultValue, size_t minValue, size_t maxValue) {
    const char* raw = std::getenv(name);
    if (!raw || !*raw) {
        return defaultValue;
    }
    char* end = nullptr;
    unsigned long long parsed = std::strtoull(raw, &end, 10);
    if (end == raw) {
        return defaultValue;
    }
    size_t value = static_cast<size_t>(parsed);
    if (value < minValue) {
        value = minValue;
    }
    if (value > maxValue) {
        value = maxValue;
    }
    return value;
}

static int readIntEnv(const char* name, int defaultValue, int minValue, int maxValue) {
    const char* raw = std::getenv(name);
    if (!raw || !*raw) {
        return defaultValue;
    }
    char* end = nullptr;
    long parsed = std::strtol(raw, &end, 10);
    if (end == raw) {
        return defaultValue;
    }
    int value = static_cast<int>(parsed);
    if (value < minValue) {
        value = minValue;
    }
    if (value > maxValue) {
        value = maxValue;
    }
    return value;
}

static bool readBoolEnv(const char* name, bool defaultValue) {
    const char* raw = std::getenv(name);
    if (!raw || !*raw) {
        return defaultValue;
    }
    return ParseBool(raw, defaultValue);
}

static void loadServerConfigFromEnv() {
    g_workerCount = readIntEnv("X64DBG_HTTP_WORKERS", DEFAULT_WORKER_COUNT, 1, 32);
    g_maxQueueSize = readSizeEnv("X64DBG_HTTP_QUEUE", DEFAULT_MAX_QUEUE_SIZE, 8, 1024);
    g_maxRequestSize = readSizeEnv("X64DBG_HTTP_MAX_REQUEST", DEFAULT_MAX_REQUEST_SIZE, 8192, 8 * 1024 * 1024);
    g_maxExecQueue = readSizeEnv("X64DBG_EXEC_QUEUE", DEFAULT_MAX_EXEC_QUEUE, 1, 256);
    g_maxExecJobs = readSizeEnv("X64DBG_EXEC_MAX_JOBS", DEFAULT_MAX_EXEC_JOBS, 16, 2048);
    g_maxExecOutput = readSizeEnv("X64DBG_EXEC_MAX_OUTPUT", DEFAULT_MAX_EXEC_OUTPUT, 16 * 1024, 4 * 1024 * 1024);
    g_execJobTtlMs = readSizeEnv("X64DBG_EXEC_TTL_MS", DEFAULT_EXEC_TTL_MS, 10 * 1000, 60ULL * 60ULL * 1000ULL);
    g_execRedirectEnabled = readBoolEnv("X64DBG_EXEC_REDIRECT", DEFAULT_EXEC_REDIRECT);
    g_execDirectEnabled = readBoolEnv("X64DBG_EXEC_DIRECT", DEFAULT_EXEC_DIRECT);
    g_safeModeEnabled = readBoolEnv("X64DBG_SAFE_MODE", DEFAULT_SAFE_MODE);
    g_dbgTaskQueueMax = readSizeEnv("X64DBG_DBG_TASK_QUEUE", DEFAULT_DBG_TASK_QUEUE, 32, 2048);
}

static std::string jsonEscape(const std::string& input) {
    std::string out;
    out.reserve(input.size() + 16);
    for (unsigned char c : input) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

static void clearRunUntilUserCodeState() {
    g_runUntilUserCodeActive.store(false);
    g_runUntilUserCodeUserBpHit.store(false);
    g_runUntilUserCodeOwnsBp.store(false);
    g_runUntilUserCodeAutoResume.store(true);
    g_runUntilUserCodeBp = 0;
}

static void cancelRunUntilUserCodeIfActive() {
    if (!g_runUntilUserCodeActive.load()) {
        return;
    }
    if (g_runUntilUserCodeBp && g_runUntilUserCodeOwnsBp.load()) {
        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
        Script::Debug::DeleteBreakpoint(g_runUntilUserCodeBp);
    }
    clearRunUntilUserCodeState();
}

static bool queueDebugStepCommand(const char* cmd) {
    bool expected = false;
    if (!g_debugStepBusy.compare_exchange_strong(expected, true)) {
        return false;
    }
    bool submitted = DbgCmdExec(cmd);
    if (!submitted) {
        g_debugStepBusy.store(false);
    }
    return submitted;
}

static void stopStepBatch() {
    g_stepBatchActive.store(false);
    g_stepBatchRemaining.store(0);
    g_debugStepBusy.store(false);
}

static bool startStepBatch(int count) {
    if (count <= 0) {
        return false;
    }
    bool expected = false;
    if (!g_debugStepBusy.compare_exchange_strong(expected, true)) {
        return false;
    }
    g_stepBatchRemaining.store(count);
    g_stepBatchActive.store(true);
    bool submitted = DbgCmdExec("sto");
    if (!submitted) {
        stopStepBatch();
        return false;
    }
    return true;
}

static bool waitForDebuggerPause(unsigned int timeoutMs) {
    unsigned long long deadline = GetTickCount64() + timeoutMs;
    while (GetTickCount64() < deadline) {
        bool debugging = false;
        bool running = false;
        {
            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
            debugging = DbgIsDebugging();
            running = DbgIsRunning();
        }
        if (!debugging) {
            return false;
        }
        if (!running) {
            return true;
        }
        Sleep(10);
    }
    return false;
}

static bool startRunUntilUserCode(std::string& jsonOut, bool autoResume) {
    duint rip = Script::Register::Get(REG_IP);
    if (rip && !isSystemModuleAddr(rip)) {
        clearRunUntilUserCodeState();
        std::stringstream ss;
        ss << "{";
        ss << "\"already_user\":true,";
        ss << "\"rip\":\"0x" << std::hex << rip << "\"";
        ss << "}";
        jsonOut = ss.str();
        return true;
    }

    duint target = 0;
    const int maxFrames = 32;
    for (int i = 0; i < maxFrames; ++i) {
        duint candidate = Script::Stack::Peek(i * static_cast<int>(sizeof(duint)));
        if (!candidate) {
            continue;
        }
        if (!isSystemModuleAddr(candidate)) {
            target = candidate;
            break;
        }
    }

    bool bpSet = false;
    bool submitted = false;
    std::string mode = "command";

    if (target) {
        bpSet = Script::Debug::SetBreakpoint(target);
        g_runUntilUserCodeBp = target;
        g_runUntilUserCodeOwnsBp.store(bpSet);
        mode = "return_bp";
        submitted = DbgCmdExec("run");
    } else {
        g_runUntilUserCodeBp = 0;
        g_runUntilUserCodeOwnsBp.store(false);
        submitted = DbgCmdExec("RunToUserCode");
        if (!submitted) {
            duint entry = Script::Module::GetMainModuleEntry();
            if (entry) {
                bpSet = Script::Debug::SetBreakpoint(entry);
                g_runUntilUserCodeBp = entry;
                g_runUntilUserCodeOwnsBp.store(bpSet);
                mode = "entry_bp";
                submitted = DbgCmdExec("run");
            } else {
                jsonOut = "Failed to resolve main module entry";
                return false;
            }
        }
    }

    g_runUntilUserCodeUserBpHit.store(false);
    g_runUntilUserCodeActive.store(true);
    g_runUntilUserCodeAutoResume.store(autoResume);

    std::stringstream ss;
    ss << "{";
    ss << "\"mode\":\"" << mode << "\",";
    if (target) {
        ss << "\"target\":\"0x" << std::hex << target << "\",";
    } else if (g_runUntilUserCodeBp) {
        ss << "\"target\":\"0x" << std::hex << g_runUntilUserCodeBp << "\",";
    }
    ss << "\"breakpoint_set\":" << (bpSet ? "true" : "false") << ",";
    ss << "\"run_queued\":" << (submitted ? "true" : "false");
    ss << "}";

    jsonOut = ss.str();
    return submitted;
}

void sendHttpResponse(SOCKET clientSocket, int statusCode, const std::string& contentType, const std::string& responseBody);

static bool ensureDebugging(SOCKET clientSocket) {
    bool debugging = false;
    {
        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
        debugging = DbgIsDebugging();
    }
    if (!debugging) {
        sendHttpResponse(clientSocket, 409, "text/plain", "No active debug session");
        return false;
    }
    return true;
}

static bool ensureConfirmed(SOCKET clientSocket,
                            const std::unordered_map<std::string, std::string>& queryParams,
                            const char* action) {
    if (!g_safeModeEnabled) {
        return true;
    }
    std::string confirmStr;
    auto it = queryParams.find("confirm");
    if (it != queryParams.end()) {
        confirmStr = it->second;
    }
    bool confirmed = ParseBool(confirmStr, false);
    if (!confirmed) {
        std::string msg = "Confirmation required";
        if (action && *action) {
            msg += " for ";
            msg += action;
        }
        sendHttpResponse(clientSocket, 403, "text/plain", msg);
        return false;
    }
    return true;
}

static bool isExecCommandSafeReadOnly(const std::string& cmd) {
    std::string trimmed = cmd;
    trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
                                               [](unsigned char c) { return !std::isspace(c); }));
    if (trimmed.empty()) {
        return false;
    }
    std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(), ::tolower);
    std::istringstream iss(trimmed);
    std::string first;
    iss >> first;
    if (first.empty()) {
        return false;
    }
    static const std::unordered_set<std::string> kReadOnly = {
        "help", "?", "eip", "rip", "cip", "csp", "cax", "cbp",
        "rtr", "script", "sc", "getcmdline", "getthreadid",
        "mod", "modinfo", "modbase", "modlist", "modulelist",
        "sym", "symfind", "symaddr", "symname",
        "eval", "print", "printf", "dump", "db", "dw", "dd", "dq",
        "disasm", "dis", "disasmaddr",
        "ref", "refget", "reffind",
        "memlist", "memmap", "meminfo", "memdump",
        "commentlist", "labelist", "bookmarklist",
        "stack", "threads", "threadlist",
        "bp", "bplist"
    };
    return kReadOnly.find(first) != kReadOnly.end();
}

static std::string makeExecJobId() {
    unsigned long long counter = g_execJobCounter.fetch_add(1);
    unsigned long long tick = GetTickCount64();
    std::stringstream ss;
    ss << std::hex << tick << "_" << counter;
    return ss.str();
}

static void pruneExecJobsLocked(unsigned long long nowMs);

static bool enqueueExecCommandJob(const std::string& cmd, std::string& jobIdOut, std::string& errorOut) {
    std::lock_guard<std::mutex> lock(g_execJobsMutex);
    unsigned long long nowMs = GetTickCount64();
    pruneExecJobsLocked(nowMs);
    if (g_execQueue.size() >= g_maxExecQueue) {
        errorOut = "Exec queue full";
        return false;
    }
    if (g_execJobs.size() >= g_maxExecJobs) {
        errorOut = "Exec job limit reached";
        return false;
    }
    std::string jobId = makeExecJobId();
    ExecCommandJob job;
    job.id = jobId;
    job.cmd = cmd;
    job.status = "queued";
    job.createdAtMs = nowMs;
    g_execJobs.emplace(jobId, job);
    g_execQueue.push(jobId);
    g_execQueueCv.notify_one();
    jobIdOut = jobId;
    return true;
}

static void execCommandWorkerLoop() {
    while (g_execWorkerRunning.load()) {
        std::string jobId;
        {
            std::unique_lock<std::mutex> lock(g_execJobsMutex);
            g_execQueueCv.wait(lock, [] {
                return !g_execWorkerRunning.load() || !g_execQueue.empty();
            });
            if (!g_execWorkerRunning.load() && g_execQueue.empty()) {
                return;
            }
            if (!g_execQueue.empty()) {
                jobId = g_execQueue.front();
                g_execQueue.pop();
                auto it = g_execJobs.find(jobId);
                if (it != g_execJobs.end()) {
                    it->second.status = "running";
                }
            }
        }

        if (jobId.empty()) {
            continue;
        }

        std::string cmd;
        {
            std::lock_guard<std::mutex> lock(g_execJobsMutex);
            auto it = g_execJobs.find(jobId);
            if (it == g_execJobs.end()) {
                continue;
            }
            cmd = it->second.cmd;
        }

        std::string output;
        std::string error;
        bool success = false;

        if (!g_execRedirectEnabled) {
            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
            if (g_execDirectEnabled) {
                success = DbgCmdExecDirect(cmd.c_str());
            } else {
                success = DbgCmdExec(cmd.c_str());
            }
            if (!g_execDirectEnabled && output.empty()) {
                output = success ? "Command queued (direct disabled)" : "Failed to queue command";
            }
        } else {
            char tempPath[MAX_PATH];
            GetTempPathA(MAX_PATH, tempPath);
            char tempFile[MAX_PATH];
            if (!GetTempFileNameA(tempPath, "xdbg", 0, tempFile)) {
                error = "Failed to create temp file";
            } else {
                std::string logFile = tempFile;
                {
                    std::lock_guard<std::mutex> execLock(g_execCommandMutex);
                    std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                    GuiLogRedirect(logFile.c_str());
                    if (g_execDirectEnabled) {
                        success = DbgCmdExecDirect(cmd.c_str());
                    } else {
                        success = DbgCmdExec(cmd.c_str());
                    }
                    GuiFlushLog();
                    Sleep(200);
                    GuiLogRedirectStop();
                    Sleep(50);
                }

                std::ifstream file(logFile);
                if (file) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    output = buffer.str();
                }
                DeleteFileA(logFile.c_str());
            }
        }

        if (output.empty()) {
            output = success ? "Command executed (no output)" : "Command failed";
        }
        bool truncated = false;
        if (output.size() > g_maxExecOutput) {
            truncated = true;
            const char* suffix = "\n...(truncated)";
            size_t suffixLen = std::strlen(suffix);
            size_t keep = g_maxExecOutput > suffixLen ? g_maxExecOutput - suffixLen : 0;
            output.resize(keep);
            output += suffix;
        }

        {
            std::lock_guard<std::mutex> lock(g_execJobsMutex);
            auto it = g_execJobs.find(jobId);
            if (it == g_execJobs.end()) {
                continue;
            }
            it->second.output = output;
            it->second.error = error;
            it->second.success = success;
            it->second.status = success ? "done" : "failed";
            it->second.truncated = truncated;
            it->second.finishedAtMs = GetTickCount64();
            pruneExecJobsLocked(it->second.finishedAtMs);
        }
    }
}

static void startExecCommandWorker() {
    if (g_execWorkerRunning.load()) {
        return;
    }
    g_execWorkerRunning.store(true);
    g_execWorkerThread = std::thread(execCommandWorkerLoop);
}

static void stopExecCommandWorker() {
    if (!g_execWorkerRunning.load()) {
        return;
    }
    g_execWorkerRunning.store(false);
    g_execQueueCv.notify_all();
    if (g_execWorkerThread.joinable()) {
        g_execWorkerThread.join();
    }
    std::lock_guard<std::mutex> lock(g_execJobsMutex);
    while (!g_execQueue.empty()) {
        g_execQueue.pop();
    }
    g_execJobs.clear();
}

static void dbgTaskWorkerLoop() {
    while (g_dbgTaskRunning.load()) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(g_dbgTaskMutex);
            g_dbgTaskCv.wait(lock, [] {
                return !g_dbgTaskRunning.load() || !g_dbgTaskQueue.empty();
            });
            if (!g_dbgTaskRunning.load() && g_dbgTaskQueue.empty()) {
                return;
            }
            if (!g_dbgTaskQueue.empty()) {
                task = std::move(g_dbgTaskQueue.front());
                g_dbgTaskQueue.pop();
            }
        }

        if (task) {
            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
            task();
        }
    }
}

static void startDbgTaskWorker() {
    if (g_dbgTaskRunning.load()) {
        return;
    }
    g_dbgTaskRunning.store(true);
    g_dbgTaskThread = std::thread(dbgTaskWorkerLoop);
}

static void stopDbgTaskWorker() {
    if (!g_dbgTaskRunning.load()) {
        return;
    }
    g_dbgTaskRunning.store(false);
    g_dbgTaskCv.notify_all();
    if (g_dbgTaskThread.joinable()) {
        g_dbgTaskThread.join();
    }
    std::lock_guard<std::mutex> lock(g_dbgTaskMutex);
    while (!g_dbgTaskQueue.empty()) {
        g_dbgTaskQueue.pop();
    }
}

static bool enqueueDbgTask(const std::function<void()>& task) {
    if (!task) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_dbgTaskMutex);
    if (g_dbgTaskQueue.size() >= g_dbgTaskQueueMax) {
        _plugin_logputs("Dbg task queue full; dropping task");
        return false;
    }
    g_dbgTaskQueue.push(task);
    g_dbgTaskCv.notify_one();
    return true;
}

static void pruneExecJobsLocked(unsigned long long nowMs) {
    if (g_execJobs.empty()) {
        return;
    }
    for (auto it = g_execJobs.begin(); it != g_execJobs.end(); ) {
        const ExecCommandJob& job = it->second;
        if (job.finishedAtMs > 0 && nowMs > job.finishedAtMs &&
            nowMs - job.finishedAtMs > g_execJobTtlMs) {
            it = g_execJobs.erase(it);
        } else {
            ++it;
        }
    }
}

static bool addStepBatch(int count) {
    if (count <= 0) {
        return false;
    }
    if (g_stepBatchActive.load()) {
        g_stepBatchRemaining.fetch_add(count);
        return true;
    }
    if (g_debugStepBusy.load()) {
        bool expected = false;
        if (g_stepBatchActive.compare_exchange_strong(expected, true)) {
            g_stepBatchRemaining.store(count + 1);
        } else {
            g_stepBatchRemaining.fetch_add(count);
        }
        return true;
    }
    return startStepBatch(count);
}

// Forward declarations
bool startHttpServer();
void stopHttpServer();
DWORD WINAPI HttpServerThread(LPVOID lpParam);
void handleClientConnection(SOCKET clientSocket);
void workerThreadLoop();
std::string readHttpRequest(SOCKET clientSocket, bool* tooLarge);
void sendHttpResponse(SOCKET clientSocket, int statusCode, const std::string& contentType, const std::string& responseBody);
void parseHttpRequest(const std::string& request, std::string& method, std::string& path, std::string& query, std::string& body);

// Command callback declarations
bool cbEnableHttpServer(int argc, char* argv[]);
bool cbSetHttpPort(int argc, char* argv[]);
void registerCommands();

// Debug callbacks
void cbSystemBreakpoint(CBTYPE cbType, void* callbackInfo);
void cbBreakpoint(CBTYPE cbType, void* callbackInfo);
void cbPauseDebug(CBTYPE cbType, void* callbackInfo);
void cbStopDebug(CBTYPE cbType, void* callbackInfo);
void cbStepped(CBTYPE cbType, void* callbackInfo);

//=============================================================================
// Plugin Interface Implementation
//============================================================================


// Initialize the plugin
bool pluginInit(PLUG_INITSTRUCT* initStruct) {
    initStruct->pluginVersion = PLUGIN_VERSION;
    initStruct->sdkVersion = PLUG_SDKVERSION;
    strncpy_s(initStruct->pluginName, PLUGIN_NAME, _TRUNCATE);
    g_pluginHandle = initStruct->pluginHandle;
    
    _plugin_logputs("x64dbg HTTP Server plugin loading...");
    
    // Register commands
    registerCommands();

    // Register callbacks for run-until-user-code behavior
    _plugin_registercallback(g_pluginHandle, CB_SYSTEMBREAKPOINT, cbSystemBreakpoint);
    _plugin_registercallback(g_pluginHandle, CB_BREAKPOINT, cbBreakpoint);
    _plugin_registercallback(g_pluginHandle, CB_PAUSEDEBUG, cbPauseDebug);
    _plugin_registercallback(g_pluginHandle, CB_STOPDEBUG, cbStopDebug);
    _plugin_registercallback(g_pluginHandle, CB_STEPPED, cbStepped);

    // Start the HTTP server
    if (startHttpServer()) {
        _plugin_logprintf("x64dbg HTTP Server started on port %d\n", g_httpPort);
    } else {
        _plugin_logputs("Failed to start HTTP server!");
    }
    
    _plugin_logputs("x64dbg HTTP Server plugin loaded!");
    return true;
}

// Stop the plugin
void pluginStop() {
    _plugin_logputs("Stopping x64dbg HTTP Server...");
    stopHttpServer();
    _plugin_logputs("x64dbg HTTP Server stopped.");
}

// Plugin setup
bool pluginSetup() {
    return true;
}

// Plugin exports
extern "C" __declspec(dllexport) bool pluginit(PLUG_INITSTRUCT* initStruct) {
    return pluginInit(initStruct);
}

extern "C" __declspec(dllexport) void plugstop() {
    pluginStop();
}

extern "C" __declspec(dllexport) void plugsetup(PLUG_SETUPSTRUCT* setupStruct) {
    pluginSetup();
}

//=============================================================================
// HTTP Server Implementation
//=============================================================================

// Start the HTTP server
bool startHttpServer() {
    std::lock_guard<std::mutex> lock(g_httpMutex);
    
    // Stop existing server if running
    if (g_httpServerRunning.load()) {
        stopHttpServer();
    }

    loadServerConfigFromEnv();
    startExecCommandWorker();
    startDbgTaskWorker();
    
    // Create and start the server thread
    g_httpServerRunning.store(true);
    g_httpServerThread = CreateThread(NULL, 0, HttpServerThread, NULL, 0, NULL);
    if (g_httpServerThread == NULL) {
        _plugin_logputs("Failed to create HTTP server thread");
        g_httpServerRunning.store(false);
        stopExecCommandWorker();
        stopDbgTaskWorker();
        return false;
    }
    
    return true;
}

// Stop the HTTP server
void stopHttpServer() {
    std::lock_guard<std::mutex> lock(g_httpMutex);
    
    if (g_httpServerRunning.load()) {
        g_httpServerRunning.store(false);
        g_queueCv.notify_all();
        stopExecCommandWorker();
        stopDbgTaskWorker();
        
        // Close the server socket to unblock any accept calls
        if (g_serverSocket != INVALID_SOCKET) {
            shutdown(g_serverSocket, SD_BOTH);
            closesocket(g_serverSocket);
            g_serverSocket = INVALID_SOCKET;
        }
        
        // Wait for the thread to exit
        if (g_httpServerThread != NULL) {
            WaitForSingleObject(g_httpServerThread, 1000);
            CloseHandle(g_httpServerThread);
            g_httpServerThread = NULL;
        }
    }
}



// HTTP server thread function using standard Winsock
DWORD WINAPI HttpServerThread(LPVOID lpParam) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        _plugin_logprintf("WSAStartup failed with error: %d\n", result);
        g_httpServerRunning.store(false);
        stopExecCommandWorker();
        stopDbgTaskWorker();
        return 1;
    }
    
    // Create a socket for the server
    g_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_serverSocket == INVALID_SOCKET) {
        _plugin_logprintf("Failed to create socket, error: %d\n", WSAGetLastError());
        WSACleanup();
        g_httpServerRunning.store(false);
        stopExecCommandWorker();
        stopDbgTaskWorker();
        return 1;
    }
    
    // Setup the server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
    serverAddr.sin_port = htons((u_short)g_httpPort);
    
    // Bind the socket
    if (bind(g_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        _plugin_logprintf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(g_serverSocket);
        WSACleanup();
        g_httpServerRunning.store(false);
        stopExecCommandWorker();
        stopDbgTaskWorker();
        return 1;
    }
    
    // Listen for incoming connections
    if (listen(g_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        _plugin_logprintf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(g_serverSocket);
        WSACleanup();
        g_httpServerRunning.store(false);
        stopExecCommandWorker();
        stopDbgTaskWorker();
        return 1;
    }
    
    _plugin_logprintf("HTTP server started at http://localhost:%d/\n", g_httpPort);
    _plugin_logprintf("HTTP server config: workers=%d queue=%zu max_request=%zu exec_queue=%zu exec_jobs=%zu exec_output=%zu exec_ttl_ms=%llu exec_redirect=%s exec_direct=%s safe_mode=%s dbg_task_queue=%zu\n",
                      g_workerCount, g_maxQueueSize, g_maxRequestSize, g_maxExecQueue,
                      g_maxExecJobs, g_maxExecOutput, g_execJobTtlMs,
                      g_execRedirectEnabled ? "true" : "false",
                      g_execDirectEnabled ? "true" : "false",
                      g_safeModeEnabled ? "true" : "false",
                      g_dbgTaskQueueMax);
    
    // Start worker threads
    g_workerThreads.clear();
    g_workerThreads.reserve(static_cast<size_t>(g_workerCount));
    for (int i = 0; i < g_workerCount; ++i) {
        g_workerThreads.emplace_back(workerThreadLoop);
    }

    // Main server loop
    while (g_httpServerRunning.load()) {
        // Accept a client connection
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(g_serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSocket == INVALID_SOCKET) {
            if (!g_httpServerRunning.load()) {
                break;
            }
            _plugin_logprintf("Accept failed with error: %d\n", WSAGetLastError());
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(g_queueMutex);
            if (g_socketQueue.size() >= g_maxQueueSize) {
                sendHttpResponse(clientSocket, 429, "text/plain", "Server busy");
                closesocket(clientSocket);
                continue;
            }
            g_socketQueue.push(clientSocket);
        }
        g_queueCv.notify_one();
    }

    // Clean up
    if (g_serverSocket != INVALID_SOCKET) {
        closesocket(g_serverSocket);
        g_serverSocket = INVALID_SOCKET;
    }

    g_queueCv.notify_all();
    for (auto& t : g_workerThreads) {
        if (t.joinable()) {
            t.join();
        }
    }
    g_workerThreads.clear();

    WSACleanup();
    return 0;
}

void workerThreadLoop() {
    while (g_httpServerRunning.load()) {
        SOCKET clientSocket = INVALID_SOCKET;
        {
            std::unique_lock<std::mutex> lock(g_queueMutex);
            g_queueCv.wait(lock, [] {
                return !g_httpServerRunning.load() || !g_socketQueue.empty();
            });
            if (!g_httpServerRunning.load() && g_socketQueue.empty()) {
                return;
            }
            if (!g_socketQueue.empty()) {
                clientSocket = g_socketQueue.front();
                g_socketQueue.pop();
            }
        }

        if (clientSocket != INVALID_SOCKET) {
            handleClientConnection(clientSocket);
        }
    }
}

void handleClientConnection(SOCKET clientSocket) {
    do {
        // Read the HTTP request
        bool tooLarge = false;
        std::string requestData = readHttpRequest(clientSocket, &tooLarge);
        if (tooLarge) {
            _plugin_logputs("HTTP request rejected: too large");
            sendHttpResponse(clientSocket, 413, "text/plain", "Request too large");
            break;
        }
        
        if (!requestData.empty()) {
            // Parse the HTTP request
            std::string method, path, query, body;
            parseHttpRequest(requestData, method, path, query, body);
            
            _plugin_logprintf("HTTP Request: %s %s\n", method.c_str(), path.c_str());
            
            // Parse query parameters
            std::unordered_map<std::string, std::string> queryParams = ParseQueryParams(query);

            // Handle different endpoints
            try {
                // Unified command execution endpoint
                if (path == "/ExecCommand") {
                    std::string cmd = queryParams["cmd"];
                    if (cmd.empty() && !body.empty()) {
                        cmd = body;
                    }

                    std::string waitStr = queryParams["wait"];
                    bool wait = ParseBool(waitStr, false);
                    
                    if (cmd.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing command parameter");
                        continue;
                    }

                    if (g_safeModeEnabled && !isExecCommandSafeReadOnly(cmd)) {
                        if (!ensureConfirmed(clientSocket, queryParams, "ExecCommand")) {
                            continue;
                        }
                    }

                    std::string cmdLower = cmd;
                    std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);
                    if (cmdLower == "runtousercode" || cmdLower == "runuser" || cmdLower == "runto user code") {
                        std::string jsonOut;
                        bool ok = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            ok = startRunUntilUserCode(jsonOut, true);
                        }
                        sendHttpResponse(clientSocket, ok ? 200 : 500, "application/json", jsonOut);
                        continue;
                    }

                    if (!wait) {
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec(cmd.c_str());
                        }
                        sendHttpResponse(clientSocket, submitted ? 200 : 500, "text/plain",
                            submitted ? "Command queued" : "Failed to queue command");
                        continue;
                    }

                    std::string jobId;
                    std::string error;
                    bool queued = enqueueExecCommandJob(cmd, jobId, error);
                    if (!queued) {
                        sendHttpResponse(clientSocket, 503, "text/plain",
                            error.empty() ? "Failed to queue command" : error);
                        continue;
                    }

                    std::stringstream ss;
                    ss << "{";
                    ss << "\"job_id\":\"" << jsonEscape(jobId) << "\",";
                    ss << "\"queued\":true";
                    ss << "}";
                    sendHttpResponse(clientSocket, 202, "application/json", ss.str());
                }
                else if (path == "/ExecCommand/Result") {
                    std::string jobId = queryParams["id"];
                    if (jobId.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing id parameter");
                        continue;
                    }
                    bool consume = ParseBool(queryParams["consume"], false);

                    ExecCommandJob job;
                    bool found = false;
                    {
                        std::lock_guard<std::mutex> lock(g_execJobsMutex);
                        pruneExecJobsLocked(GetTickCount64());
                        auto it = g_execJobs.find(jobId);
                        if (it != g_execJobs.end()) {
                            job = it->second;
                            found = true;
                            if (consume && (job.status == "done" || job.status == "failed")) {
                                g_execJobs.erase(it);
                            }
                        }
                    }

                    if (!found) {
                        sendHttpResponse(clientSocket, 404, "text/plain", "Unknown job id");
                        continue;
                    }

                    std::stringstream ss;
                    ss << "{";
                    ss << "\"job_id\":\"" << jsonEscape(job.id) << "\",";
                    ss << "\"status\":\"" << jsonEscape(job.status) << "\",";
                    ss << "\"success\":" << (job.success ? "true" : "false");
                    if (!job.output.empty()) {
                        ss << ",\"output\":\"" << jsonEscape(job.output) << "\"";
                    }
                    if (!job.error.empty()) {
                        ss << ",\"error\":\"" << jsonEscape(job.error) << "\"";
                    }
                    if (job.truncated) {
                        ss << ",\"truncated\":true";
                    }
                    ss << "}";
                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                else if (path == "/IsDebugActive") {
                    std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                    bool isRunning = DbgIsRunning();
                    _plugin_logprintf("DbgIsRunning() called, result: %s\n", isRunning ? "true" : "false");
                    std::stringstream ss;
                    ss << "{\"isRunning\":" << (isRunning ? "true" : "false") << "}";
                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                else if (path == "/Is_Debugging") {
                    std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                    bool isDebugging = DbgIsDebugging();
                    _plugin_logprintf("DbgIsDebugging() called, result: %s\n", isDebugging ? "true" : "false");
                    std::stringstream ss;
                    ss << "{\"isDebugging\":" << (isDebugging ? "true" : "false") << "}";
                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                // =============================================================================
                // REGISTER API ENDPOINTS
                // =============================================================================
                else if (path == "/Register/Get") {
                    std::string regName = queryParams["register"];
                    if (regName.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing register parameter");
                        continue;
                    }
                    
                    // Convert register name to enum (simplified mapping)
                    Script::Register::RegisterEnum reg;
                    if (regName == "EAX" || regName == "eax") reg = Script::Register::EAX;
                    else if (regName == "EBX" || regName == "ebx") reg = Script::Register::EBX;
                    else if (regName == "ECX" || regName == "ecx") reg = Script::Register::ECX;
                    else if (regName == "EDX" || regName == "edx") reg = Script::Register::EDX;
                    else if (regName == "ESI" || regName == "esi") reg = Script::Register::ESI;
                    else if (regName == "EDI" || regName == "edi") reg = Script::Register::EDI;
                    else if (regName == "EBP" || regName == "ebp") reg = Script::Register::EBP;
                    else if (regName == "ESP" || regName == "esp") reg = Script::Register::ESP;
                    else if (regName == "EIP" || regName == "eip") reg = Script::Register::EIP;
#ifdef _WIN64
                    else if (regName == "RAX" || regName == "rax") reg = Script::Register::RAX;
                    else if (regName == "RBX" || regName == "rbx") reg = Script::Register::RBX;
                    else if (regName == "RCX" || regName == "rcx") reg = Script::Register::RCX;
                    else if (regName == "RDX" || regName == "rdx") reg = Script::Register::RDX;
                    else if (regName == "RSI" || regName == "rsi") reg = Script::Register::RSI;
                    else if (regName == "RDI" || regName == "rdi") reg = Script::Register::RDI;
                    else if (regName == "RBP" || regName == "rbp") reg = Script::Register::RBP;
                    else if (regName == "RSP" || regName == "rsp") reg = Script::Register::RSP;
                    else if (regName == "RIP" || regName == "rip") {
#ifdef _WIN64
                        reg = Script::Register::RIP;
#else
                        // On x86, map RIP queries to EIP for compatibility
                        reg = Script::Register::EIP;
#endif
                    }
                    else if (regName == "R8" || regName == "r8") reg = Script::Register::R8;
                    else if (regName == "R9" || regName == "r9") reg = Script::Register::R9;
                    else if (regName == "R10" || regName == "r10") reg = Script::Register::R10;
                    else if (regName == "R11" || regName == "r11") reg = Script::Register::R11;
                    else if (regName == "R12" || regName == "r12") reg = Script::Register::R12;
                    else if (regName == "R13" || regName == "r13") reg = Script::Register::R13;
                    else if (regName == "R14" || regName == "r14") reg = Script::Register::R14;
                    else if (regName == "R15" || regName == "r15") reg = Script::Register::R15;
#endif
                    else {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Unknown register");
                        continue;
                    }
                    
                    duint value = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        value = Script::Register::Get(reg);
                    }
                    std::stringstream ss;
                    ss << "0x" << std::hex << value;
                    sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                }
                else if (path == "/Register/Set") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Register/Set")) {
                        continue;
                    }
                    std::string regName = queryParams["register"];
                    std::string valueStr = queryParams["value"];
                    if (regName.empty() || valueStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing register or value parameter");
                        continue;
                    }
                    
                    // Convert register name to enum (same mapping as above)
                    Script::Register::RegisterEnum reg;
                    if (regName == "EAX" || regName == "eax") reg = Script::Register::EAX;
                    else if (regName == "EBX" || regName == "ebx") reg = Script::Register::EBX;
                    else if (regName == "ECX" || regName == "ecx") reg = Script::Register::ECX;
                    else if (regName == "EDX" || regName == "edx") reg = Script::Register::EDX;
                    else if (regName == "ESI" || regName == "esi") reg = Script::Register::ESI;
                    else if (regName == "EDI" || regName == "edi") reg = Script::Register::EDI;
                    else if (regName == "EBP" || regName == "ebp") reg = Script::Register::EBP;
                    else if (regName == "ESP" || regName == "esp") reg = Script::Register::ESP;
                    else if (regName == "EIP" || regName == "eip") reg = Script::Register::EIP;
#ifdef _WIN64
                    else if (regName == "RAX" || regName == "rax") reg = Script::Register::RAX;
                    else if (regName == "RBX" || regName == "rbx") reg = Script::Register::RBX;
                    else if (regName == "RCX" || regName == "rcx") reg = Script::Register::RCX;
                    else if (regName == "RDX" || regName == "rdx") reg = Script::Register::RDX;
                    else if (regName == "RSI" || regName == "rsi") reg = Script::Register::RSI;
                    else if (regName == "RDI" || regName == "rdi") reg = Script::Register::RDI;
                    else if (regName == "RBP" || regName == "rbp") reg = Script::Register::RBP;
                    else if (regName == "RSP" || regName == "rsp") reg = Script::Register::RSP;
                    else if (regName == "RIP" || regName == "rip") {
#ifdef _WIN64
                        reg = Script::Register::RIP;
#else
                        reg = Script::Register::EIP;
#endif
                    }
                    else if (regName == "R8" || regName == "r8") reg = Script::Register::R8;
                    else if (regName == "R9" || regName == "r9") reg = Script::Register::R9;
                    else if (regName == "R10" || regName == "r10") reg = Script::Register::R10;
                    else if (regName == "R11" || regName == "r11") reg = Script::Register::R11;
                    else if (regName == "R12" || regName == "r12") reg = Script::Register::R12;
                    else if (regName == "R13" || regName == "r13") reg = Script::Register::R13;
                    else if (regName == "R14" || regName == "r14") reg = Script::Register::R14;
                    else if (regName == "R15" || regName == "r15") reg = Script::Register::R15;
#endif
                    else {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Unknown register");
                        continue;
                    }
                    
                    duint value = 0;
                    if (!parseAddressMaybeHex(valueStr, value)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid value format");
                        continue;
                    }
                    
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Register::Set(reg, value);
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Register set successfully" : "Failed to set register");
                }
                else if (path == "/Memory/Read") {
                    std::string addrStr = queryParams["addr"];
                    std::string sizeStr = queryParams["size"];
                    
                    if (addrStr.empty() || sizeStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address or size");
                        continue;
                    }
                    
                    duint addr = 0;
                    duint size = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address or size format");
                        continue;
                    }

                    unsigned long long sizeParsed = 0;
                    if (!parseIntMaybeHex(sizeStr, sizeParsed)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address or size format");
                        continue;
                    }
                    size = static_cast<duint>(sizeParsed);
                    
                    if (size > 1024 * 1024) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Size too large");
                        continue;
                    }

                    std::string hexOut;
                    std::string errorOut;
                    duint sizeRead = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        if (!readMemoryHex(addr, size, hexOut, sizeRead, errorOut)) {
                            if (errorOut == "Invalid memory address") {
                                sendHttpResponse(clientSocket, 400, "text/plain", errorOut);
                            } else {
                                sendHttpResponse(clientSocket, 500, "text/plain", errorOut);
                            }
                            continue;
                        }
                    }

                    int status = (sizeRead == size) ? 200 : 206;
                    sendHttpResponse(clientSocket, status, "text/plain", hexOut);
                }
                else if (path == "/Memory/ReadDetailed") {
                    std::string addrStr = queryParams["addr"];
                    std::string sizeStr = queryParams["size"];
                    
                    if (addrStr.empty() || sizeStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "application/json", "{\"error\":\"Missing address or size\"}");
                        continue;
                    }
                    
                    duint addr = 0;
                    duint size = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "application/json", "{\"error\":\"Invalid address or size format\"}");
                        continue;
                    }
                    unsigned long long sizeParsed = 0;
                    if (!parseIntMaybeHex(sizeStr, sizeParsed)) {
                        sendHttpResponse(clientSocket, 400, "application/json", "{\"error\":\"Invalid address or size format\"}");
                        continue;
                    }
                    size = static_cast<duint>(sizeParsed);
                    
                    if (size > 1024 * 1024) {
                        sendHttpResponse(clientSocket, 400, "application/json", "{\"error\":\"Size too large\"}");
                        continue;
                    }

                    std::string hexOut;
                    std::string errorOut;
                    duint sizeRead = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        if (!readMemoryHex(addr, size, hexOut, sizeRead, errorOut)) {
                            std::stringstream err;
                            err << "{\"error\":\"" << jsonEscape(errorOut) << "\"}";
                            sendHttpResponse(clientSocket, errorOut == "Invalid memory address" ? 400 : 500,
                                             "application/json", err.str());
                            continue;
                        }
                    }

                    bool partial = sizeRead != size;
                    std::stringstream ss;
                    ss << "{";
                    ss << "\"data\":\"" << jsonEscape(hexOut) << "\",";
                    ss << "\"bytes\":" << sizeRead << ",";
                    ss << "\"partial\":" << (partial ? "true" : "false");
                    ss << "}";
                    sendHttpResponse(clientSocket, partial ? 206 : 200, "application/json", ss.str());
                }
                else if (path == "/Memory/Write") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Memory/Write")) {
                        continue;
                    }
                    std::string addrStr = queryParams["addr"];
                    std::string dataStr = !body.empty() ? body : queryParams["data"];
                    
                    if (addrStr.empty() || dataStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address or data");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    std::string cleaned;
                    if (!sanitizeHexBytes(dataStr, cleaned)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid data format");
                        continue;
                    }

                    std::vector<unsigned char> buffer;
                    buffer.reserve(cleaned.size() / 2);
                    for (size_t i = 0; i < cleaned.length(); i += 2) {
                        std::string byteString = cleaned.substr(i, 2);
                        try {
                            unsigned char byte = (unsigned char)std::stoi(byteString, nullptr, 16);
                            buffer.push_back(byte);
                        } catch (const std::exception& e) {
                            sendHttpResponse(clientSocket, 400, "text/plain", "Invalid data format");
                            continue;
                        }
                    }
                    
                    duint sizeWritten = 0;
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Memory::Write(addr, buffer.data(), buffer.size(), &sizeWritten);
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Memory written successfully" : "Failed to write memory");
                }
                else if (path == "/Memory/IsValidPtr") {
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    bool isValid = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        isValid = Script::Memory::IsValidPtr(addr);
                    }
                    sendHttpResponse(clientSocket, 200, "text/plain", isValid ? "true" : "false");
                }
                else if (path == "/Memory/GetProtect") {
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    unsigned int protect = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        protect = Script::Memory::GetProtect(addr);
                    }
                    std::stringstream ss;
                    ss << "0x" << std::hex << protect;
                    sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                }
                
                // =============================================================================
                // DEBUG API ENDPOINTS
                // =============================================================================
                else if (path == "/Debug/Run") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    bool submitted = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        submitted = DbgCmdExec("run");
                    }
                    sendHttpResponse(clientSocket, submitted ? 200 : 500, "text/plain",
                        submitted ? "Debug run queued" : "Failed to queue debug run");
                }
                else if (path == "/Debug/RunUntilUserCode") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    std::string autoStr = queryParams["autoResume"];
                    bool autoResume = ParseBool(autoStr, true);

                    std::string jsonOut;
                    bool ok = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        ok = startRunUntilUserCode(jsonOut, autoResume);
                    }
                    sendHttpResponse(clientSocket, ok ? 200 : 500, "application/json", jsonOut);
                }
                else if (path == "/Debug/CancelRunUntilUserCode") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    if (g_runUntilUserCodeBp && g_runUntilUserCodeOwnsBp.load()) {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        Script::Debug::DeleteBreakpoint(g_runUntilUserCodeBp);
                    }
                    clearRunUntilUserCodeState();
                    sendHttpResponse(clientSocket, 200, "text/plain", "Run-until-user-code canceled");
                }
                else if (path == "/Debug/Restart") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    bool submitted = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        submitted = DbgCmdExec("restart");
                    }
                    sendHttpResponse(clientSocket, submitted ? 200 : 500, "text/plain",
                        submitted ? "Debug restart queued" : "Failed to queue debug restart");
                }
                else if (path == "/Debug/Pause") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    cancelRunUntilUserCodeIfActive();
                    std::string waitStr = queryParams["wait"];
                    bool wait = ParseBool(waitStr, true);
                    unsigned int timeoutMs = 5000;
                    std::string timeoutStr = queryParams["timeoutMs"];
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool submitted = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        submitted = DbgCmdExec("pause");
                    }
                    if (!submitted) {
                        sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                        continue;
                    }
                    if (wait) {
                        bool paused = waitForDebuggerPause(timeoutMs);
                        sendHttpResponse(clientSocket, paused ? 200 : 504, "text/plain",
                            paused ? "Debug paused" : "Pause timed out");
                        continue;
                    }
                    sendHttpResponse(clientSocket, 200, "text/plain", "Debug pause queued");
                }
                else if (path == "/Debug/Stop") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Debug/Stop")) {
                        continue;
                    }
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        Script::Debug::Stop();
                    }
                    sendHttpResponse(clientSocket, 200, "text/plain", "Debug stop executed");
                }
                else if (path == "/Debug/StepIn") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    cancelRunUntilUserCodeIfActive();
                    std::string autoPauseStr = queryParams["autoPause"];
                    bool autoPause = ParseBool(autoPauseStr, true);
                    std::string timeoutStr = queryParams["timeoutMs"];
                    unsigned int timeoutMs = 5000;
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool running = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        running = DbgIsRunning();
                    }
                    if (running) {
                        if (!autoPause) {
                            sendHttpResponse(clientSocket, 409, "text/plain", "Debugger running; pause first");
                            continue;
                        }
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec("pause");
                        }
                        if (!submitted) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                            continue;
                        }
                        if (!waitForDebuggerPause(timeoutMs)) {
                            sendHttpResponse(clientSocket, 504, "text/plain", "Pause timed out");
                            continue;
                        }
                    }
                    bool queued = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        queued = queueDebugStepCommand("sti");
                    }
                    sendHttpResponse(clientSocket, queued ? 200 : 409, "text/plain",
                        queued ? "Step in queued" : "Step in busy");
                }
                else if (path == "/Debug/StepOver") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    cancelRunUntilUserCodeIfActive();
                    std::string autoPauseStr = queryParams["autoPause"];
                    bool autoPause = ParseBool(autoPauseStr, true);
                    std::string timeoutStr = queryParams["timeoutMs"];
                    unsigned int timeoutMs = 5000;
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool running = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        running = DbgIsRunning();
                    }
                    if (running) {
                        if (!autoPause) {
                            sendHttpResponse(clientSocket, 409, "text/plain", "Debugger running; pause first");
                            continue;
                        }
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec("pause");
                        }
                        if (!submitted) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                            continue;
                        }
                        if (!waitForDebuggerPause(timeoutMs)) {
                            sendHttpResponse(clientSocket, 504, "text/plain", "Pause timed out");
                            continue;
                        }
                    }
                    bool queued = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        queued = addStepBatch(1);
                    }
                    sendHttpResponse(clientSocket, queued ? 200 : 409, "text/plain",
                        queued ? "Step over queued" : "Step over busy");
                }
                else if (path == "/Debug/StepOverN") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    cancelRunUntilUserCodeIfActive();
                    std::string autoPauseStr = queryParams["autoPause"];
                    bool autoPause = ParseBool(autoPauseStr, true);
                    std::string timeoutStr = queryParams["timeoutMs"];
                    unsigned int timeoutMs = 5000;
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool running = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        running = DbgIsRunning();
                    }
                    if (running) {
                        if (!autoPause) {
                            sendHttpResponse(clientSocket, 409, "text/plain", "Debugger running; pause first");
                            continue;
                        }
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec("pause");
                        }
                        if (!submitted) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                            continue;
                        }
                        if (!waitForDebuggerPause(timeoutMs)) {
                            sendHttpResponse(clientSocket, 504, "text/plain", "Pause timed out");
                            continue;
                        }
                    }
                    std::string countStr = queryParams["count"];
                    int count = 0;
                    if (!countStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(countStr, parsed)) {
                            count = static_cast<int>(parsed);
                        }
                    }
                    if (count <= 0) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid count");
                        continue;
                    }
                    bool started = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        started = addStepBatch(count);
                    }
                    sendHttpResponse(clientSocket, started ? 200 : 409, "text/plain",
                        started ? "Step over batch queued" : "Step over busy");
                }
                else if (path == "/Debug/CancelStepBatch") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        stopStepBatch();
                    }
                    sendHttpResponse(clientSocket, 200, "text/plain", "Step batch canceled");
                }
                else if (path == "/Debug/StepOut") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    cancelRunUntilUserCodeIfActive();
                    std::string autoPauseStr = queryParams["autoPause"];
                    bool autoPause = ParseBool(autoPauseStr, true);
                    std::string timeoutStr = queryParams["timeoutMs"];
                    unsigned int timeoutMs = 5000;
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool running = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        running = DbgIsRunning();
                    }
                    if (running) {
                        if (!autoPause) {
                            sendHttpResponse(clientSocket, 409, "text/plain", "Debugger running; pause first");
                            continue;
                        }
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec("pause");
                        }
                        if (!submitted) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                            continue;
                        }
                        if (!waitForDebuggerPause(timeoutMs)) {
                            sendHttpResponse(clientSocket, 504, "text/plain", "Pause timed out");
                            continue;
                        }
                    }
                    bool queued = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        queued = queueDebugStepCommand("rto");
                    }
                    sendHttpResponse(clientSocket, queued ? 200 : 409, "text/plain",
                        queued ? "Step out queued" : "Step out busy");
                }
                else if (path == "/Debug/SetBreakpoint") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Debug::SetBreakpoint(addr);
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Breakpoint set successfully" : "Failed to set breakpoint");
                }
                else if (path == "/Debug/DeleteBreakpoint") {
                    if (!ensureDebugging(clientSocket)) {
                        continue;
                    }
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Debug::DeleteBreakpoint(addr);
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Breakpoint deleted successfully" : "Failed to delete breakpoint");
                }
                else if (path == "/Cmdline/Get") {
                    const DBGFUNCTIONS* dbg = nullptr;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        dbg = DbgFunctions();
                    }
                    if (!dbg || !dbg->GetCmdline) {
                        sendHttpResponse(clientSocket, 500, "text/plain", "GetCmdline not available");
                        continue;
                    }

                    size_t size = 0;
                    bool ok = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        ok = dbg->GetCmdline(nullptr, &size);
                    }
                    if (!ok || size == 0) {
                        size = 4096;
                    }

                    std::string buffer(size, '\0');
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        if (!dbg->GetCmdline(buffer.data(), &size)) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to get cmdline");
                            continue;
                        }
                    }

                    std::string cmdline = std::string(buffer.c_str());
                    sendHttpResponse(clientSocket, 200, "text/plain", cmdline);
                }
                else if (path == "/Cmdline/Set") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Cmdline/Set")) {
                        continue;
                    }
                    const DBGFUNCTIONS* dbg = nullptr;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        dbg = DbgFunctions();
                    }
                    if (!dbg || !dbg->SetCmdline) {
                        sendHttpResponse(clientSocket, 500, "text/plain", "SetCmdline not available");
                        continue;
                    }

                    std::string cmdline = queryParams["cmdline"];
                    if (cmdline.empty() && !body.empty()) {
                        cmdline = body;
                    }

                    if (cmdline.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing cmdline parameter");
                        continue;
                    }

                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = dbg->SetCmdline(cmdline.c_str());
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain",
                        success ? "Cmdline set successfully" : "Failed to set cmdline");
                }
                
                else if (path == "/Assembler/Assemble") {
                    std::string addrStr = queryParams["addr"];
                    std::string instruction = queryParams["instruction"];
                    if (instruction.empty() && !body.empty()) {
                        instruction = body;
                    }
                    
                    if (addrStr.empty() || instruction.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address or instruction parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    unsigned char dest[16];
                    int size = 16;
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Assembler::Assemble(addr, dest, &size, instruction.c_str());
                    }
                    
                    if (success) {
                        std::stringstream ss;
                        ss << "{\"success\":true,\"size\":" << size << ",\"bytes\":\"";
                        for (int i = 0; i < size; i++) {
                            ss << std::setw(2) << std::setfill('0') << std::hex << (int)dest[i];
                        }
                        ss << "\"}";
                        sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                    } else {
                        sendHttpResponse(clientSocket, 500, "text/plain", "Failed to assemble instruction");
                    }
                }
                else if (path == "/Assembler/AssembleMem") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Assembler/AssembleMem")) {
                        continue;
                    }
                    std::string addrStr = queryParams["addr"];
                    std::string instruction = queryParams["instruction"];
                    if (instruction.empty() && !body.empty()) {
                        instruction = body;
                    }
                    
                    if (addrStr.empty() || instruction.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address or instruction parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Assembler::AssembleMem(addr, instruction.c_str());
                    }
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Instruction assembled in memory successfully" : "Failed to assemble instruction in memory");
                }
                else if (path == "/Stack/Pop") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Stack/Pop")) {
                        continue;
                    }
                    duint value = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        value = Script::Stack::Pop();
                    }
                    std::stringstream ss;
                    ss << "0x" << std::hex << value;
                    sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                }
                else if (path == "/Stack/Push") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Stack/Push")) {
                        continue;
                    }
                    std::string valueStr = queryParams["value"];
                    if (valueStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing value parameter");
                        continue;
                    }
                    
                    duint value = 0;
                    if (!parseAddressMaybeHex(valueStr, value)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid value format");
                        continue;
                    }
                    
                    duint prevTop = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        prevTop = Script::Stack::Push(value);
                    }
                    std::stringstream ss;
                    ss << "0x" << std::hex << prevTop;
                    sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                }
                else if (path == "/Stack/Peek") {
                    std::string offsetStr = queryParams["offset"];
                    int offset = 0;
                    if (!offsetStr.empty()) {
                        unsigned long long parsed = 0;
                        if (!parseIntMaybeHex(offsetStr, parsed)) {
                            sendHttpResponse(clientSocket, 400, "text/plain", "Invalid offset format");
                            continue;
                        }
                        offset = static_cast<int>(parsed);
                    }
                    
                    duint value = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        value = Script::Stack::Peek(offset);
                    }
                    std::stringstream ss;
                    ss << "0x" << std::hex << value;
                    sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                }
                else if (path == "/Disasm/GetInstruction") {
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    
                    // Use the correct DISASM_INSTR structure
                    DISASM_INSTR instr;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        DbgDisasmAt(addr, &instr);
                    }
                    
                    // Create JSON response with available instruction details
                    std::stringstream ss;
                    ss << "{";
                    ss << "\"address\":\"0x" << std::hex << addr << "\",";
                    ss << "\"instruction\":\"" << instr.instruction << "\",";
                    ss << "\"size\":" << std::dec << instr.instr_size;
                    ss << "}";
                    
                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                else if (path == "/Disasm/GetInstructionRange") {
                    std::string addrStr = queryParams["addr"];
                    std::string countStr = queryParams["count"];
                    
                    if (addrStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing address parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    int count = 1;
                    
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address or count format");
                        continue;
                    }
                    if (!countStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(countStr, parsed)) {
                            count = static_cast<int>(parsed);
                        } else {
                            sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address or count format");
                            continue;
                        }
                    }
                    
                    if (count <= 0 || count > 100) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Count must be between 1 and 100");
                        continue;
                    }
                    
                    // Get multiple instructions
                    std::stringstream ss;
                    ss << "[";
                    
                    duint currentAddr = addr;
                    for (int i = 0; i < count; i++) {
                        DISASM_INSTR instr;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            DbgDisasmAt(currentAddr, &instr);
                        }
                        
                        if (instr.instr_size > 0) {
                            if (i > 0) ss << ",";
                            
                            ss << "{";
                            ss << "\"address\":\"0x" << std::hex << currentAddr << "\",";
                            ss << "\"instruction\":\"" << instr.instruction << "\",";
                            ss << "\"size\":" << std::dec << instr.instr_size;
                            ss << "}";
                            
                            currentAddr += instr.instr_size;
                        } else {
                            break;
                        }
                    }
                    
                    ss << "]";
                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                else if (path == "/Disasm/GetInstructionAtRIP") {
                    duint rip = 0;
                    DISASM_INSTR instr;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        rip = Script::Register::Get(REG_IP);
                        DbgDisasmAt(rip, &instr);
                    }

                    std::stringstream ss;
                    ss << "{";
                    ss << "\"address\":\"0x" << std::hex << rip << "\",";
                    ss << "\"instruction\":\"" << instr.instruction << "\",";
                    ss << "\"size\":" << std::dec << instr.instr_size;
                    ss << "}";

                    sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                }
                else if (path == "/Disasm/StepInWithDisasm") {
                    cancelRunUntilUserCodeIfActive();
                    std::string autoPauseStr = queryParams["autoPause"];
                    bool autoPause = ParseBool(autoPauseStr, true);
                    std::string timeoutStr = queryParams["timeoutMs"];
                    unsigned int timeoutMs = 5000;
                    if (!timeoutStr.empty()) {
                        unsigned long long parsed = 0;
                        if (parseIntMaybeHex(timeoutStr, parsed)) {
                            timeoutMs = static_cast<unsigned int>(parsed);
                        }
                    }
                    bool running = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        running = DbgIsRunning();
                    }
                    if (running) {
                        if (!autoPause) {
                            sendHttpResponse(clientSocket, 409, "text/plain", "Debugger running; pause first");
                            continue;
                        }
                        bool submitted = false;
                        {
                            std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                            submitted = DbgCmdExec("pause");
                        }
                        if (!submitted) {
                            sendHttpResponse(clientSocket, 500, "text/plain", "Failed to queue debug pause");
                            continue;
                        }
                        if (!waitForDebuggerPause(timeoutMs)) {
                            sendHttpResponse(clientSocket, 504, "text/plain", "Pause timed out");
                            continue;
                        }
                    }
                    duint rip = 0;
                    DISASM_INSTR instr;
                    bool queued = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        rip = Script::Register::Get(REG_IP);
                        DbgDisasmAt(rip, &instr);
                        queued = queueDebugStepCommand("sti");
                    }

                    std::stringstream ss;
                    ss << "{";
                    ss << "\"queued\":" << (queued ? "true" : "false") << ",";
                    ss << "\"rip_before\":\"0x" << std::hex << rip << "\",";
                    ss << "\"instruction_before\":\"" << instr.instruction << "\",";
                    ss << "\"size\":" << std::dec << instr.instr_size;
                    ss << "}";

                    sendHttpResponse(clientSocket, queued ? 200 : 409, "application/json", ss.str());
                }
                // =============================================================================
                // FLAG API ENDPOINTS
                // =============================================================================
                else if (path == "/Flag/Get") {
                    std::string flagName = queryParams["flag"];
                    if (flagName.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing flag parameter");
                        continue;
                    }
                    
                    bool value = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        if (flagName == "ZF" || flagName == "zf") value = Script::Flag::GetZF();
                        else if (flagName == "OF" || flagName == "of") value = Script::Flag::GetOF();
                        else if (flagName == "CF" || flagName == "cf") value = Script::Flag::GetCF();
                        else if (flagName == "PF" || flagName == "pf") value = Script::Flag::GetPF();
                        else if (flagName == "SF" || flagName == "sf") value = Script::Flag::GetSF();
                        else if (flagName == "TF" || flagName == "tf") value = Script::Flag::GetTF();
                        else if (flagName == "AF" || flagName == "af") value = Script::Flag::GetAF();
                        else if (flagName == "DF" || flagName == "df") value = Script::Flag::GetDF();
                        else if (flagName == "IF" || flagName == "if") value = Script::Flag::GetIF();
                        else {
                            sendHttpResponse(clientSocket, 400, "text/plain", "Unknown flag");
                            continue;
                        }
                    }
                    
                    sendHttpResponse(clientSocket, 200, "text/plain", value ? "true" : "false");
                }
                else if (path == "/Flag/Set") {
                    if (!ensureConfirmed(clientSocket, queryParams, "Flag/Set")) {
                        continue;
                    }
                    std::string flagName = queryParams["flag"];
                    std::string valueStr = queryParams["value"];
                    if (flagName.empty() || valueStr.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing flag or value parameter");
                        continue;
                    }
                    
                    bool value = (valueStr == "true" || valueStr == "1");
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        if (flagName == "ZF" || flagName == "zf") success = Script::Flag::SetZF(value);
                        else if (flagName == "OF" || flagName == "of") success = Script::Flag::SetOF(value);
                        else if (flagName == "CF" || flagName == "cf") success = Script::Flag::SetCF(value);
                        else if (flagName == "PF" || flagName == "pf") success = Script::Flag::SetPF(value);
                        else if (flagName == "SF" || flagName == "sf") success = Script::Flag::SetSF(value);
                        else if (flagName == "TF" || flagName == "tf") success = Script::Flag::SetTF(value);
                        else if (flagName == "AF" || flagName == "af") success = Script::Flag::SetAF(value);
                        else if (flagName == "DF" || flagName == "df") success = Script::Flag::SetDF(value);
                        else if (flagName == "IF" || flagName == "if") success = Script::Flag::SetIF(value);
                        else {
                            sendHttpResponse(clientSocket, 400, "text/plain", "Unknown flag");
                            continue;
                        }
                    }
                    
                    sendHttpResponse(clientSocket, success ? 200 : 500, "text/plain", 
                        success ? "Flag set successfully" : "Failed to set flag");
                }
                
                // =============================================================================
                // PATTERN API ENDPOINTS
                // =============================================================================
                else if (path == "/Pattern/FindMem") {
                    std::string startStr = queryParams["start"];
                    std::string sizeStr = queryParams["size"];
                    std::string pattern = queryParams["pattern"];
                    std::string Pattern = SanitizePattern(pattern);
                    if (startStr.empty() || sizeStr.empty() || pattern.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing start, size, or pattern parameter");
                        continue;
                    }
                    
                    duint start = 0, size = 0;

                    if (!parseAddressMaybeHex(startStr, start)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid start or size format");
                        continue;
                    }
                    if (!parseAddressMaybeHex(sizeStr, size)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid start or size format");
                        continue;
                    }
                    
                    duint result = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        result = Script::Pattern::FindMem(start, size, Pattern.c_str());
                    }
                    if (result != 0) {
                        std::stringstream ss;
                        ss << "0x" << std::hex << result;
                        sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                    } else {
                        sendHttpResponse(clientSocket, 404, "text/plain", "Pattern not found");
                    }
                }
                
                else if (path == "/Misc/ParseExpression") {
                    std::string expression = queryParams["expression"];
                    if (expression.empty() && !body.empty()) {
                        expression = body;
                    }
                    
                    if (expression.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing expression parameter");
                        continue;
                    }
                    
                    duint value = 0;
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Misc::ParseExpression(expression.c_str(), &value);
                    }
                    
                    if (success) {
                        std::stringstream ss;
                        ss << "0x" << std::hex << value;
                        sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                    } else {
                        sendHttpResponse(clientSocket, 500, "text/plain", "Failed to parse expression");
                    }
                }
                else if (path == "/Misc/RemoteGetProcAddress") {
                    std::string module = queryParams["module"];
                    std::string api = queryParams["api"];
                    
                    if (module.empty() || api.empty()) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Missing module or api parameter");
                        continue;
                    }
                    
                    duint addr = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        addr = Script::Misc::RemoteGetProcAddress(module.c_str(), api.c_str());
                    }
                    if (addr != 0) {
                        std::stringstream ss;
                        ss << "0x" << std::hex << addr;
                        sendHttpResponse(clientSocket, 200, "text/plain", ss.str());
                    } else {
                        sendHttpResponse(clientSocket, 404, "text/plain", "Function not found");
                    }
                }
                else if (path == "/MemoryBase") {
                    std::string addrStr = queryParams["addr"];
                    if (addrStr.empty() && !body.empty()) {
                        addrStr = body;
                    }
                    _plugin_logprintf("MemoryBase endpoint called with addr: %s\n", addrStr.c_str());
                    // Convert string address to duint
                    duint addr = 0;
                    if (!parseAddressMaybeHex(addrStr, addr)) {
                        sendHttpResponse(clientSocket, 400, "text/plain", "Invalid address format");
                        continue;
                    }
                    _plugin_logprintf("Converted address: " FMT_DUINT_HEX "\n", DUINT_CAST_PRINTF(addr));
                    
                    // Get the base address and size
                    duint size = 0;
                    duint baseAddr = 0;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        baseAddr = DbgMemFindBaseAddr(addr, &size);
                    }
                    _plugin_logprintf("Base address found: " FMT_DUINT_HEX ", size: " FMT_DUINT_DEC "\n", DUINT_CAST_PRINTF(baseAddr), DUSIZE_CAST_PRINTF(size));
                    if (baseAddr == 0) {
                        sendHttpResponse(clientSocket, 404, "text/plain", "No module found for this address");
                    }
                    else {
                        // Format the response as JSON
                        std::stringstream ss;
                        ss << "{\"base_address\":\"0x" << std::hex << baseAddr << "\",\"size\":\"0x" << std::hex << size << "\"}";
                        sendHttpResponse(clientSocket, 200, "application/json", ss.str());
                    }
                }
                else if (path == "/GetModuleList") {
                    // Create a list to store the module information
                    ListInfo moduleList;
                    
                    // Get the list of modules
                    bool success = false;
                    {
                        std::lock_guard<std::mutex> dbgLock(g_dbgApiMutex);
                        success = Script::Module::GetList(&moduleList);
                    }
                    
                    if (!success) {
                        sendHttpResponse(clientSocket, 500, "text/plain", "Failed to get module list");
                    }
                    else {
                        // Create a JSON array to hold the module information
                        std::stringstream jsonResponse;
                        jsonResponse << "[";
                        
                        // Iterate through each module in the list
                        size_t count = moduleList.count;
                        Script::Module::ModuleInfo* modules = (Script::Module::ModuleInfo*)moduleList.data;
                        
                        for (size_t i = 0; i < count; i++) {
                            if (i > 0) jsonResponse << ",";
                            
                            // Add module info as JSON object
                            jsonResponse << "{";
                            jsonResponse << "\"name\":\"" << modules[i].name << "\",";
                            jsonResponse << "\"base\":\"0x" << std::hex << modules[i].base << "\",";
                            jsonResponse << "\"size\":\"0x" << std::hex << modules[i].size << "\",";
                            jsonResponse << "\"entry\":\"0x" << std::hex << modules[i].entry << "\",";
                            jsonResponse << "\"sectionCount\":" << std::dec << modules[i].sectionCount << ",";
                            jsonResponse << "\"path\":\"" << modules[i].path << "\"";
                            jsonResponse << "}";
                        }
                        
                        jsonResponse << "]";
                        
                        // Free the list
                        BridgeFree(moduleList.data);
                        
                        // Send the response
                        sendHttpResponse(clientSocket, 200, "application/json", jsonResponse.str());
                    }
                }
                // Memory Access Functions (Legacy endpoints for compatibility)
                
            }
            catch (const std::exception& e) {
                // Exception in handling request
                sendHttpResponse(clientSocket, 500, "text/plain", std::string("Internal Server Error: ") + e.what());
            }
        }
        
    } while(false);

    // Close the client socket
    closesocket(clientSocket);
}

// Function to read the HTTP request
std::string readHttpRequest(SOCKET clientSocket, bool* tooLarge) {
    std::string request;
    char buffer[1024];
    int bytesReceived;
    
    // Set socket to blocking mode to receive full request
    u_long mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    // Set a receive timeout to avoid hanging the server
    int timeoutMs = 5000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
    
    bool headersDone = false;
    size_t contentLength = 0;
    size_t headerEndPos = std::string::npos;

    if (tooLarge) {
        *tooLarge = false;
    }

    while (request.size() < g_maxRequestSize) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        request.append(buffer, bytesReceived);

        if (!headersDone) {
            headerEndPos = request.find("\r\n\r\n");
            if (headerEndPos != std::string::npos) {
                headersDone = true;
                std::string headers = request.substr(0, headerEndPos);
                std::string headersLower = headers;
                std::transform(headersLower.begin(), headersLower.end(), headersLower.begin(), ::tolower);

                size_t pos = headersLower.find("content-length:");
                if (pos != std::string::npos) {
                    size_t lineEnd = headersLower.find("\r\n", pos);
                    std::string lenStr = headersLower.substr(pos + 15,
                        lineEnd == std::string::npos ? std::string::npos : lineEnd - (pos + 15));
                    // trim leading spaces
                    size_t start = lenStr.find_first_not_of(" \t");
                    if (start != std::string::npos) {
                        lenStr = lenStr.substr(start);
                    }
                    try {
                        contentLength = std::stoul(lenStr);
                    } catch (...) {
                        contentLength = 0;
                    }
                    if (contentLength > g_maxRequestSize) {
                        if (tooLarge) {
                            *tooLarge = true;
                        }
                        return "";
                    }
                }
            }
        }

        if (headersDone) {
            if (contentLength == 0) {
                break;
            }
            size_t bodySize = request.size() - (headerEndPos + 4);
            if (bodySize >= contentLength) {
                break;
            }
        }
    }

    if (request.size() >= g_maxRequestSize) {
        if (tooLarge) {
            *tooLarge = true;
        }
        return "";
    }
    
    return request;
}

// Function to parse an HTTP request
void parseHttpRequest(const std::string& request, std::string& method, std::string& path, std::string& query, std::string& body) {
    // Parse the request line
    size_t firstLineEnd = request.find("\r\n");
    if (firstLineEnd == std::string::npos) {
        return;
    }
    
    std::string requestLine = request.substr(0, firstLineEnd);
    
    // Extract method and URL
    size_t methodEnd = requestLine.find(' ');
    if (methodEnd == std::string::npos) {
        return;
    }
    
    method = requestLine.substr(0, methodEnd);
    
    size_t urlEnd = requestLine.find(' ', methodEnd + 1);
    if (urlEnd == std::string::npos) {
        return;
    }
    
    std::string url = requestLine.substr(methodEnd + 1, urlEnd - methodEnd - 1);
    
    // Split URL into path and query
    size_t queryStart = url.find('?');
    if (queryStart != std::string::npos) {
        path = url.substr(0, queryStart);
        query = url.substr(queryStart + 1);
    } else {
        path = url;
        query = "";
    }
    
    // Find the end of headers and start of body
    size_t headersEnd = request.find("\r\n\r\n");
    if (headersEnd == std::string::npos) {
        return;
    }
    
    // Extract body
    body = request.substr(headersEnd + 4);
}

// Function to send HTTP response
void sendHttpResponse(SOCKET clientSocket, int statusCode, const std::string& contentType, const std::string& responseBody) {
    // Prepare status line
    std::string statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 206: statusText = "Partial Content"; break;
        case 400: statusText = "Bad Request"; break;
        case 403: statusText = "Forbidden"; break;
        case 409: statusText = "Conflict"; break;
        case 413: statusText = "Payload Too Large"; break;
        case 429: statusText = "Too Many Requests"; break;
        case 404: statusText = "Not Found"; break;
        case 503: statusText = "Service Unavailable"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "Unknown";
    }
    
    // Build the response
    std::stringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << responseBody.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << responseBody;
    
    // Send the response
    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), (int)responseStr.length(), 0);
}



// Command callback for toggling HTTP server
bool cbEnableHttpServer(int argc, char* argv[]) {
    if (g_httpServerRunning.load()) {
        _plugin_logputs("Stopping HTTP server...");
        stopHttpServer();
        _plugin_logputs("HTTP server stopped");
    } else {
        _plugin_logputs("Starting HTTP server...");
        if (startHttpServer()) {
            _plugin_logprintf("HTTP server started on port %d\n", g_httpPort);
        } else {
            _plugin_logputs("Failed to start HTTP server");
        }
    }
    return true;
}

// Command callback for changing HTTP server port
bool cbSetHttpPort(int argc, char* argv[]) {
    if (argc < 2) {
        _plugin_logputs("Usage: httpport [port_number]");
        return false;
    }
    
    int port;
    try {
        port = std::stoi(argv[1]);
    }
    catch (const std::exception&) {
        _plugin_logputs("Invalid port number");
        return false;
    }
    
    if (port <= 0 || port > 65535) {
        _plugin_logputs("Port number must be between 1 and 65535");
        return false;
    }
    
    g_httpPort = port;
    
    if (g_httpServerRunning.load()) {
        _plugin_logputs("Restarting HTTP server with new port...");
        stopHttpServer();
        if (startHttpServer()) {
            _plugin_logprintf("HTTP server restarted on port %d\n", g_httpPort);
        } else {
            _plugin_logputs("Failed to restart HTTP server");
        }
    } else {
        _plugin_logprintf("HTTP port set to %d\n", g_httpPort);
    }
    
    return true;
}

// Register plugin commands
void registerCommands() {
    _plugin_registercommand(g_pluginHandle, "httpserver", cbEnableHttpServer, 
                           "Toggle HTTP server on/off");
    _plugin_registercommand(g_pluginHandle, "httpport", cbSetHttpPort, 
                           "Set HTTP server port");
}

// =============================================================================
// Debug callbacks
// =============================================================================

void cbSystemBreakpoint(CBTYPE cbType, void* callbackInfo) {
    (void)cbType;
    (void)callbackInfo;

    if (g_runUntilUserCodeActive.load() && g_runUntilUserCodeAutoResume.load()) {
        enqueueDbgTask([] {
            DbgCmdExec("run");
        });
    }
}

void cbBreakpoint(CBTYPE cbType, void* callbackInfo) {
    (void)cbType;
    if (!callbackInfo) {
        return;
    }

    if (g_stepBatchActive.load()) {
        stopStepBatch();
    }

    if (!g_runUntilUserCodeActive.load()) {
        return;
    }

    PLUG_CB_BREAKPOINT* info = (PLUG_CB_BREAKPOINT*)callbackInfo;
    if (!info->breakpoint) {
        return;
    }

    duint addr = info->breakpoint->addr;
    if (g_runUntilUserCodeBp && addr == g_runUntilUserCodeBp) {
        duint bpAddr = addr;
        if (g_runUntilUserCodeOwnsBp.load()) {
            enqueueDbgTask([bpAddr] {
                Script::Debug::DeleteBreakpoint(bpAddr);
            });
        }
        clearRunUntilUserCodeState();
        return;
    }

    g_runUntilUserCodeUserBpHit.store(true);
}

void cbPauseDebug(CBTYPE cbType, void* callbackInfo) {
    (void)cbType;
    (void)callbackInfo;

    g_debugStepBusy.store(false);
    if (g_stepBatchActive.load()) {
        stopStepBatch();
    }

    bool runActive = g_runUntilUserCodeActive.load();
    bool autoResume = g_runUntilUserCodeAutoResume.load();
    bool userBpHit = g_runUntilUserCodeUserBpHit.load();

    enqueueDbgTask([runActive, autoResume, userBpHit] {
        duint rip = Script::Register::Get(REG_IP);
        bool shouldResume = ShouldResumeAfterPause(
            runActive,
            autoResume,
            userBpHit,
            isSystemModuleAddr(rip));

        if (shouldResume) {
            DbgCmdExec("run");
            return;
        }

        if (runActive) {
            clearRunUntilUserCodeState();
        }
    });
}

void cbStopDebug(CBTYPE cbType, void* callbackInfo) {
    (void)cbType;
    (void)callbackInfo;
    g_debugStepBusy.store(false);
    stopStepBatch();
    clearRunUntilUserCodeState();
}

void cbStepped(CBTYPE cbType, void* callbackInfo) {
    (void)cbType;
    (void)callbackInfo;
    if (g_stepBatchActive.load()) {
        int remaining = g_stepBatchRemaining.load();
        if (remaining > 0) {
            remaining -= 1;
            g_stepBatchRemaining.store(remaining);
        }

        if (remaining > 0) {
            bool queued = enqueueDbgTask([] {
                DbgCmdExec("sto");
            });
            if (!queued) {
                stopStepBatch();
            }
            return;
        }
        stopStepBatch();
        return;
    }

    g_debugStepBusy.store(false);
}

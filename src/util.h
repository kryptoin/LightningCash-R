// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2025 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_UTIL_H
#define BITCOIN_UTIL_H

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <compat.h>
#include <fs.h>
#include <stdint.h>
#include <sync.h>
#include <tinyformat.h>
#include <utiltime.h>

#include <atomic>
#include <boost/signals2/signal.hpp>
#include <boost/thread/condition_variable.hpp>  // for boost::thread_interrupted
#include <exception>
#include <map>
#include <string>
#include <vector>

// Application startup time (used for uptime calculation)
int64_t GetStartupTime();

static const bool DEFAULT_LOGTIMEMICROS = false;
static const bool DEFAULT_LOGIPS = false;
static const bool DEFAULT_LOGTIMESTAMPS = true;
extern const char* const DEFAULT_DEBUGLOGFILE;

class CTranslationInterface {
 public:
  boost::signals2::signal<std::string(const char* psz)> Translate;
};

extern bool fPrintToConsole;
extern bool fPrintToDebugLog;

extern bool fLogTimestamps;
extern bool fLogTimeMicros;
extern bool fLogIPs;
extern std::atomic<bool> fReopenDebugLog;
extern CTranslationInterface translationInterface;

extern const char* const BITCOIN_CONF_FILENAME;
extern const char* const BITCOIN_PID_FILENAME;

extern std::atomic<uint32_t> logCategories;

inline std::string _(const char* psz) {
  boost::optional<std::string> rv = translationInterface.Translate(psz);
  return rv ? (*rv) : psz;
}

void SetupEnvironment();
bool SetupNetworking();

struct CLogCategoryActive {
  std::string category;
  bool active;
};

namespace BCLog {
enum LogFlags : uint32_t {
  NONE = 0,
  NET = (1 << 0),
  TOR = (1 << 1),
  MEMPOOL = (1 << 2),
  HTTP = (1 << 3),
  BENCH = (1 << 4),
  ZMQ = (1 << 5),
  DB = (1 << 6),
  RPC = (1 << 7),
  ESTIMATEFEE = (1 << 8),
  ADDRMAN = (1 << 9),
  SELECTCOINS = (1 << 10),
  REINDEX = (1 << 11),
  CMPCTBLOCK = (1 << 12),
  RAND = (1 << 13),
  PRUNE = (1 << 14),
  PROXY = (1 << 15),
  MEMPOOLREJ = (1 << 16),
  LIBEVENT = (1 << 17),
  COINDB = (1 << 18),
  QT = (1 << 19),
  LEVELDB = (1 << 20),
  HIVE = (1 << 21),  // LightningCashr: Hive logging
  ALL = ~(uint32_t)0,
};
}

static inline bool LogAcceptCategory(uint32_t category) {
  return (logCategories.load(std::memory_order_relaxed) & category) != 0;
}

std::string ListLogCategories();

std::vector<CLogCategoryActive> ListActiveLogCategories();

bool GetLogCategory(uint32_t* f, const std::string* str);

int LogPrintStr(const std::string& str);

template <typename... Args>
static inline void LogPrintf(const char* fmt, const Args&... args) {
    std::string _msg = tfm::format(fmt, args...);
    LogPrintStr(_msg);
}

template <typename... Args>
static inline void LogPrint(uint32_t category, const char* fmt, const Args&... args) {
    if (LogAcceptCategory(category)) {
        std::string _msg = tfm::format(fmt, args...);
        LogPrintStr(_msg);
    }
}

template <typename... Args>
std::string FormatStringFromLogArgs(const char* fmt, const Args&... args) {
  return fmt;
}

static inline void MarkUsed() {}
template <typename T, typename... Args>
static inline void MarkUsed(const T& t, const Args&... args) {
  (void)t;
  MarkUsed(args...);
}

template <typename... Args>
bool error(const char* fmt, const Args&... args) {
  LogPrintStr("ERROR: " + tfm::format(fmt, args...) + "\n");
  return false;
}

void PrintExceptionContinue(const std::exception* pex, const char* pszThread);
void FileCommit(FILE* file);
bool TruncateFile(FILE* file, unsigned int length);
int RaiseFileDescriptorLimit(int nMinFD);
void AllocateFileRange(FILE* file, unsigned int offset, unsigned int length);
bool RenameOver(fs::path src, fs::path dest);
bool LockDirectory(const fs::path& directory, const std::string lockfile_name,
                   bool probe_only = false);

void ReleaseDirectoryLocks();

bool TryCreateDirectories(const fs::path& p);
fs::path GetDefaultDataDir();
const fs::path& GetDataDir(bool fNetSpecific = true);
void ClearDatadirCache();
fs::path GetConfigFile(const std::string& confPath);
#ifndef WIN32
fs::path GetPidFile();
void CreatePidFile(const fs::path& path, pid_t pid);
#endif
#ifdef WIN32
fs::path GetSpecialFolderPath(int nFolder, bool fCreate = true);
#endif
fs::path GetDebugLogPath();
bool OpenDebugLog();
void ShrinkDebugFile();
void runCommand(const std::string& strCommand);

inline bool IsSwitchChar(char c) {
#ifdef WIN32
  return c == '-' || c == '/';
#else
  return c == '-';
#endif
}

class ArgsManager {
 protected:
  mutable CCriticalSection cs_args;
  std::map<std::string, std::string> mapArgs;
  std::map<std::string, std::vector<std::string>> mapMultiArgs;

 public:
  void ParseParameters(int argc, const char* const argv[]);
  void ReadConfigFile(const std::string& confPath);

  std::vector<std::string> GetArgs(const std::string& strArg) const;

  bool IsArgSet(const std::string& strArg) const;

  std::string GetArg(const std::string& strArg,
                     const std::string& strDefault) const;

  int64_t GetArg(const std::string& strArg, int64_t nDefault) const;

  bool GetBoolArg(const std::string& strArg, bool fDefault) const;

  bool SoftSetArg(const std::string& strArg, const std::string& strValue);

  bool SoftSetBoolArg(const std::string& strArg, bool fValue);

  // Forces an arg setting. Called by SoftSetArg() if the arg hasn't already
  // been set. Also called directly in testing.
  void ForceSetArg(const std::string& strArg, const std::string& strValue);
};

extern ArgsManager gArgs;

std::string HelpMessageGroup(const std::string& message);

std::string HelpMessageOpt(const std::string& option,
                           const std::string& message);

int GetNumCores();
int GetNumVirtualCores();  // LightningCashr: Hive: Mining Optimisations: Return
                           // number of virt cores

void RenameThread(const char* name);

template <typename Callable>
void TraceThread(const char* name, Callable func) {
  std::string s = strprintf("bitcoin-%s", name);
  RenameThread(s.c_str());
  try {
    //LogPrintf("%s thread start\n", name);
    func();
    //LogPrintf("%s thread exit\n", name);
  } catch (const boost::thread_interrupted&) {
    //LogPrintf("%s thread interrupt\n", name);
    throw;
  } catch (const std::exception& e) {
    PrintExceptionContinue(&e, name);
    throw;
  } catch (...) {
    PrintExceptionContinue(nullptr, name);
    throw;
  }
}

std::string CopyrightHolders(const std::string& strPrefix);

//! Substitute for C++14 std::make_unique.
template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif  // BITCOIN_UTIL_H
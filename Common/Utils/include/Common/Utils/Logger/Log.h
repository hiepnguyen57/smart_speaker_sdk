#ifndef DEVICE_CLIENT_SDK_COMMON_UTILS_LOGGER_LOG_H_
#define DEVICE_CLIENT_SDK_COMMON_UTILS_LOGGER_LOG_H_

#include <sstream>
#include <string>
#include <mutex>
#include <iomanip>
#include "Level.h"

namespace deviceClientSDK {
namespace common {
namespace utils {
namespace logger {


//#define NDEBUG
#ifdef NDEBUG
#define LOG_DEBUG     Log<OutputToFile>().Print(Level::NONE)
#define LOG_INFO      Log<OutputToFile>().Print(Level::NONE)
#define LOG_WARN      Log<OutputToFile>().Print(Level::NONE)
#define LOG_ERROR     Log<OutputToFile>().Print(Level::NONE)    
#define LOG_CRITICAL  Log<OutputToFile>().Print(Level::NONE)    
#else //DEBUG

#define LOG_DEBUG     Log<OutputToFile>().Print(Level::DEBUG)
#define LOG_INFO      Log<OutputToFile>().Print(Level::INFO)
#define LOG_WARN      Log<OutputToFile>().Print(Level::WARN)
#define LOG_ERROR     Log<OutputToFile>().Print(Level::ERROR)
#define LOG_CRITICAL  Log<OutputToFile>().Print(Level::CRITICAL)
#endif


/*
 * Log class.
 * typename log_policy is output policy: stderr, stdout, OutputToFile, etc.
 */
template <typename log_policy>
class Log {
public:
    Log();
    virtual ~Log();
    std::ostringstream& Print(Level level = Level::INFO);
    Level& getCurrentLevel();
    void setLogLevel(Level newLevel);
protected:
    std::ostringstream os;
private:
    inline const tm* getLocalTime();
private:
    std::mutex    m_oMutex;
    Log(const Log&);
    Log& operator =(const Log&);
    Level m_level;
    tm mLocalTime;
};

template <typename log_policy> 
Log<log_policy>::Log()
{
}

template <typename log_policy> 
std::ostringstream& Log<log_policy>::Print(Level level)
{
    m_oMutex.lock();
    if(level != Level::NONE)
    {
        os << "[" << std::put_time(getLocalTime(), "%Y-%m-%d %H:%M:%S") << "]";
        os << "[" << convertLevelToName(level) << "]\t";    
    }
    m_level = level;
    m_oMutex.unlock();
    return os;
}
/*
 * In the destructor print out the message
 */
template <typename log_policy> 
Log<log_policy>::~Log()
{
    if(m_level != Level::NONE) {
        os << std::endl;
        log_policy::Output(os.str());   
    } 

}

template <typename log_policy> 
Level& Log<log_policy>::getCurrentLevel()
{
    return m_level;
}

template <typename log_policy>
void Log<log_policy>::setLogLevel(Level newLevel)
{
    m_level = newLevel;
}

template <typename log_policy> 
inline const tm* Log<log_policy>::getLocalTime() {
    auto in_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    localtime_r(&in_time_t, &mLocalTime);
    return &mLocalTime;
}

/*
 * OutputToFile class
 */
class OutputToFile {
public:
    static FILE*& Stream();
    static void Output(const std::string& msg);
};

inline FILE*& OutputToFile::Stream()
{
    static FILE* pStream = stderr;
    return pStream;
}

inline void OutputToFile::Output(const std::string& msg)
{
    FILE* pStream = Stream();
    if (!pStream) return;

    fprintf(pStream, "%s", msg.c_str());
    fflush(pStream);
}

} // namespace logger
} // namespace utils
} // namespace common
} // namespace deviceClientSDK

#endif // DEVICE_CLIENT_SDK_COMMON_UTILS_LOGGER_LOG_H_
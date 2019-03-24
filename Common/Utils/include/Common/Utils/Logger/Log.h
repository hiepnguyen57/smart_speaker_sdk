#include <sstream>
#include <string>
#include <mutex>
#include <iomanip>
#include "Level.h"

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

// Convert date and time info from tm to a character string
// in format "YYYY-mm-DD HH:MM:SS" and send it to a stream
std::ostream& operator<< (std::ostream& stream, const tm* tm)
{
    // This section since GCC 4.8.1 did not implement std::put_time
    // return stream << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return stream << 1900 + tm->tm_year << '-' <<
    std::setfill('0') << std::setw(2) << tm->tm_mon + 1 << '-'
    << std::setfill('0') << std::setw(2) << tm->tm_mday << ' '
    << std::setfill('0') << std::setw(2) << tm->tm_hour << ':'
    << std::setfill('0') << std::setw(2) << tm->tm_min << ':'
    << std::setfill('0') << std::setw(2) << tm->tm_sec;
}

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
        os << "[" << getLocalTime() << "]";
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


// #include <sys/time.h>
// inline std::string LogNowTime()
// {
//  char buffer[64];
//  time_t t;
//  time(&t);
//  tm r = {0};
//  strftime(buffer, sizeof(buffer), "%Y-%m-%d  %H:%M:%S", localtime_r(&t, &r));
//  struct timeval tv;
//  gettimeofday(&tv, 0);
//  char result[100] = {0};
//  //std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
//  sprintf(result, "%s.%06ld", buffer, (long)tv.tv_usec / 1000); 
//  return result;
// }

#include <thread>
#include <list>
#include <memory>
#include "Common/Utils/Logger/Log.h"

#define TAG_LOGGER_TEST         "[LOGGER_TEST]\t"

using namespace std;
using namespace deviveClientSDK::common::utils::logger;
// the thread function
void threadFunc(int id)
{
    for(int i=0; i<100; i++)
    {
        // add a log message
        LOG_DEBUG << TAG_LOGGER_TEST << "Thread " << id << ", Message " << i;
    }
}

int main()
{
#ifdef FILE_LOGGER
    FILE *pFile = fopen("LoggerTest.log", "a");
    OutputToFile::Stream() = pFile;
#endif
    list<shared_ptr<thread>> oThreads;

    // create 100 threads
    for(int i=0; i<100; i++)
    {
        oThreads.push_back(shared_ptr<thread>(new thread(threadFunc, i)));
    }

    // also do logging from the main thread
    for(int i=0; i<100; i++)
    {
        // add a log message
        LOG_INFO << TAG_LOGGER_TEST << "Main thread, Message " << i;
    }

    //wait for all the threads to finish
    for(auto oThread : oThreads)
    {
        oThread->join();
    }

    return 0;
}
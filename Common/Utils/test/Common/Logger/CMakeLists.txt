cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(loggerTest)

#add_definitions(-DFILE_LOGGER)
#add_definitions(-DNDEBUG)

#Bring the headers into the project
include_directories(../../../include)

#add the sources using the set command as follows:
set(SOURCES LoggerTest.cpp ../../../src/Logger/Level.cpp)

find_package(Threads)
add_executable(loggerTest ${SOURCES})
target_link_libraries(loggerTest ${CMAKE_THREAD_LIBS_INIT} )

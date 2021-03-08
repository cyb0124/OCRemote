cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
project(OCRemote)
FIND_PACKAGE(Boost REQUIRED)
add_executable (OCRemote Actions.cpp Actions.h Entry.cpp Factory.cpp Factory.h
  Item.cpp Item.h Overload.h Processes.cpp Processes.h Serialize.cpp Serialize.h
  Server.cpp Server.h Storages.cpp Storages.h WeakCallback.h)
target_include_directories(OCRemote PRIVATE ${Boost_INCLUDE_DIR})

IF (WIN32)
  add_definitions(-DBOOST_ALL_NO_LIB)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  target_link_libraries(OCRemote ws2_32 mswsock)
ELSE ()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
  target_link_libraries(OCRemote Threads::Threads)
ENDIF ()

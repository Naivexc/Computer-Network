option(USE_FMT "Use fmt for debug" OFF)
cmake_minimum_required(VERSION 3.11)
project(assignment2-rtp)

aux_source_directory(. DIR_SRCS)

enable_testing()
add_subdirectory(third_party/googletest-release-1.12.1)
#if(USE_FMT)
#	find_package(fmt CONFIG REQUIRED)
#endif()
include_directories(third_party/googletest-release-1.12.1/googletest/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -no-pie")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Threads REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)


#添加头文件搜索路径
include_directories(/usr/local/include)

#添加库文件搜索路径
link_directories(/usr/local/lib)


include(GoogleTest)

#add_library(Window src/Window.cpp src/rtp.cpp src/NetConnectionInfo.cpp)
add_library(Window src/Window.cpp)
add_library(NetConnectionInfo src/NetConnectionInfo.cpp)
add_library(VecRecvAck src/VecRecvAck.cpp src/RecvAckNode.cpp)

add_library(rtpall
		src/rtp.cpp
		src/util.cpp
)

add_library(rtpsender 
		src/sender_def.cpp 
		src/NetConnectionInfo.cpp
)
target_link_libraries(rtpsender PUBLIC rtpall)

add_library(rtpreceiver src/receiver_def.cpp)
target_link_libraries(rtpreceiver PUBLIC rtpall)

add_executable(rtp_test_all
		src/test.cpp
)
target_link_libraries(rtp_test_all PUBLIC rtpsender rtpreceiver NetConnectionInfo Window VecRecvAck)
#if(USE_FMT)
#	target_link_libraries(rtp_test_all 	PUBLIC fmt::fmt)
#endif()
target_link_libraries(rtp_test_all PUBLIC Threads::Threads GTest::gtest_main)

gtest_discover_tests(rtp_test_all)

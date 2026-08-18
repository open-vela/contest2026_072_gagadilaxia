cmake_minimum_required(VERSION 3.10)
project(mindflow_agent)
set(CMAKE_CXX_STANDARD 11)

# 引入curl库
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable(mindflow_agent
    main.cc
    agent_state.cpp
)

# 链接curl
target_link_libraries(mindflow_agent ${CURL_LIBRARIES})

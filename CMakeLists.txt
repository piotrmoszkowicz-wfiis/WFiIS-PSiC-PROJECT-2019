cmake_minimum_required(VERSION 3.14)
project(WFiIS_2019_PSiC_Proxy)

set(CMAKE_CXX_STANDARD 14)

add_executable(WFiIS_2019_PSiC_Proxy src/main.cpp src/HttpServer.h src/HttpServer.cpp)

if (APPLE)
    include_directories(BEFORE /usr/local/include)

    target_link_libraries(WFiIS_2019_PSiC_Proxy
            /usr/local/Cellar/zeromq/4.3.2/lib/libzmq.a
            )
endif()
cmake_minimum_required(VERSION 3.21)
project(Proxy_Client_Server)

set(CMAKE_CXX_STANDARD 11)

add_executable(Proxy_Client_Server
        main.cpp srcs/ServerProxy.cpp srcs/ServerProxy.h)

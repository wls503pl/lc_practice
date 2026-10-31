cmake_minimum_required(VERSION 3.20)
project(websocket_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_executable(ws_server src/server.cpp)
target_include_directories(ws_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(ws_server PRIVATE Threads::Threads)

add_executable(ws_client src/client.cpp)
target_include_directories(ws_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(ws_client PRIVATE Threads::Threads)

find_package(OpenSSL REQUIRED)

add_executable(ws_binance_market src/binance_market_ws.cpp)
target_link_libraries(ws_binance_market PRIVATE OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

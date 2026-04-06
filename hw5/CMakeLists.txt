cmake_minimum_required(VERSION 3.16)

project(hw5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

add_executable(${PROJECT_NAME}
    main.cpp
    tcp_client.cpp
    protocol.cpp
    signal_manager.cpp
    hashrate_reader.cpp
    keyspace.cpp
    hasher.cpp
    bruteforce_engine.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)

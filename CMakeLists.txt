cmake_minimum_required (VERSION 3.0.2 FATAL_ERROR)
project (COSMOS_Server CXX)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")

set(ACCS_SOURCES
    src/cosmos/cosmos.cpp
    src/cosmos/packets.cpp
    src/cosmos/cosmosQueue.cpp
    src/ads1148/ads1148.cpp
    src/mcp3424/mcp3424.cpp
    src/mcp4725/mcp4725.cpp
    src/main.cpp
)

set (ACCS_LIBS ${EXTRA_LIBS} wiringPi pthread)

add_executable (accs ${ACCS_SOURCES})

target_link_libraries (accs ${ACCS_LIBS})

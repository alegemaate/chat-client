cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project (ChatClient)

# Add sources
file(GLOB_RECURSE SOURCES ${CMAKE_BINARY_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_BINARY_DIR}/src/*.h)

# Create lib
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)

# Find libs
find_library(GameNetworkingLib NAMES GameNetworkingSockets.dll REQUIRED)

# Link Libs
target_link_libraries(${PROJECT_NAME} ${GameNetworkingLib})

cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# Set the project version and language
project(hekzam-marqueurs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")

include(FetchContent)

# Find the required dependencies
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package(ZXing QUIET)
if(NOT ZXing_FOUND)
    FetchContent_Declare(
        zxing
        GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
        GIT_TAG        v2.3.0
    )
    FetchContent_MakeAvailable(zxing)
    file(GLOB_RECURSE ZXING_HEADERS ${zxing_SOURCE_DIR}/core/src/*.h)
    file(COPY ${ZXING_HEADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ZXing)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()
find_package(nlohmann_json REQUIRED)

# Add the source files
set(SRC
    src/expl_pars.cpp
)

set(SRC_BENCH
    src/benchmark.cpp
)

file(GLOB_RECURSE UTILS_SRC src/utils/*.cpp)
file(GLOB_RECURSE PARSER_SRC src/parser/*.cpp)

set(INCLUDE_DIRS
    src
    src/utils
    src/parser
    "include"
)

include_directories(parser PRIVATE ${INCLUDE_DIRS})
include_directories(benchmark PRIVATE ${INCLUDE_DIRS})

# Create the executable
add_executable(parser ${SRC} ${UTILS_SRC} ${PARSER_SRC})
add_executable(benchmark ${SRC_BENCH} ${UTILS_SRC} ${PARSER_SRC})

# Link the dependencies
target_link_libraries(parser
    PRIVATE
        ${OpenCV_LIBS}
        ZXing::ZXing
        nlohmann_json::nlohmann_json
)

target_link_libraries(benchmark
    PRIVATE
        ${OpenCV_LIBS}
        ZXing::ZXing
        nlohmann_json::nlohmann_json
)
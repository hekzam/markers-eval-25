cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# Set the project version and language
project(hekzam-marqueurs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")

include(FetchContent)

option(ENABLE_ZBAR OFF)
option(AUTO_DOWNLOAD_ZXING "Automatically download ZXing if not found" OFF)

# Find the required dependencies
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
if(ENABLE_ZBAR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZBAR REQUIRED zbar)
    include_directories(${ZBAR_INCLUDE_DIRS})
    add_definitions(-DENABLE_ZBAR)
else()
    find_package(ZXing QUIET)
    if (NOT ZXing_FOUND AND NOT AUTO_DOWNLOAD_ZXING)
        message(FATAL_ERROR "ZXing not found. Please install it or set AUTO_DOWNLOAD_ZXING to ON.")
    endif()
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
endif()

find_package(nlohmann_json REQUIRED)

# Add the source files
set(SRC
    src/expl_pars.cpp
    src/external-tools/create_copy.cpp
)

set(SRC_BENCH
    src/benchmark.cpp
    src/utils/benchmark_helper.cpp
    src/external-tools/create_copy.cpp
    src/external-tools/modifier.cpp
)

set(SRC_MODIFIEUR
    src/modifier_cli.cpp
    src/utils/math_utils.cpp
    src/external-tools/modifier.cpp
)

set(SRC_CREATE_COPY
    src/gen_copies.cpp
    src/external-tools/create_copy.cpp
)

file(GLOB UTILS_SRC src/utils/*.cpp)
file(GLOB PARSER_SRC src/parser/*.cpp)
file(GLOB BENCHMARK_SRC src/bench/*.cpp)

# add debug definitions
if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    add_definitions(-DDEBUG)
endif()

set(INCLUDE_DIRS
    src
    src/utils
    src/parser
    src/external-tools
    "include"
)
include_directories(parser PRIVATE ${INCLUDE_DIRS})
include_directories(bench PRIVATE ${INCLUDE_DIRS})
include_directories(create-copy PRIVATE ${INCLUDE_DIRS})
include_directories(modifier PRIVATE ${INCLUDE_DIRS})

# Create the executable
add_executable(parser ${SRC} ${UTILS_SRC} ${PARSER_SRC})
add_executable(bench ${SRC_BENCH} ${UTILS_SRC} ${PARSER_SRC} ${BENCHMARK_SRC})
add_executable(create-copy ${SRC_CREATE_COPY})
add_executable(modifier ${SRC_MODIFIEUR})

# Link the dependencies
target_link_libraries(parser
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

target_link_libraries(bench
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

target_link_libraries(create-copy
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

target_link_libraries(modifier
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

if(ENABLE_ZBAR)
target_link_libraries(parser PRIVATE ${ZBAR_LINK_LIBRARIES})
target_link_libraries(bench PRIVATE ${ZBAR_LINK_LIBRARIES})
target_link_libraries(create-copy PRIVATE ${ZBAR_LINK_LIBRARIES})
else()
target_link_libraries(parser PRIVATE ZXing::ZXing)
target_link_libraries(bench PRIVATE ZXing::ZXing)
target_link_libraries(create-copy PRIVATE ZXing::ZXing)
endif()
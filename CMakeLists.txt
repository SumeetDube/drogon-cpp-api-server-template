cmake_minimum_required(VERSION 3.23)
project(ApiServer LANGUAGES CXX)

# ---------------------------
# Options
# ---------------------------
option(BUILD_TESTS "Build the unit tests (needs GoogleTest)" OFF)
set(SANITIZER "" CACHE STRING "Build with a sanitizer: address (ASan+UBSan) or thread (TSan)")

# ---------------------------
# Compiler configuration
# ---------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O1")
endif()

if(SANITIZER STREQUAL "address")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
elseif(SANITIZER STREQUAL "thread")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
elseif(NOT SANITIZER STREQUAL "")
    message(FATAL_ERROR "Unknown SANITIZER '${SANITIZER}' (use address or thread)")
endif()

# ---------------------------
# Dependencies
# ---------------------------
find_package(Drogon CONFIG REQUIRED)
find_package(reflectcpp REQUIRED)
find_package(yaml-cpp REQUIRED)
# zlib (gzip compression) is Drogon's own dependency; Drogon::Drogon already links it.

# ---------------------------
# Library: everything except main(), so tests can link it.
# OBJECT, not STATIC: Drogon controllers register themselves from static initializers
# that nothing references, so a static archive would let the linker drop them (all 404s).
# ---------------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cpp"
)
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main.cpp")

add_library(api_core OBJECT ${SRC_FILES})

target_include_directories(api_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
)

target_link_libraries(api_core PUBLIC
    Drogon::Drogon
    yaml-cpp::yaml-cpp
    reflectcpp::reflectcpp
)

target_compile_options(api_core PRIVATE -Wall -Wextra)

# ---------------------------
# Server executable
# ---------------------------
add_executable(${PROJECT_NAME} app/src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE api_core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# ---------------------------
# Tests
# ---------------------------
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

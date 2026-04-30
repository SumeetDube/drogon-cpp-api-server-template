cmake_minimum_required(VERSION 3.23)
project(ApiServer LANGUAGES CXX)

# ---------------------------
# Compiler configuration
# ---------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O1")
endif()

# ---------------------------
# Dependencies
# ---------------------------
find_package(Drogon CONFIG REQUIRED)
find_package(reflectcpp REQUIRED)
find_package(yaml-cpp REQUIRED)

# ---------------------------
# Sources
# ---------------------------
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Drogon::Drogon
    yaml-cpp::yaml-cpp
    reflectcpp::reflectcpp
    ZLIB::ZLIB
)

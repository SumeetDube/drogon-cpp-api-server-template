find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(unit_tests
    ThreadPoolTest.cc
    ExceptionHandlerTest.cc
    RequestConvertTest.cc
)

target_link_libraries(unit_tests PRIVATE api_core GTest::gtest_main)
target_compile_options(unit_tests PRIVATE -Wall -Wextra)

gtest_discover_tests(unit_tests)

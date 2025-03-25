cmake_minimum_required(VERSION 3.30)
project("TaskWeaver.test")

find_package(GTest REQUIRED)
enable_testing()

file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${PROJECT_NAME} ${TEST_HEADERS} ${TEST_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS OFF

    DEBUG_POSTFIX _d
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -pedantic
        -Wall
        -Wextra
        -Wfatal-errors
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Wall
    )
endif()

target_link_libraries(${PROJECT_NAME} gtest gtest_main -pthread)

target_compile_definitions(${PROJECT_NAME} PRIVATE TESTING)

add_test(NAME taskweaver-test COMMAND ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} TaskWeaver)


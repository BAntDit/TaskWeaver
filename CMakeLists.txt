cmake_minimum_required(VERSION 3.30)
project(TaskWeaver VERSION 0.0.1.0)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

include(ClangFormat.cmake)
set(ALL_FILES_TO_ADJUST_CODESTYLE ${SOURCE_FILES} ${HEADER_FILES})
CODE_STYLE_CORRECTION("${ALL_FILES_TO_ADJUST_CODESTYLE}")

add_library(TaskWeaver
        SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS OFF

        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"

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

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

find_package(GTest REQUIRED)

enable_testing()
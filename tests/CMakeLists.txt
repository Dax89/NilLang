cmake_minimum_required(VERSION 3.25)

set(PARENT_TARGET "${PROJECT_NAME_LIB}")

project(CmakeConfigPackageTests C)

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    find_package(${PARENT_TARGET} CONFIG REQUIRED)

    if(NOT TARGET ${PARENT_TARGET}_options)
        message(FATAL_ERROR "Required config package not found!")
        return()
    endif()
endif()

file(COPY "samples" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(tests)
setup_compiler(tests)

target_sources(tests 
    PRIVATE
        main.c
)

target_link_libraries(tests
    PRIVATE
        ${PARENT_TARGET}
)

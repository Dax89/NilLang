cmake_minimum_required(VERSION 3.25)

project(NilLang C)

if(EXISTS src/main.c)
    set(PROJECT_NAME_LIB "${PROJECT_NAME}_lib")
else()
    set(PROJECT_NAME_LIB "${PROJECT_NAME}")
endif()

add_library(${PROJECT_NAME_LIB} STATIC)

target_sources(${PROJECT_NAME_LIB} 
    PRIVATE
        src/nil.c
        src/utils.c
        src/stack.c
        src/heap.c
        src/stringtable.c
        src/lexer/lexer.c
        src/compiler/bytecode.c
        src/compiler/compiler.c
        src/compiler/common.c
        src/compiler/intrinsics/compare.c
        src/compiler/intrinsics/conditions.c
        src/compiler/intrinsics/controlflow.c
        src/compiler/intrinsics/intrinsics.c
        src/compiler/intrinsics/math.c
        src/compiler/intrinsics/memory.c
        src/compiler/intrinsics/procedural.c
        src/compiler/intrinsics/stack.c
        src/vm/vm.c
        src/vm/runtime.c
        src/vm/memory.c
        src/vm/disasm.c
)

target_include_directories(${PROJECT_NAME_LIB}
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
)

include(cmake/Compiler.cmake)
setup_compiler(${PROJECT_NAME_LIB})

if(EXISTS src/main.c)
    add_library(${PROJECT_NAME}::lib ALIAS ${PROJECT_NAME_LIB})
    add_executable(${PROJECT_NAME})

    target_sources(${PROJECT_NAME}
        PRIVATE 
            src/main.c
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}::lib)
    setup_compiler(${PROJECT_NAME})
endif()

if(NOT PROJECT_IS_TOP_LEVEL)
    return()
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    configure_file(
        "${CMAKE_SOURCE_DIR}/lsan.supp"
        "${CMAKE_BINARY_DIR}/lsan.supp"
        COPYONLY
    )
    configure_file(
        "${CMAKE_SOURCE_DIR}/asan.supp"
        "${CMAKE_BINARY_DIR}/asan.supp"
        COPYONLY
    )
endif()

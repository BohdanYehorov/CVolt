cmake_minimum_required(VERSION 4.1)
project(CVolt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LLVM REQUIRED CONFIG)

set(SOURCES
        Src/Lexer.cpp
        Src/Arena.cpp
        Src/Parser.cpp
        Src/Operator.cpp
        Src/LLVMCompiler.cpp
        Src/ParserFuzzer.cpp
        Src/TypeChecker.cpp
        Src/BuiltinFunctionTable.cpp
        Src/ExprResult.cpp
        Src/CompilationContext.cpp
        Src/Token.cpp
        Src/LexError.cpp
        Src/ParseError.cpp
        Src/TypeError.cpp
        Src/JITEngine.cpp
        Src/DataType.cpp
        Src/IRValue.cpp
        Src/ExprAddress.cpp
        Src/DebugOutput.cpp
        Src/ClassType.cpp
        Src/ClassInst.cpp
        Src/VoltClass.cpp
)

if (COMPILE_TO_LIB)
    add_library(CVolt STATIC ${SOURCES})
else ()
    add_executable(CVolt Src/main.cpp ${SOURCES})
endif ()

target_include_directories(CVolt PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Include
)

target_compile_definitions(CVolt PRIVATE ${LLVM_DEFINITIONS})

target_precompile_headers(CVolt PRIVATE
        <llvm/IR/IRBuilder.h>
        <llvm/IR/Module.h>
        <llvm/IR/LLVMContext.h>
)

target_link_libraries(CVolt PRIVATE LLVM)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT COMPILE_TO_LIB)
    target_compile_options(CVolt PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(CVolt PRIVATE -fsanitize=address,undefined)
endif ()
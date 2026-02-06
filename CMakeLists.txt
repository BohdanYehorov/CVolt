cmake_minimum_required(VERSION 4.1)
project(CVolt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(LLVM REQUIRED CONFIG)

include_directories(
        ${LLVM_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Include
)

add_definitions(${LLVM_DEFINITIONS})

add_executable(CVolt
        Src/main.cpp
        Src/Lexer.cpp
        Src/Arena.cpp
        Src/Parser.cpp
        Src/Operator.cpp
        Src/Errors.cpp
        Src/BufferView.cpp
        Src/LLVMCompiler.cpp
        Src/Hash.cpp
        Src/DataTypeUtils.cpp
        Src/DataTypeHash.cpp
        Src/FunctionSignatureHash.cpp
        Src/ParserFuzzer.cpp
        Src/TypeChecker.cpp
        Src/BuiltinFunctionTable.cpp
        Src/CTimeValue.cpp
        Src/CompilationContext.cpp
        Src/Token.cpp
)

target_link_libraries(CVolt PRIVATE LLVM)
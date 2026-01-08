cmake_minimum_required(VERSION 4.1)
project(CVolt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "/home/bohdan/llvm/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

include_directories(${CMAKE_SOURCE_DIR}/Include)

add_executable(CVolt Src/main.cpp
        Src/Lexer.cpp
        Include/Lexer.h
        Include/ASTNodes.h
        Include/Object.h
        Src/Arena.cpp
        Include/Arena.h
        Src/Parser.cpp
        Include/Parser.h
        Src/Operator.cpp
        Include/Operator.h
        Src/Errors.cpp
        Include/Errors.h
        Include/TypeDefs.h
        Include/BufferView.h
        Src/BufferView.cpp
        Include/CompilerTypes.h
        Include/DataType.h
        Src/LLVMCompiler.cpp
        Include/LLVMCompiler.h
        Include/DefaultFunction.h
        Src/Hash.cpp
        Include/Hash.h
        Src/Hash.cpp
        Include/Hash.h
        Include/FunctionSignature.h
        Src/TypedValue.cpp
        Include/TypedValue.h
)

target_link_libraries(CVolt PRIVATE LLVM edit xml2)
include_directories(${LLVM_INCLUDE_DIRS})

set_target_properties(CVolt PROPERTIES
        BUILD_RPATH "/home/bohdan/llvm/lib"
        INSTALL_RPATH "/home/bohdan/llvm/lib"
)
cmake_minimum_required(VERSION 4.1)
project(CVolt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "/home/bohdan/llvm/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

include_directories(${CMAKE_SOURCE_DIR}/Include)

add_executable(CVolt Src/main.cpp
        Src/Lexer.cpp
        Include/Volt/Core/Lexer/Lexer.h
        Include/Volt/Core/AST/ASTNodes.h
        Include/Volt/Core/Object/Object.h
        Src/Arena.cpp
        Include/Volt/Core/Memory/Arena.h
        Src/Parser.cpp
        Include/Volt/Core/Parser/Parser.h
        Src/Operator.cpp
        Include/Volt/Core/Parser/Operators/Operator.h
        Src/Errors.cpp
        Include/Volt/Core/Errors/Errors.h
        Include/Volt/Core/Types/TypeDefs.h
        Include/Volt/Core/Memory/BufferView.h
        Src/BufferView.cpp
        Include/Volt/Compiler/Types/CompilerTypes.h
        Src/LLVMCompiler.cpp
        Include/Volt/Compiler/LLVMCompiler.h
        Include/Volt/Compiler/Functions/DefaultFunctions.h
        Src/Hash.cpp
        Include/Volt/Compiler/Hash/Hash.h
        Src/Hash.cpp
        Include/Volt/Compiler/Hash/Hash.h
        Include/Volt/Compiler/Functions/FunctionSignature.h
        Include/Volt/Compiler/Types/TypedValue.h
        Src/DataType.cpp
        Include/Volt/Compiler/Types/DataType.h
        Include/Volt/Compiler/Types/DataTypeBase.h
        Include/Volt/Compiler/Hash/DataTypeHash.h
        Include/Volt/Compiler/Hash/FunctionSignatureHash.h
        Src/DataTypeHash.cpp
        Src/FunctionSignatureHash.cpp
        Include/Volt/Tests/Fuzzer/ParserFuzzer.h
        Src/ParserFuzzer.cpp
)

target_link_libraries(CVolt PRIVATE LLVM edit xml2)
include_directories(${LLVM_INCLUDE_DIRS})

set_target_properties(CVolt PROPERTIES
        BUILD_RPATH "/home/bohdan/llvm/lib"
        INSTALL_RPATH "/home/bohdan/llvm/lib"
)
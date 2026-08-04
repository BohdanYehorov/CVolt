#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctions.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Runtime/JITEngine/JITEngine.h"
#include "Volt/ADT/String.h"
#include "Volt/Debug/DebugOutput/DebugOutput.h"
#include <fstream>
#include <sstream>
#include <Volt/Tests/Fuzzer/ParserFuzzer.h>

#include "Volt/Core/Functions/FunctionTable.h"

int main(int Argc, char* Argv[])
{
    Volt::JITEngine::Init();

#ifdef _DEBUG
    std::ifstream File("../VoltFiles/test.volt");
    if (!File.is_open())
        return -1;

    std::stringstream SStr;
    SStr << File.rdbuf();
#else
    if (Argc < 2)
    {
        std::cerr << "Error: No input files\n";
        return -1;
    }

    std::ifstream File(Argv[1]);
    if (!File.is_open())
    {
        std::cerr << "Error: Cannot open this file: '" << Argv[1] << "'" << std::endl;
        return -1;
    }

    std::stringstream SStr;
    SStr << File.rdbuf();
#endif

    Volt::CompilationContext CContext(SStr.str().c_str(), "test.volt");
    Volt::DebugOutput DebugOutput(llvm::outs(), llvm::errs(), CContext);

    Volt::BuiltinFunctionTable FuncTable(CContext);

    FuncTable.AddFunctionOverloads("Out",
        Out<bool>, Out<char>, Out<Volt::Int8>, Out<Volt::Int16>,
        Out<Volt::Int32>, Out<Volt::Int64>, Out<Volt::UInt8>, Out<Volt::UInt16>,
        Out<Volt::UInt32>, Out<Volt::UInt64>, Out<const char*>, Out<float>,
        Out<double>, Out<void*>
    );

    FuncTable.AddFunctionOverloads("OutLine",
        OutLine<bool>, OutLine<char>, OutLine<Volt::Int8>, OutLine<Volt::Int16>,
        OutLine<Volt::Int32>, OutLine<Volt::Int64>, OutLine<Volt::UInt8>, OutLine<Volt::UInt16>,
        OutLine<Volt::UInt32>, OutLine<Volt::UInt64>, OutLine<const char*>, OutLine<float>,
        OutLine<double>, OutLine<void*>
    );

    FuncTable.AddFunctionOverloads("In",
        In<bool>, In<char>, In<Volt::Int8>, In<Volt::Int16>,
        In<Volt::Int32>, In<Volt::Int64>, In<Volt::UInt8>, In<Volt::UInt16>,
        In<Volt::UInt32>, In<Volt::UInt64>, In<float>, In<double>
    );

    FuncTable.AddFunction("Time", Time);
    FuncTable.AddFunction("Sqrt", Sqrt);
    FuncTable.AddFunction("Sin", Sin);
    FuncTable.AddFunction("Cos", Cos);
    FuncTable.AddFunction("Tan", Tan);
    FuncTable.AddFunction("RandomInt", RandomInt);
    FuncTable.AddFunction("System", System);
    FuncTable.AddFunction("MemAlloc", MemAlloc);
    FuncTable.AddFunction("Realloc", Realloc);
    FuncTable.AddFunction("MemFree", MemFree);
    FuncTable.AddFunction("MemCpy", MemCpy);

    Volt::FunctionTable FTable;

    for (const auto& [Name, Overload] : FTable)
    {
        std::cout << Name.str() << '(';
        for (Volt::QualType Arg : Overload.Args)
            std::cout << Arg.ToString() << ", ";
        std::cout << ")\n";
    }

    Volt::Lexer MyLexer(CContext);
    MyLexer.Lex();

#ifdef _DEBUG
    DebugOutput.WriteTokens();
#endif

    Volt::Parser MyParser(CContext);
    MyParser.Parse();

    Volt::TypeChecker MyTypeChecker(CContext, FuncTable);
    MyTypeChecker.Check();

#ifdef _DEBUG
    DebugOutput.WriteAST();
#endif

    if (CContext.HasErrors())
    {
        DebugOutput.WriteLexErrors();
        DebugOutput.WriteParseErrors();
        DebugOutput.WriteTypeErrors();
        return -1;
    }

    Volt::LLVMCompiler MyCompiler(CContext, FuncTable);
    MyCompiler.Compile();

#ifdef _DEBUG
    DebugOutput.WriteIR();
#endif

#ifdef _DEBUG
    std::cout << "=======================Output=======================\n\n";
#endif

    Volt::JITEngine Engine(CContext, FuncTable);
    auto Res = Engine.CallFunction<Volt::Int32>("Main");

#ifdef _DEBUG
    std::cout << "\n====================================================\n";
#endif

    return Res;
}

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctions.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Runtime/JITEngine/JITEngine.h"
#include "Volt/ADT/String.h"
#include "Volt/Debug/DebugOutput/DebugOutput.h"
#include "Volt/Core/Types/ClassInst.h"
#include <fstream>
#include <sstream>
#include <Volt/Tests/Fuzzer/ParserFuzzer.h>

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
    Volt::DebugOutput DebugOutput(llvm::outs(), CContext);

    Volt::BuiltinFunctionTable FuncTable(CContext);

    FuncTable.AddFunctionOverloads("Out",
        OutBool, OutChar, OutI8, OutI16,
        OutI32, OutI64, OutU8, OutU16,
        OutU32, OutU64, OutStr, OutFloat,
        OutDouble, OutPtr
    );

    FuncTable.AddFunctionOverloads("In", InInt, InIntWithLabel);
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
    Engine.CallFunction<Volt::UInt32>("Main");

#ifdef _DEBUG
    std::cout << "\n====================================================\n";
#endif

    return 0; //Res;
}
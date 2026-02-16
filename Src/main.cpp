#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctions.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Runtime/JITEngine/JITEngine.h"
#include "Volt/ADT/String.h"
#include <fstream>
#include <sstream>

int main(int Argc, char* Argv[])
{
#ifdef _DEBUG
    std::ifstream File("../VoltFiles/test.volt");
    if (!File.is_open())
        return -1;

    std::stringstream SStr;
    SStr << File.rdbuf();

    Volt::JITEngine::Init();

    Volt::CompilationContext CContext(SStr.str().c_str(), "test.volt");
    Volt::BuiltinFunctionTable FuncTable(CContext);
    FuncTable.AddFunction("Out", "OutBool", &OutBool);
    FuncTable.AddFunction("Out", "OutChar", &OutChar);
    FuncTable.AddFunction("Out", "OutByte", &OutByte);
    FuncTable.AddFunction("Out", "OutInt", &OutInt);
    FuncTable.AddFunction("Out", "OutLong", &OutLong);
    FuncTable.AddFunction("Out", "OutStr", &OutStr);
    FuncTable.AddFunction("Out", "OutFloat", &OutFloat);
    FuncTable.AddFunction("Out", "OutDouble", &OutDouble);
    FuncTable.AddFunction("Out", "OutPtr", &OutPtr);
    FuncTable.AddFunction("In", "InInt", &InInt);
    FuncTable.AddFunction("In", "InIntWithLabel", &InIntWithLabel);
    FuncTable.AddFunction("Time", "Time", &Time);
    FuncTable.AddFunction("Sin", "Sin", &Sin);
    FuncTable.AddFunction("Cos", "Cos", &Cos);
    FuncTable.AddFunction("Tan", "Tan", &Tan);
    FuncTable.AddFunction("RandomInt", "RandomInt", &RandomInt);
    FuncTable.AddFunction("System", "System", &System);
    FuncTable.AddFunction("MemAlloc", "MemAlloc", &MemAlloc);
    FuncTable.AddFunction("MemFree", "MemFree", &MemFree);
    FuncTable.AddFunction("MemCpy", "MemCpy", &MemCpy);

    Volt::Lexer MyLexer(CContext);
    MyLexer.Lex();
    MyLexer.PrintTokens();

    if (MyLexer.PrintErrors())
        return -1;

    Volt::Parser MyParser(CContext);
    MyParser.Parse();
    MyParser.PrintASTTree();

    if (MyParser.PrintErrors())
        return -1;

    Volt::TypeChecker MyTypeChecker(CContext, FuncTable);
    MyTypeChecker.Check();

    if (MyTypeChecker.PrintErrors())
        return -1;

    MyParser.PrintASTTree();

    Volt::LLVMCompiler MyCompiler(CContext, FuncTable);
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    Volt::JITEngine Engine(CContext, FuncTable);
    int Res = Engine.CallFunction<int>("Main");

    std::cout << "\n====================================================\n";

    return Res;

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

    Volt::CompilationContext CContext(SStr.str().c_str());

    Volt::Lexer MyLexer(CContext);
    MyLexer.Lex();

    if (MyLexer.PrintErrors())
        return -1;

    Volt::Parser MyParser(CContext);
    MyParser.Parse();

    if (MyParser.PrintErrors())
        return -1;

    Volt::BuiltinFunctionTable FuncTable(CContext);
    FuncTable.AddFunction("Out", "OutBool", &OutBool);
    FuncTable.AddFunction("Out", "OutChar", &OutChar);
    FuncTable.AddFunction("Out", "OutByte", &OutByte);
    FuncTable.AddFunction("Out", "OutInt", &OutInt);
    FuncTable.AddFunction("Out", "OutLong", &OutLong);
    FuncTable.AddFunction("Out", "OutStr", &OutStr);
    FuncTable.AddFunction("Out", "OutFloat", &OutFloat);
    FuncTable.AddFunction("Out", "OutDouble", &OutDouble);
    FuncTable.AddFunction("In", "InInt", &InInt);
    FuncTable.AddFunction("In", "InIntWithLabel", &InIntWithLabel);
    FuncTable.AddFunction("Time", "Time", &Time);
    FuncTable.AddFunction("Sin", "Sin", &Sin);
    FuncTable.AddFunction("Cos", "Cos", &Cos);
    FuncTable.AddFunction("Tan", "Tan", &Tan);
    FuncTable.AddFunction("RandomInt", "RandomInt", &RandomInt);
    FuncTable.AddFunction("System", "System", &System);

    Volt::TypeChecker MyTypeChecker(CContext, FuncTable);
    MyTypeChecker.Check();

    if (MyTypeChecker.PrintErrors())
        return -1;

    Volt::LLVMCompiler MyCompiler(CContext, FuncTable, MyTypeChecker.GetFunctions());
    MyCompiler.Compile();
    return MyCompiler.Run();
#endif
}
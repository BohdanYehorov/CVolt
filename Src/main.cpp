#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctions.h"
#include <fstream>
#include <sstream>

int main(int Argc, char* Argv[])
{
    Volt::Arena MainArena;
    Volt::BuilderBase Builder(MainArena);
    Volt::BuiltinFunctionTable FuncTable(Builder);
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

#ifdef _DEBUG
    std::ifstream File("../Resources/test.volt");
    if (!File.is_open())
        return -1;

    std::stringstream SStr;
    SStr << File.rdbuf();

    Volt::Lexer MyLexer(SStr.str());
    MyLexer.Lex();
    MyLexer.PrintTokens();

    if (MyLexer.PrintErrors())
        return -1;

    Volt::Parser MyParser(MainArena, MyLexer);
    MyParser.Parse();
    MyParser.PrintASTTree();

    if (MyParser.PrintErrors())
        return -1;

    Volt::TypeChecker MyTypeChecker(MyParser, MainArena, Builder, FuncTable);
    MyTypeChecker.Check();

    if (MyTypeChecker.PrintErrors())
        return -1;

    MyParser.PrintASTTree();

    Volt::LLVMCompiler MyCompiler(MainArena, MyTypeChecker);
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    int Res = MyCompiler.Run();

    std::cout << "\n====================================================\n";
    std::cout << "Exited With Code: " << Res << std::endl;

    // Volt::DataTypeBase* I32Type = Builder.GetIntegerType(32);
    // Volt::DataTypeBase* I64Type = Builder.GetIntegerType(64);
    //
    // // std::cout << std::boolalpha << (I32Type == I64Type) << std::endl;
    // // std::cout << Volt::DataTypeHash{}(I32Type) << " " << Volt::DataTypeHash{}(I64Type) << std::endl;
    //
    // Volt::FunctionSignature Signature1{ "Hello", {I32Type} };
    // Volt::FunctionSignature Signature2{ "Hello", {I64Type} };
    //
    // std::cout << std::boolalpha << (Signature1 == Signature2) << std::endl;

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

    Volt::Lexer MyLexer(SStr.str());
    MyLexer.Lex();

    if (MyLexer.PrintErrors())
        return -1;

    Volt::Parser MyParser(MainArena, MyLexer);
    MyParser.Parse();

    if (MyParser.PrintErrors())
        return -1;

    Volt::TypeChecker MyTypeChecker(MyParser, MainArena, FuncTable);
    MyTypeChecker.Check();

    if (MyTypeChecker.PrintErrors())
        return -1;

    Volt::LLVMCompiler MyCompiler(MainArena, MyTypeChecker);
    MyCompiler.Compile();
    int Res = MyCompiler.Run();
    std::cout << "\nExited With Code: " << Res << std::endl;
#endif
    return 0;
}
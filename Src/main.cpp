#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctions.h"
#include <fstream>
#include <sstream>

int main()
{
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

    Volt::Arena MainArena;

    Volt::Parser MyParser(MainArena, MyLexer);
    MyParser.Parse();
    MyParser.PrintASTTree();

    if (MyParser.PrintErrors())
        return -1;

    Volt::BuiltinFunctionTable FuncTable(MainArena);
    FuncTable.AddFunction("Out", "OutBool", &OutBool);
    FuncTable.AddFunction("Out", "OutChar", &OutChar);
    FuncTable.AddFunction("Out", "OutByte", &OutByte);
    FuncTable.AddFunction("Out", "OutInt", &OutInt);
    FuncTable.AddFunction("Out", "OutLong", &OutLong);
    FuncTable.AddFunction("Out", "OutStr", &OutStr);
    FuncTable.AddFunction("Out", "OutFloat", &OutFloat);
    FuncTable.AddFunction("Out", "OutDouble", &OutDouble);
    FuncTable.AddFunction("Time", "Time", &Time);
    FuncTable.AddFunction("Sin", "Sin", &Sin);
    FuncTable.AddFunction("Cos", "Cos", &Cos);
    FuncTable.AddFunction("Tan", "Tan", &Tan);

    Volt::TypeChecker MyTypeChecker(MyParser, MainArena, FuncTable);
    MyTypeChecker.Check();

    if (MyTypeChecker.PrintErrors())
        return -1;

    Volt::LLVMCompiler MyCompiler(MainArena, MyParser.GetASTTree(), FuncTable);
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    int Res = MyCompiler.Run();

    std::cout << "\n====================================================\n";
    std::cout << "Exited With Code: " << Res << std::endl;

    return 0;
}
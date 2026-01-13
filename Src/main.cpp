#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/LLVMCompiler.h"
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

    Volt::Parser MyParser(MyLexer);
    MyParser.Parse();
    MyParser.PrintASTTree();

    if (MyParser.PrintErrors())
        return -1;

    Volt::LLVMCompiler MyCompiler(MyParser.GetASTTree());
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    int Res = MyCompiler.Run();

    std::cout << "\n====================================================\n";
    std::cout << "Exited With Code: " << Res << std::endl;

    return 0;
}
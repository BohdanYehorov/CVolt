#include <iostream>
#include "Object.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include "LLVMCompiler.h"

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

    if (MyLexer.HasErrors())
    {
        for (const auto& Err : MyLexer.GetErrors())
            std::cout << Err.ToString() << std::endl;

        return -1;
    }

    Volt::Parser MyParser(MyLexer);
    MyParser.Parse();

    const std::vector<Volt::ParseError>& ErrorList = MyParser.GetErrorList();
    if (!ErrorList.empty())
    {
        std::cout << "Errors:\n";
        for (const auto& Error : ErrorList)
            std::cout << Error.ToString() << std::endl;

        MyParser.PrintASTTree();
        return -1;
    }

    MyParser.PrintASTTree();

    Volt::LLVMCompiler MyCompiler(MyParser.GetASTTree());
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    int Res = MyCompiler.Run();

    std::cout << "\n====================================================\n";

    std::cout << "Exited With Code: " << Res << std::endl;

    return 0;
}
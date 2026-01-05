#include <iostream>
#include "../Include/Object.h"
#include "../Include/Parser.h"
#include "../Include/Compiler.h"
#include <fstream>
#include <sstream>
#include "../Include/LLVMCompiler.h"
#include "../Include/BufferView.h"

int main()
{
    std::ifstream File("../test.volt");
    if (!File.is_open())
        return -1;

    std::stringstream SStr;
    SStr << File.rdbuf();

    Lexer MyLexer(SStr.str());
    MyLexer.Lex();
    MyLexer.PrintTokens();

    if (MyLexer.HasErrors())
    {
        for (const auto& Err : MyLexer.GetErrors())
        {
            std::cout << Err.ToString() << std::endl;
        }
    }

    Parser MyParser(MyLexer);
    MyParser.Parse();

    const std::vector<ParseError> ErrorList = MyParser.GetErrorList();
    if (!ErrorList.empty())
    {
        std::cout << "Errors:\n";
        for (const auto& Error : ErrorList)
            std::cout << Error.ToString() << std::endl;
    }

    MyParser.PrintASTTree();

    // Compiler MyCompiler(MyParser);
    // MyCompiler.Compile();
    // MyCompiler.PrintVariables();
    // MyCompiler.PrintSections();

    LLVMCompiler MyCompiler(MyParser.GetASTTree());
    MyCompiler.Compile();
    MyCompiler.Print();

    std::cout << "=======================Output=======================\n\n";

    int Res = MyCompiler.Run();

    std::cout << "\n====================================================\n";

    std::cout << "Exited With Code: " << Res << std::endl;

    return 0;
}
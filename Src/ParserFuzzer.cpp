//
// Created by bohdan on 14.01.26.
//

#include <Volt/Tests/Fuzzer/ParserFuzzer.h>
#include <thread>

namespace Volt
{
    std::mt19937 ParserFuzzer::Gen{ std::random_device{}() };

    std::string ParserFuzzer::GenExpr()
    {
        if (TabsCount < MaxDepth)
        {
            int Operation = RandomInt(0, 4);
            switch (Operation)
            {
                case 0:
                    return GenBinaryExpr() + ";\n";
                case 1:
                    return GenVarDeclExpr() + ";\n";
                case 2:
                    return GenIf() + "\n";
                case 3:
                    return GenWhile() + "\n";
                case 4:
                    return GenFor() + "\n";
                default:
                    return "?";
            }
        }

        int Operation = RandomInt(0, 1);
        switch (Operation)
        {
            case 0:
                return GenBinaryExpr() + ";\n";
            case 1:
                return GenVarDeclExpr() + ";\n";
            default:
                return "?";
        }
    }

    std::string ParserFuzzer::GenBlock()
    {
        if (TabsCount > MaxDepth)
            return "\n";

        std::string Block = Tabs + "{\n";
        int ExprCount = RandomInt(1, MaxDepth);
        bool OldInBlock = InBlock;
        InBlock = true;
        Tabs.push_back('\t');
        TabsCount++;

        while (ExprCount--)
        {
            std::string Expr = GenExpr();
            if (Expr.empty())
                continue;

            Block += Tabs + Expr;
        }

        TabsCount--;
        Tabs.pop_back();
        InBlock = OldInBlock;
        return Block + Tabs + "}";
    }

    std::string ParserFuzzer::GenFunctionDeclExpr()
    {
        InFunction = true;
        std::string Func = "fun: int " + GenIdentifier() + "() \n" + GenBlock();
        InFunction = false;

        return Func;
    }

    std::string ParserFuzzer::GenVarDeclExpr()
    {
        return "let: int " + GenIdentifier() + (RandomInt(0, 1) == 0 ? " = " + GenBinaryExpr() : "");
    }

    std::string ParserFuzzer::GenIf()
    {
        std::string IfExpr = "if (" + GenBinaryExpr() + ")\n" + GenBlock();
        return IfExpr;
    }

    std::string ParserFuzzer::GenWhile()
    {
        std::string WhileExpr = "while (" + GenBinaryExpr() + ")\n" + GenBlock();
        return WhileExpr;
    }

    std::string ParserFuzzer::GenFor()
    {
        std::string WhileExpr = "for (" + GenVarDeclExpr() + "; " +
            GenBinaryExpr() + "; " + GenBinaryExpr() + ")\n" + GenBlock();
        return WhileExpr;
    }

    std::string ParserFuzzer::GenGlobalExpr()
    {

        int Operation = RandomInt(0, 1);
        switch (Operation)
        {
            case 0:
                return GenFunctionDeclExpr() + "\n";
            case 1:
                return GenVarDeclExpr() + ";\n";
            default:
                return "?";
        }
    }

    void ParserFuzzer::StartTest(size_t Tests)
    {
        for (size_t i = 0; i < Tests; i++)
        {
            std::string Expr = GenGlobalExpr();
            OutFile.open("../Tests/GenCode/" + std::to_string(i) + ".volt");
            OutFile.write(Expr.c_str(), Expr.size());
            OutFile.close();

            try
            {
                // Lexer TestLexer(Expr);
                // TestLexer.Lex();
                //
                // if (TestLexer.HasErrors())
                // {
                //     OutFile.open("../Tests/LexErrors/" + std::to_string(i) + ".errs");
                //     for (const LexError& Error : TestLexer.GetErrors())
                //     {
                //         OutFile << "LexError: " << Error.ToString() << " At position: [" <<
                //             Error.Line << ":" << Error.Column << "]\n";
                //     }
                //     OutFile.close();
                //     std::cout << "Test " << i << ": LexErrors\n";
                //     continue;
                // }
                //
                // OutFile.open("../Tests/Tokens/" + std::to_string(i) + ".tok");
                // TestLexer.WriteErrors(OutFile);
                // OutFile.close();
                //
                // Arena MainArena;
                // Parser TestParser(MainArena, TestLexer);
                // TestParser.Parse();
                //
                // if (TestParser.HasErrors())
                // {
                //     OutFile.open("../Tests/ParseErrors/" + std::to_string(i) + ".errs");
                //     for (const ParseError& Error : TestParser.GetErrorList())
                //     {
                //         OutFile << "ParseError: " << Error.ToString() << " At position: [" <<
                //             Error.Line << ":" << Error.Column << "]\n";
                //     }
                //     OutFile.close();
                //     std::cout << "Test " << i << ": ParseErrors\n";
                //     continue;
                // }
                //
                // OutFile.open("../Tests/AST/" + std::to_string(i) + ".ast");
                // TestParser.WriteASTTree(OutFile);
                // OutFile.close();
                //
                // std::cout << "Test " << i << ": Success\n";
            }
            catch (const std::exception& Ex)
            {
                std::cout << "Test " << i << ": Exception: '" << Ex.what() << "'\n";
            }
            catch (...)
            {
                std::cout << "Test " << i << ": Unknown exception\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::string ParserFuzzer::GenBinaryExpr()
    {
        static std::string Operators[] = { "+", "-", "*", "/", "%" };

        std::string Prefix = GenPrefixUnary();
        int Depth = RandomInt(0, MaxDepth);

        for (size_t i = 0; i < Depth; i++)
        {
            int Operation = RandomInt(0, std::size(Operators));
            Prefix += " " + Operators[Operation] + " " + GenPrefixUnary();
        }
        return Prefix;
    }

    std::string ParserFuzzer::GenPrefixUnary()
    {
        static std::string Operators[] = { "+", "-", "!", "++", "--", "$" };
        std::string Suffix = GenSuffixUnary();

        int Depth = RandomInt(0, MaxDepth);
        for (size_t i = 0; i < Depth; i++)
        {
            int Operation = RandomInt(0, std::size(Operators) - 1);
            Suffix = " " + Operators[Operation] + Suffix;
        }

        return Suffix;
    }

    std::string ParserFuzzer::GenSuffixUnary()
    {
        static std::string Operators[] = { "++", "--" };

        std::string Primary = GenPrimary();

        int Depth = RandomInt(0, MaxDepth);
        for (size_t i = 0; i < Depth; i++)
        {
            int Operation = RandomInt(0, std::size(Operators));
            if (Operation < std::size(Operators))
                Primary += Operators[Operation] + " ";
        }

        return Primary;
    }

    std::string ParserFuzzer::GenPrimary()
    {
        int Primary = RandomInt(0, 5);
        switch (Primary)
        {
            case 0: return GenIdentifier();
            case 1: return GenByte();
            case 2: return GenInt();
            case 3: return GenLong();
            case 4: return GenFloat();
            case 5: return GenDouble();
            default: return "?";
        }
    }

    std::string ParserFuzzer::GenIdentifier()
    {
        static std::string Chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        int Len = RandomInt(1, 10);

        std::string Result;
        for (int i = 0; i < Len; i++)
        {
            char Chr = Chars[RandomInt(0, Chars.size() - 1)];
            Result.push_back(Chr);
        }

        static std::unordered_set<std::string> Keywords =
            { "if", "fun", "let", "for", "while", "continue", "break", "return" };

        if (Keywords.contains(Result))
            Result += Chars[RandomInt(0, Chars.size())];

        return Result;
    }

    std::string ParserFuzzer::GenByte()
    {
        return std::to_string(RandomInt8(0, std::numeric_limits<Int8>::max())) + "b";
    }

    std::string ParserFuzzer::GenInt()
    {
        return std::to_string(RandomInt(0, 100000));
    }

    std::string ParserFuzzer::GenLong()
    {
        return std::to_string(RandomInt64(0, 1000000)) + "l";
    }

    std::string ParserFuzzer::GenFloat()
    {
        return std::to_string(RandomFloat(0.f, 100000.0)) + "f";
    }

    std::string ParserFuzzer::GenDouble()
    {
        return std::to_string(RandomDouble(0.0, 100000.0));
    }

    Int8 ParserFuzzer::RandomInt8(Int8 Min, Int8 Max)
    {
        std::uniform_int_distribution<Int8> Dist(Min, Max);
        return Dist(Gen);
    }

    int ParserFuzzer::RandomInt(int Min, int Max)
    {
        std::uniform_int_distribution<int> Dist(Min, Max);
        return Dist(Gen);
    }

    Int64 ParserFuzzer::RandomInt64(Int64 Min, Int64 Max)
    {
        std::uniform_int_distribution<Int64> Dist(Min, Max);
        return Dist(Gen);
    }

    float ParserFuzzer::RandomFloat(float Min, float Max)
    {
        std::uniform_real_distribution<float> Dist(Min, Max);
        return Dist(Gen);
    }

    double ParserFuzzer::RandomDouble(double Min, double Max)
    {
        std::uniform_real_distribution<double> Dist(Min, Max);
        return Dist(Gen);
    }
}

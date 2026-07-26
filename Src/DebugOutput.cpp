//
// Created by bohdan on 03.06.26.
//

#include "Volt/Debug/DebugOutput/DebugOutput.h"

#include <complex.h>

#include "Volt/Core/TypeChecker/ExprResult.h"

namespace Volt
{
    void DebugOutput::WriteTokens() const
    {
        for (const Token& Tok : CContext.Tokens)
            Os << Tok.ToString(CContext) << '\n';
    }

    void DebugOutput::WriteLexErrors() const
    {
        for (const LexError& Error : CContext.LexErrors)
            ErrOs << "LexError: " << Error.ToString() <<
                " At position: [" << Error.Line << ":" << Error.Column << "]\n";
    }

    void DebugOutput::WriteParseErrors() const
    {
        for (const auto& Err : CContext.ParseErrors)
            ErrOs << "ParseError: " << Err.ToString() <<
                " At position: [" << Err.Line << ":" << Err.Column << "]\n";
    }

    void DebugOutput::WriteTypeErrors() const
    {
        for (const TypeError& Error : CContext.TypeErrors)
            ErrOs << "TypeError: " << Error.ToString() <<
                " At position: [" << Error.Line << ":" << Error.Column << "]\n";
    }

    void DebugOutput::WriteAST(ASTNode* Node, size_t Indent) const
    {
        WriteIndent(Indent);

        if (!Node)
        {
            Os << "NULL\n";
            return;
        }

        Os << Node->GetName() <<
            " [" << Node->Pos << ":" << Node->Line << ":" << Node->Column << "] ";

        Os << "[Compile Time Value: ";
        WriteCompileTimeValue(Node->CompileTimeValue);
        Os << "] ";

        if (auto Sequence = Cast<SequenceNode>(Node))
        {
            Os << '\n';
            for (auto Statement : Sequence->Statements)
                WriteAST(Statement, Indent + 1);
        }
        else if (auto Block = Cast<BlockNode>(Node))
        {
            Os << '\n';
            for (auto Statement : Block->Statements)
                WriteAST(Statement, Indent + 1);
        }
        else if (auto Identifier = Cast<IdentifierNode>(Node))
            Os << Identifier->Value.str() << '\n';
        else if (auto Float = Cast<FloatingPointNode>(Node))
            Os << Float->Value << '\n';
        else if (auto Int = Cast<IntegerNode>(Node))
            Os << Int->Value << '\n';
        else if (auto Bool = Cast<BoolNode>(Node))
            Os << Bool->Value << '\n';
        else if (auto Char = Cast<CharNode>(Node))
            Os << Char->Value << '\n';
        else if (auto String = Cast<StringNode>(Node))
            Os << String->Value.str() << '\n';
        else if (auto Array = Cast<ArrayNode>(Node))
        {
            Os << "Elements:\n";
            for (auto El : Array->Elements)
                WriteAST(El, Indent + 1);
        }
        else if (auto Ref = Cast<RefNode>(Node))
        {
            Os << "Target:\n";
            WriteAST(Ref->Target, Indent + 1);
        }
        else if (auto Unref = Cast<UnrefNode>(Node))
        {
            Os << "Target:\n";
            WriteAST(Unref->Target, Indent + 1);
        }
        else if (auto UnaryOp = Cast<UnaryOpNode>(Node))
        {
            Os << "OpType: " << Operator::ToString(UnaryOp->Type);
            Os << '\n';
            WriteAST(UnaryOp->Operand, Indent + 1);
        }
        else if (auto BinaryOp = Cast<BinaryOpNode>(Node))
        {
            Os << "OpType: " << Operator::ToString(BinaryOp->Type);
            Os << '\n';
            WriteAST(BinaryOp->Left, Indent + 1);
            WriteAST(BinaryOp->Right, Indent + 1);
        }
        else if (auto Call = Cast<CallNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Callee:\n";
            WriteAST(Call->Callee, Indent + 1);
            WriteIndent(Indent);
            Os << "Args:\n";
            for (auto Arg : Call->Arguments)
                WriteAST(Arg, Indent + 1);
        }
        else if (auto Subscript = Cast<SubscriptNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Target:\n";
            WriteAST(Subscript->Target, Indent + 1);
            WriteIndent(Indent);
            Os << "Index:\n";
            WriteAST(Subscript->Index, Indent + 1);
        }
        else if (auto QualTy = Cast<QualTypeNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Type:\n";
            WriteAST(QualTy->Type, Indent + 1);
            WriteIndent(Indent);
            Os << "Quals: " << QualTy->Quals << '\n';
        }
        else if (auto PrimitiveType = Cast<PrimitiveTypeNode>(Node))
        {
            Os << "Type: " << PrimitiveType->Type->ToString() << '\n';
        }
        else if (auto ArrType = Cast<ArrayTypeNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "BaseType:\n";
            WriteAST(ArrType->BaseType, Indent + 1);
            WriteIndent(Indent);
            Os << "Length:\n";
            WriteAST(ArrType->Length, Indent + 1);
        }
        else if (auto DerivedType = Cast<DerivedTypeNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "BaseType:\n";
            WriteAST(DerivedType->BaseType, Indent + 1);
        }
        else if (auto ClassTy = Cast<ClassTypeNode>(Node))
        {
            Os << "Name: " << ClassTy->Name << '\n';
        }
        else if (auto ExplicitCast = Cast<ExplicitCastNode>(Node))
        {
            Os << "Type: \n";
            WriteAST(ExplicitCast->Type, Indent + 1);
            WriteIndent(Indent);
            Os << "Target: \n";
            WriteAST(ExplicitCast->Target, Indent + 1);
        }
        else if (auto Variable = Cast<VariableNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "DataType:\n";
            WriteAST(Variable->Type, Indent + 1);
            WriteIndent(Indent);
            Os << "Name: " << Variable->Name.str() << '\n';
            WriteIndent(Indent);
            Os << "Value:\n";
            WriteAST(Variable->Value, Indent + 1);
        }
        else if (auto Param = Cast<ParamNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "DataType:\n";
            WriteAST(Param->Type, Indent + 1);
            WriteIndent(Indent);
            Os << "Name: " << Param->Name.str() << '\n';
            WriteIndent(Indent);
            Os << "DefaultValue:\n";
            WriteAST(Param->DefaultValue, Indent + 1);
        }
        else if (auto Function = Cast<FunctionNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "ReturnType:\n";
            WriteAST(Function->ReturnType, Indent + 1);
            WriteIndent(Indent);
            Os << "Name: " << Function->Name.str() << '\n';
            WriteIndent(Indent);
            Os << "Parameters:\n";

            for (auto Parameter : Function->Params)
                WriteAST(Parameter, Indent + 1);
            WriteIndent(Indent);
            Os << "Body:\n";
            WriteAST(Function->Body, Indent + 1);
        }
        else if (auto Return = Cast<ReturnNode>(Node))
        {
            Os << "Return Value:\n";
            WriteAST(Return->ReturnValue, Indent + 1);
        }
        else if (auto Class = Cast<ClassNode>(Node))
        {
            Os << "Fields:\n";
            for (auto Field : Class->Fields)
                WriteAST(Field, Indent + 1);

            WriteIndent(Indent);
            Os << "Methods:\n";
            for (auto Method : Class->Methods)
                WriteAST(Method, Indent + 1);
        }
        else if (auto MemberAccess = Cast<MemberAccessNode>(Node))
        {
            Os << "Target:\n";
            WriteAST(MemberAccess->Target, Indent + 1);
            WriteIndent(Indent);
            Os << "Member:\n";
            WriteAST(MemberAccess->Member, Indent + 1);
        }
        else if (auto If = Cast<IfNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Condition:\n";
            WriteAST(If->Condition, Indent + 1);
            WriteIndent(Indent);
            Os << "Branch:\n";
            WriteAST(If->Branch, Indent + 1);
            if (If->ElseBranch)
            {
                WriteIndent(Indent);
                Os << "ElseBranch:\n";
                WriteAST(If->ElseBranch, Indent + 1);
            }
        }
        else if (auto While = Cast<WhileNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Condition:\n";
            WriteAST(While->Condition, Indent + 1);
            WriteIndent(Indent);
            Os << "Branch:\n";
            WriteAST(While->Branch, Indent + 1);
        }
        else if (auto For = Cast<ForNode>(Node))
        {
            Os << '\n';
            WriteIndent(++Indent);
            Os << "Initialization:\n";
            WriteAST(For->Initialization, Indent + 1);
            WriteIndent(Indent);
            Os << "Condition:\n";
            WriteAST(For->Condition, Indent + 1);
            WriteIndent(Indent);
            Os << "Iteration:\n";
            WriteAST(For->Iteration, Indent + 1);
            WriteIndent(Indent);
            Os << "Body:\n";
            WriteAST(For->Body, Indent + 1);
        }
        else
            Os << '\n';
    }

    void DebugOutput::WriteCompileTimeValue(SemaResult *Value) const
    {
        auto Res = Cast<ExprResult>(Value);
        if (!Res || Res->IsEmpty())
        {
            Os << "Null";
            return;
        }

        switch (Value->GetType()->GetCategory())
        {
            case TypeCategory::INTEGER:        Os << (Value->GetType()->IsSignedIntegerType() ?
                                               Res->GetInt() : Res->GetUInt());    break;
            case TypeCategory::FLOATING_POINT: Os << Res->GetFloat();   break;
            case TypeCategory::BOOLEAN:        Os << Res->GetBool();    break;
            case TypeCategory::CHAR:           Os << Res->GetChar();    break;
            case TypeCategory::POINTER:        Os << Res->GetPointer(); break;
            default:                           Os << "Null";            break;
        }
    }

    void DebugOutput::WriteIndent(size_t Indent) const
    {
        for (size_t i = 0; i < Indent; i++)
            Os << "  ";
    }
}

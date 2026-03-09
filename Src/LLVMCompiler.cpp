//
// Created by bohdan on 03.01.26.
//

#include "Volt/Compiler/LLVMCompiler.h"
#include <llvm/Support/TargetSelect.h>

#define ERROR(Message) throw CompilerError(Message);

namespace Volt
{
    void LLVMCompiler::Compile()
    {
        CompileNode(ASTTree);
    }

    TypedValue *LLVMCompiler::CompileNode(ASTNode *Node)
    {
        if (Node->CompileTimeValue && !Node->CompileTimeValue->IsEmpty)
        {
            CTimeValue* Value = Node->CompileTimeValue;
            switch (Value->Type->GetCategory())
            {
                case TypeCategory::INTEGER:
                    return Create<TypedValue>(llvm::ConstantInt::get(
                    CContext.GetLLVMType(Value->Type), Value->Int), Value->Type);
                case TypeCategory::FLOATING_POINT:
                    return Create<TypedValue>(llvm::ConstantFP::get(
                        CContext.GetLLVMType(Value->Type), Value->Float), Value->Type);
                case TypeCategory::BOOLEAN:
                    return Create<TypedValue>(llvm::ConstantInt::get(
                        llvm::Type::getInt1Ty(Context), Value->Bool), Value->Type);
                default:
                    ERROR("Invalid compile tyme value type\n");
            }
        }

        if (auto Sequence = Cast<SequenceNode>(Node))
        {
            for (auto Statement : Sequence->Statements)
                CompileNode(Statement);

            return nullptr;
        }
        if (const auto Block = Cast<BlockNode>(Node))
            return CompileBlock(Block);
        if (const auto Char = Cast<CharNode>(Node))
            return CompileChar(Char);
        if (const auto Int = Cast<IntegerNode>(Node))
            return CompileInt(Int);
        if (const auto Bool = Cast<BoolNode>(Node))
            return CompileBool(Bool);
        if (const auto Float = Cast<FloatingPointNode>(Node))
            return CompileFloat(Float);
        if (const auto String = Cast<StringNode>(Node))
            return CompileString(String);
        if (const auto Array = Cast<ArrayNode>(Node))
            return CompileArray(Array);
        if (const auto Identifier = Cast<IdentifierNode>(Node))
            return CompileIdentifier(Identifier);
        if (const auto Ref = Cast<RefNode>(Node))
            return CompileRef(Ref);
        if (const auto Unref = Cast<UnrefNode>(Node))
            return CompileUnref(Unref);
        if (const auto Prefix = Cast<PrefixOpNode>(Node))
            return CompilePrefix(Prefix);
        if (const auto Suffix = Cast<SuffixOpNode>(Node))
            return CompileSuffix(Suffix);
        if (const auto Unary = Cast<UnaryOpNode>(Node))
            return CompileUnary(Unary);
        if (const auto Comparison = Cast<ComparisonNode>(Node))
            return CompileComparison(Comparison);
        if (const auto Logical = Cast<LogicalNode>(Node))
            return CompileLogical(Logical);
        if (const auto AssignOp = Cast<AssignmentNode>(Node))
            return CompileAssignment(AssignOp);
        if (const auto BinaryOp = Cast<BinaryOpNode>(Node))
            return CompileBinary(BinaryOp);
        if (const auto Call = Cast<CallNode>(Node))
            return CompileCall(Call);
        if (const auto Subscript = Cast<SubscriptNode>(Node))
            return CompileSubscript(Subscript);
        if (const auto ExplicitCast = Cast<ExplicitCastNode>(Node))
            return CompileExplicitCast(ExplicitCast);
        if (const auto Var = Cast<VariableNode>(Node))
            return CompileVariable(Var);
        if (const auto Function = Cast<FunctionNode>(Node))
            return CompileFunction(Function);
        if (const auto Return = Cast<ReturnNode>(Node))
            return CompileReturn(Return);
        if (const auto If = Cast<IfNode>(Node))
            return CompileIf(If);
        if (const auto While = Cast<WhileNode>(Node))
            return CompileWhile(While);
        if (const auto For = Cast<ForNode>(Node))
            return CompileFor(For);
        if (Cast<BreakNode>(Node))
            return CompileBreak();
        if (Cast<ContinueNode>(Node))
            return CompileContinue();

        ERROR("Cannot resolve node: '" + Node->GetName() + "'");
    }

    TypedValue *LLVMCompiler::CompileBlock(const BlockNode *Block)
    {
        EnterScope();

        if (CurrentFunction)
        {
            for (size_t i = 0; i < FunctionParams.size(); i++)
            {
                llvm::Argument& Arg = *(CurrentFunction->arg_begin() + i);
                llvm::Type* ArgType = Arg.getType();

                llvm::AllocaInst* Alloca = Builder.CreateAlloca(ArgType, nullptr, Arg.getName());

                Builder.CreateStore(&Arg, Alloca);
                DeclareVariable(Arg.getName().str(),
                    Create<TypedValue>(Alloca, FunctionParams[i]));
            }
            CurrentFunction = nullptr;
        }

        FunctionParams = {};

        for (auto Stmt : Block->Statements)
        {
            CompileNode(Stmt);

            if (Builder.GetInsertBlock()->getTerminator())
                break;
        }

        ExitScope();

        return nullptr;
    }

    TypedValue* LLVMCompiler::CompileInt(const IntegerNode *Int)
    {
        return Create<TypedValue>(llvm::ConstantInt::get(
            CContext.GetLLVMType(Int->CompileTimeValue->Type), Int->Value), Int->CompileTimeValue->Type);
    }

    TypedValue *LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        return Create<TypedValue>(llvm::ConstantFP::get(
            CContext.GetLLVMType(Float->CompileTimeValue->Type), Float->Value), Float->CompileTimeValue->Type);
    }

    TypedValue *LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
            CContext.GetBoolType());
    }

    TypedValue *LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value), CContext.GetCharType());
    }

    TypedValue *LLVMCompiler::CompileString(const StringNode *String)
    {
        return Create<TypedValue>(Builder.CreateGlobalString(String->Value.str()),
            CContext.GetPointerType(CContext.GetCharType()));
    }

    TypedValue *LLVMCompiler::CompileArray(const ArrayNode *Array)
    {
        if (Array->Elements.empty())
            ERROR("Array empty")

        llvm::Type* ArrType = nullptr;

        if (auto Type = Cast<ArrayType>(Array->CompileTimeValue->Type))
            ArrType = llvm::ArrayType::get(
                CContext.GetLLVMType(Type->BaseType), Array->Elements.size());

        llvm::AllocaInst* Arr = Builder.CreateAlloca(ArrType);

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            TypedValue* El = CompileNode(Array->Elements[i]);

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }

        return Create<TypedValue>(Arr, Array->CompileTimeValue->Type);
    }

    TypedValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.str();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        {
            TypedValue* Var = Iter->second;
            return Create<TypedValue>(Builder.CreateLoad(CContext.GetLLVMType(Var->GetDataType()),
                        Var->GetValue(), Value + "_val"), Var->GetDataType());
        }

        ERROR("Cannot resolve symbol: '" + Value + "'")
    }

    TypedValue *LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        TypedValue* LValue = GetLValue(Ref->Target);
        if (!LValue)
            ERROR("Cannot apply operator '$' to r-value")

       return Create<TypedValue>(LValue->GetValue(), CContext.GetPointerType(LValue->GetDataType()));
    }

    TypedValue *LLVMCompiler::CompileUnref(const UnrefNode *Unref)
    {
        TypedValue *TValue = CompileNode(Unref->Target);
        if (!TValue)
            return nullptr;

        llvm::Value* Value = TValue->GetValue();

        return Create<TypedValue>(Builder.CreateLoad(
                CContext.GetLLVMType(Unref->CompileTimeValue->Type), Value), Unref->CompileTimeValue->Type);
    }

    TypedValue *LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        TypedValue* LValue = GetLValue(Prefix->Operand);
        if (!LValue)
            ERROR("Cannot apply prefix operator to r-value")

        llvm::Value* Value = LValue->GetValue();
        Value = Builder.CreateLoad(CContext.GetLLVMType(LValue->GetDataType()), Value);
        switch (Prefix->Type)
        {
            case OperatorType::INC:
                Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            case OperatorType::DEC:
                Value = Builder.CreateSub(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            default:
                ERROR("Unknown prefix operator")
        }

        Builder.CreateStore(Value, LValue->GetValue());

        return Create<TypedValue>(Value, LValue->GetDataType());
    }

    TypedValue *LLVMCompiler::CompileSuffix(const SuffixOpNode *Suffix)
    {
        TypedValue* LValue = GetLValue(Suffix->Operand);
        if (!LValue)
            ERROR("Cannot apply suffix operator to r-value")

        llvm::Value* Value = LValue->GetValue();
        Value = Builder.CreateLoad(CContext.GetLLVMType(LValue->GetDataType()), Value);
        llvm::Value* Temp = Value;
        switch (Suffix->Type)
        {
            case OperatorType::INC:
                Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            case OperatorType::DEC:
                Value = Builder.CreateSub(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            default:
                ERROR("Unknown suffix operator")
        }

        Builder.CreateStore(Value, LValue->GetValue());

        return Create<TypedValue>(Temp, LValue->GetDataType());
    }

    TypedValue *LLVMCompiler::CompileUnary(const UnaryOpNode *Unary)
    {
        TypedValue* TValue = CompileNode(Unary->Operand);
        llvm::Value* Value = TValue->GetValue();
        DataType* Type = TValue->GetDataType();

        DataType* BoolType = CContext.GetBoolType();

        bool IsFP = Cast<FloatingPointType>(Type);

        switch (Unary->Type)
        {
            case OperatorType::ADD:         return TValue;
            case OperatorType::SUB:         return Create<TypedValue>(IsFP ?
                                            Builder.CreateFNeg(Value) :
                                            Builder.CreateNeg(Value), Unary->CompileTimeValue->Type);
            case OperatorType::LOGICAL_NOT: return Create<TypedValue>(Builder.CreateNot(
                                               ImplicitCast(TValue, BoolType)->GetValue()), Unary->CompileTimeValue->Type);
            case OperatorType::BIT_NOT:     return Create<TypedValue>(Builder.CreateNot(Value), Unary->CompileTimeValue->Type);
            default: ERROR("Unknown unary operator")
        }
    }

    TypedValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        TypedValue* Left = CompileNode(Comparison->Left);
        TypedValue* Right = CompileNode(Comparison->Right);

        if (!Left->CastTo(Comparison->LeftOperandType, Builder, CContext))
            return nullptr;

        if (!Right->CastTo(Comparison->RightOperandType, Builder, CContext))
            return nullptr;

        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        bool IsFP = Cast<FloatingPointType>(Type);

        bool IsSigned = false;
        if (!IsFP)
            if (auto IntType = Cast<IntegerType>(Type))
                IsSigned = IntType->IsSigned;

        switch (Comparison->Type)
        {
            case OperatorType::EQ:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOEQ(LeftVal, RightVal) :
                                Builder.CreateICmpEQ(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            case OperatorType::NEQ: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpONE(LeftVal, RightVal) :
                                Builder.CreateICmpNE(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            case OperatorType::LT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOLT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLT(LeftVal, RightVal) :
                                Builder.CreateICmpULT(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            case OperatorType::LTE: return Create<TypedValue>( IsFP ?
                                Builder.CreateFCmpOLE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLE(LeftVal, RightVal) :
                                Builder.CreateICmpULE(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            case OperatorType::GT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGT(LeftVal, RightVal) :
                                Builder.CreateICmpUGT(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            case OperatorType::GTE: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGE(LeftVal, RightVal) :
                                Builder.CreateICmpUGE(LeftVal, RightVal), Comparison->CompileTimeValue->Type);
            default: ERROR("Unknown comparison operator")
        }
    }

    TypedValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        TypedValue* Left = CompileNode(Logical->Left);
        TypedValue* Right = CompileNode(Logical->Right);

        if (!Left->CastTo(Logical->LeftOperandType, Builder, CContext))
            return nullptr;

        if (!Right->CastTo(Logical->RightOperandType, Builder, CContext))
            return nullptr;

        // llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        switch (Logical->Type)
        {
            case OperatorType::LOGICAL_OR:
            {
                // llvm::BasicBlock* OrRhsBB = llvm::BasicBlock::Create(Context, "or.rhs", Func);
                // llvm::BasicBlock* OrTrueBB = llvm::BasicBlock::Create(Context, "or.true", Func);
                //
                // Builder.CreateCondBr(Left->GetValue(), OrTrueBB, OrRhsBB);
                //
                // Builder.SetInsertPoint(OrRhsBB);
                // TypedValue* Right = CompileNode(Logical->Right);
                // Right = ImplicitCast(Right, Logical->OperandsType);
                //
                // llvm::BasicBlock* OrEndBB = llvm::BasicBlock::Create(Context, "or.end", Func);
                // Builder.CreateCondBr(Right->GetValue(), OrTrueBB, OrEndBB);
                //
                // Builder.SetInsertPoint(OrTrueBB);
                // Builder.CreateBr(OrEndBB);
                //
                // Builder.SetInsertPoint(OrEndBB);
                // llvm::PHINode* Phi = Builder.CreatePHI(Builder.getInt1Ty(), 2);
                // Phi->addIncoming(Builder.getTrue(), OrTrueBB);
                // Phi->addIncoming(Builder.getFalse(), OrRhsBB);
                //
                // return Create<TypedValue>(Phi, Logical->CompileTimeValue->Type);

                return Create<TypedValue>(
                    Builder.CreateOr(Left->GetValue(), Right->GetValue()), Logical->CompileTimeValue->Type);
            }
            case OperatorType::LOGICAL_AND:
            {
                // auto* AndRhsBB = llvm::BasicBlock::Create(Context, "and.rhs", Func);
                // auto* AndFalseBB= llvm::BasicBlock::Create(Context, "and.false", Func);
                //
                // Builder.CreateCondBr(Left->GetValue(), AndRhsBB, AndFalseBB);
                //
                // Builder.SetInsertPoint(AndRhsBB);
                // TypedValue* Right = CompileNode(Logical->Right);
                // Right = ImplicitCast(Right, Logical->OperandsType);
                // auto* AndEndBB  = llvm::BasicBlock::Create(Context, "and.end", Func);
                // Builder.CreateBr(AndEndBB);
                //
                // Builder.SetInsertPoint(AndFalseBB);
                // Builder.CreateBr(AndEndBB);
                //
                // Builder.SetInsertPoint(AndEndBB);
                // llvm::PHINode* Phi = Builder.CreatePHI(Builder.getInt1Ty(), 2);
                //
                // Phi->addIncoming(Right->GetValue(), AndRhsBB);
                // Phi->addIncoming(Builder.getFalse(), AndFalseBB);
                //
                // return Create<TypedValue>(Phi, Logical->CompileTimeValue->Type);

                return Create<TypedValue>(
                    Builder.CreateAnd(Left->GetValue(), Right->GetValue()), Logical->CompileTimeValue->Type);
            }
            default:
                ERROR("Unknown logical operator")
        }
    }

    TypedValue *LLVMCompiler::CompileAssignment(const AssignmentNode *Assignment)
    {
        TypedValue* LValue = GetLValue(Assignment->Left);

        if (!LValue)
            ERROR("Cannot apply assignment operator to r-value")

        llvm::Value* Value = LValue->GetValue();

        DataType* Type = LValue->GetDataType();
        TypedValue* Right = ImplicitCast(CompileNode(Assignment->Right), Type);
        llvm::Value* RightVal = Right->GetValue();

        if (Assignment->Type == OperatorType::ASSIGN)
            return Create<TypedValue>(Builder.CreateStore(RightVal, Value), Right->GetDataType());

        llvm::Value* Left = Builder.CreateLoad(CContext.GetLLVMType(Type), Value);

        bool IsFP = Cast<FloatingPointType>(Type);

        bool IsSigned = false;
        if (!IsFP)
            if (auto IntType = Cast<IntegerType>(Type))
                IsSigned = IntType->IsSigned;

        switch (Assignment->Type)
        {
            case OperatorType::ADD_ASSIGN:
                Left = IsFP ? Builder.CreateFAdd(Left, RightVal) :
                              Builder.CreateAdd(Left, RightVal);
                break;
            case OperatorType::SUB_ASSIGN:
                Left = IsFP ? Builder.CreateFSub(Left, RightVal) :
                              Builder.CreateSub(Left, RightVal);
                break;
            case OperatorType::MUL_ASSIGN:
                Left = IsFP ? Builder.CreateFMul(Left, RightVal) :
                              Builder.CreateMul(Left, RightVal);
                break;
            case OperatorType::DIV_ASSIGN:
                Left = IsFP     ? Builder.CreateFDiv(Left, RightVal) :
                       IsSigned ? Builder.CreateSDiv(Left, RightVal) :
                                  Builder.CreateUDiv(Left, RightVal);
                break;
            case OperatorType::MOD_ASSIGN:
                Left = IsSigned ? Builder.CreateSRem(Left, RightVal) :
                                  Builder.CreateURem(Left, RightVal);
                break;
            default:
                ERROR("Unknown assignment operator")
        }

        return Create<TypedValue>(Builder.CreateStore(Left, Value), Type);
    }

    TypedValue* LLVMCompiler::CompileBinary(const BinaryOpNode *BinaryOp)
    {
        TypedValue* Left = CompileNode(BinaryOp->Left);
        TypedValue* Right = CompileNode(BinaryOp->Right);

        if (!Left->CastTo(BinaryOp->LeftOperandType, Builder, CContext))
            return nullptr;

        if (!Right->CastTo(BinaryOp->RightOperandType, Builder, CContext))
            return nullptr;

        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        bool IsFP = Cast<FloatingPointType>(Type);
        auto PtrType = Cast<PointerType>(Type);
        bool IsSigned = false;
        if (auto IntType = Cast<IntegerType>(Type))
            IsSigned = IntType->IsSigned;

        switch (BinaryOp->Type)
        {
            case OperatorType::ADD:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFAdd(LeftVal, RightVal) :
                                    Builder.CreateAdd(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::SUB:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFSub(LeftVal, RightVal) :
                                    Builder.CreateSub(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::MUL:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFMul(LeftVal, RightVal) :
                                    Builder.CreateMul(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::DIV:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFDiv(LeftVal, RightVal) : IsSigned ?
                                    Builder.CreateSDiv(LeftVal, RightVal) :
                                    Builder.CreateUDiv(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::MOD:     return Create<TypedValue>(IsSigned ?
                                    Builder.CreateSRem(LeftVal, RightVal) :
                                    Builder.CreateURem(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::BIT_AND: return Create<TypedValue>(
                                    Builder.CreateAnd(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::BIT_OR:  return Create<TypedValue>(
                                    Builder.CreateOr(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::BIT_XOR: return Create<TypedValue>(
                                    Builder.CreateXor(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::LSHIFT:  return  Create<TypedValue>(
                                    Builder.CreateShl(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            case OperatorType::RSHIFT:  return Create<TypedValue>(IsSigned ?
                                    Builder.CreateAShr(LeftVal, RightVal) :
                                    Builder.CreateLShr(LeftVal, RightVal), BinaryOp->CompileTimeValue->Type);
            default: ERROR("Unknown binary operator")
        }
    }

    TypedValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        const auto& Args = Call->Arguments;

        SmallVec8<llvm::Value*> LLVMArgs;

        LLVMArgs.reserve(Args.size());

        for (const auto Arg : Args)
        {
            TypedValue* ArgValue = CompileNode(Arg);
            if (!ArgValue)
                return nullptr;

            if (Arg->ExpectedType)
                if (!ArgValue->CastTo(Arg->ExpectedType, Builder, CContext))
                    return nullptr;

            LLVMArgs.push_back(ArgValue->GetValue());
        }

        if (auto Func = Cast<FunctionCallee>(Call->ResolvedCallee))
            return Create<TypedValue>(Builder.CreateCall(Func->Function, LLVMArgs),
                Func->ReturnType);

        if (auto BuiltinFunc = Cast<BuiltinFuncCallee>(Call->ResolvedCallee))
        {
            llvm::Function* LLVMFunc = Module->getFunction(BuiltinFunc->BaseName);
            return Create<TypedValue>(Builder.CreateCall(LLVMFunc, LLVMArgs),
                BuiltinFunc->ReturnType );
        }

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileSubscript(const SubscriptNode *Subscript)
    {
        if (auto PtrType = Cast<PointerType>(Subscript->TargetType))
        {
            TypedValue* Value = CompileNode(Subscript->Target);

            if (!Value)
                return nullptr;

            llvm::Value* LLVMValue = Value->GetValue();
            llvm::Type* ElType = CContext.GetLLVMType(PtrType->BaseType);
            TypedValue* Index = CompileNode(Subscript->Index);
            llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
            llvm::Value* El = Builder.CreateLoad(ElType, ElPtr);
            return Create<TypedValue>(El, PtrType->BaseType);
        }

        if (auto ArrType = Cast<ArrayType>(Subscript->TargetType))
        {
            TypedValue* Value = GetLValue(Subscript->Target);

            if (!Value)
                return nullptr;

            llvm::Value* LLVMValue = Value->GetValue();
            llvm::Type* ElType = CContext.GetLLVMType(ArrType->BaseType);
            TypedValue* Index = CompileNode(Subscript->Index);
            llvm::Value* ElPtr = Builder.CreateGEP(CContext.GetLLVMType(ArrType), LLVMValue,
                { Builder.getInt32(0), Index->GetValue() });
            llvm::Value* El = Builder.CreateLoad(ElType, ElPtr);
            return Create<TypedValue>(El, ArrType->BaseType);
        }

        return nullptr;
    }

    TypedValue* LLVMCompiler::CompileExplicitCast(const ExplicitCastNode *ExplicitCast)
    {
        DataType* DstType = ExplicitCast->CompileTimeValue->Type;
        TypedValue* Target = CompileNode(ExplicitCast->Target);

        if (TypedValue* Value = ImplicitCast(Target, DstType))
            return Value;

        return Create<TypedValue>(
            Builder.CreateBitCast(Target->GetValue(), CContext.GetLLVMType(DstType)), DstType);
    }

    TypedValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        DataType* VarType = Var->Type->ResolvedType;

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::Type* Type = CContext.GetLLVMType(VarType);

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type);

        auto ArrType = Cast<ArrayType>(VarType);
        if (ArrType && Var->Value)
        {
            if (auto Arr = Cast<ArrayNode>(Var->Value))
            {
                if (ArrType->Length < Arr->Elements.size())
                    ERROR("Too many elements in array initializer");
                FillArray(Arr, Alloca);
            }
        }
        else if (Var->Value)
        {
            TypedValue* Value = CompileNode(Var->Value);
            if (!Value->CastTo(VarType, Builder, CContext))
                return nullptr;
            Builder.CreateStore(Value->GetValue(), Alloca);
        }

        DeclareVariable(Var->Name.str(),
            Create<TypedValue>(Alloca, VarType, true));

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        SmallVec8<llvm::Type*> Params;
        Params.reserve(Function->Params.size());

        for (const auto Param : Function->Params)
        {
            DataType* ParamType = Param->Type->ResolvedType;
            Params.push_back(CContext.GetLLVMType(ParamType));
        }

        llvm::Type* RetType = CContext.GetLLVMType(Function->ReturnType->ResolvedType);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        const std::string& FuncName = Function->Name.str();
        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());

        SmallVec8<DataType*> ParamsTypes;

        const auto& FuncParams = Function->Params;
        ParamsTypes.reserve(FuncParams.size());
        for (size_t i = 0; i < FuncParams.size(); i++)
        {
            DataType* ParamType = FuncParams[i]->Type->ResolvedType;
            auto Arg = Func->args().begin() + i;
            Arg->setName(FuncParams[i]->Name.str());
            ParamsTypes.push_back(ParamType);
        }

        CurrentFunction = Func;
        FunctionParams = ParamsTypes;

        if (auto FuncCallee = Cast<FunctionCallee>(Function->ResolvedCallee))
            FuncCallee->Function = Func;
        else
            ERROR("Function definition '" + FuncName + "' is unknown");

        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        Builder.SetInsertPoint(Entry);
        CompileBlock(Cast<BlockNode>(Function->Body));

        llvm::BasicBlock* Bb = Builder.GetInsertBlock();

        if (!Bb->getTerminator())
        {
            if (RetType->isVoidTy())
                Builder.CreateRetVoid();
            else
                ERROR("Function '" + FuncName + "' must return value");
        }

        return nullptr;
    }

    TypedValue* LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            TypedValue* RetVal = CompileNode(Return->ReturnValue);
            Builder.CreateRet(RetVal->GetValue());
            return nullptr;
        }

        Builder.CreateRetVoid();
        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileIf(const IfNode *If)
    {
        TypedValue* Cond = CompileNode(If->Condition);
        if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
            return nullptr;

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        auto ThenBB  = llvm::BasicBlock::Create(Context, "then", Func);
        auto ElseBB  = llvm::BasicBlock::Create(Context, "else", Func);
        auto MergeBB = llvm::BasicBlock::Create(Context, "ifcont");

        Builder.CreateCondBr(Cond->GetValue(), ThenBB, ElseBB);

        Builder.SetInsertPoint(ThenBB);
        CompileNode(If->Branch);
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(MergeBB);

        Builder.SetInsertPoint(ElseBB);
        if (If->ElseBranch)
            CompileNode(If->ElseBranch);
        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(MergeBB);

        if (!MergeBB->getParent())
            MergeBB->insertInto(Func);

        Builder.SetInsertPoint(MergeBB);

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileWhile(const WhileNode *While)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* LoopHeader = llvm::BasicBlock::Create(Context, "loop.header", Func);
        Builder.CreateBr(LoopHeader);
        Builder.SetInsertPoint(LoopHeader);
        TypedValue* Cond = CompileNode(While->Condition);
        if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
            return nullptr;

        llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "loop.body", Func);
        llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
        Builder.CreateCondBr(Cond->GetValue(), ThenBB, EndBB);

        LoopEndStack.push(EndBB);
        LoopHeaderStack.push(LoopHeader);

        Builder.SetInsertPoint(ThenBB);
        CompileNode(While->Branch);

        LoopEndStack.pop();
        LoopHeaderStack.pop();

        if (!Builder.GetInsertBlock()->getTerminator())
            Builder.CreateBr(LoopHeader);

        if (!EndBB->getParent())
            EndBB->insertInto(Func);

        Builder.SetInsertPoint(EndBB);
        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileFor(const ForNode *For)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        EnterScope();

        llvm::BasicBlock* InitializationBB = llvm::BasicBlock::Create(
            Context, "for.initialization", Func);
        Builder.CreateBr(InitializationBB);
        Builder.SetInsertPoint(InitializationBB);
        CompileNode(For->Initialization);

        llvm::BasicBlock* ForHeader = llvm::BasicBlock::Create(Context, "for.header", Func);
        Builder.CreateBr(ForHeader);
        Builder.SetInsertPoint(ForHeader);
        TypedValue* Cond = CompileNode(For->Condition);
        if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
            return nullptr;

        llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "for.body", Func);
        llvm::BasicBlock* LatchBB = llvm::BasicBlock::Create(Context, "for.latch", Func);
        llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
        Builder.CreateCondBr(Cond->GetValue(), ThenBB, EndBB);

        LoopEndStack.push(EndBB);
        LoopHeaderStack.push(LatchBB);

        Builder.SetInsertPoint(ThenBB);
        CompileNode(For->Body);

        LoopEndStack.pop();
        LoopHeaderStack.pop();

        Builder.CreateBr(LatchBB);

        Builder.SetInsertPoint(LatchBB);
        CompileNode(For->Iteration);
        Builder.CreateBr(ForHeader);

        if (!EndBB->getParent())
            EndBB->insertInto(Func);
        Builder.SetInsertPoint(EndBB);

        ExitScope();
        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileBreak()
    {
        if (LoopEndStack.empty())
            ERROR("'break' used outside loop")

        Builder.CreateBr(LoopEndStack.top());
        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileContinue()
    {
        if (LoopHeaderStack.empty())
            ERROR("'continue' used outside loop")

        Builder.CreateBr(LoopHeaderStack.top());
        return nullptr;
    }

    void LLVMCompiler::DeclareVariable(const std::string &Name, TypedValue *Var)
    {
        if (auto Iter = std::find_if(
            ScopeStack.Back().Begin(), ScopeStack.Back().End(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.Back().End())
            ERROR("This variable: '" + Name + "' has already declared in this scope");

        ScopeEntry Entry;
        Entry.Name = Name;

        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            Entry.Previous = Iter->second;

        ScopeStack.Back().Add(Entry);
        SymbolTable[Name] = Var;
    }

    TypedValue *LLVMCompiler::GetVariable(const std::string &Name)
    {
        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            return Iter->second;

        return nullptr;
    }

    void LLVMCompiler::EnterScope()
    {
        ScopeStack.Emplace();
    }

    void LLVMCompiler::ExitScope()
    {
        for (const auto& Entry : ScopeStack.Back())
        {
            if (Entry.Previous)
                SymbolTable[Entry.Name] = Entry.Previous;
            else
                SymbolTable.erase(Entry.Name);
        }

        ScopeStack.Pop();
    }

    TypedValue *LLVMCompiler::GetLValue(const ASTNode *Node)
    {
        if (const auto Identifier = Cast<const IdentifierNode>(Node))
            return GetVariable(Identifier->Value.str());

        if (const auto Subscript = Cast<const SubscriptNode>(Node))
        {
            if (auto PtrType = Cast<PointerType>(Subscript->TargetType))
            {
                TypedValue* Value = CompileNode(Subscript->Target);

                if (!Value)
                    return nullptr;

                llvm::Value* LLVMValue = Value->GetValue();
                llvm::Type* ElType = CContext.GetLLVMType(PtrType->BaseType);
                TypedValue* Index = CompileNode(Subscript->Index);
                llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
                return Create<TypedValue>(ElPtr, PtrType->BaseType);
            }

            if (auto ArrType = Cast<ArrayType>(Subscript->TargetType))
            {
                TypedValue* Value = GetLValue(Subscript->Target);

                if (!Value)
                    return nullptr;

                llvm::Value* LLVMValue = Value->GetValue();
                llvm::Type* ElType = CContext.GetLLVMType(ArrType->BaseType);
                TypedValue* Index = CompileNode(Subscript->Index);
                llvm::Value* ElPtr = Builder.CreateGEP(CContext.GetLLVMType(ArrType), LLVMValue,
                    { Builder.getInt32(0), Index->GetValue() });
                return Create<TypedValue>(ElPtr, ArrType->BaseType);
            }

            return nullptr;
        }

        if (auto Unref = Cast<const UnrefNode>(Node))
        {
            TypedValue *TValue = CompileNode(Unref->Target);
            if (!TValue)
                return nullptr;

            return Create<TypedValue>(TValue->GetValue(), Unref->CompileTimeValue->Type);
        }

        return nullptr;
    }

    TypedValue *LLVMCompiler::ImplicitCast(TypedValue *Value, DataType* Target)
    {
        if (Value->CastTo(Target, Builder, CContext))
            return Value;

        return nullptr;

        // DataType* SrcType = Value->GetDataType();
        // llvm::Type* TargetLLVMType = CContext.GetLLVMType(Target);
        // llvm::Type* SrcLLVMType = CContext.GetLLVMType(SrcType);
        //
        // if (SrcType == Target)
        //     return Value;
        //
        // if (Cast<BoolType>(SrcType))
        // {
        //     if (Cast<IntegerType>(Target))
        //         return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
        //             TargetLLVMType), Target);
        //
        //     if (Cast<FloatingPointType>(Target))
        //         return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
        //             TargetLLVMType), Target);
        //
        //     if (Cast<PointerType>(Target))
        //         return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
        //                     llvm::ConstantPointerNull::get(
        //                     llvm::cast<llvm::PointerType>(SrcLLVMType))), Target);
        // }
        //
        // if (auto SrcIntType = Cast<IntegerType>(SrcType))
        // {
        //     if (Cast<BoolType>(Target))
        //         return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
        //                     llvm::ConstantInt::get(SrcLLVMType, 0)), Target);
        //
        //     if (auto TargetIntType = Cast<IntegerType>(Target))
        //     {
        //         if (SrcIntType->BitWidth < TargetIntType->BitWidth)
        //             return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
        //                 TargetLLVMType), Target);
        //
        //         return Create<TypedValue>(Builder.CreateTrunc(Value->GetValue(),
        //             TargetLLVMType), Target);
        //     }
        //
        //     if (Cast<FloatingPointType>(Target))
        //         return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
        //             TargetLLVMType), Target);
        // }
        //
        // if (auto SrcFloatType = Cast<FloatingPointType>(SrcType))
        // {
        //     if (Cast<BoolType>(Target))
        //         return Create<TypedValue>(Builder.CreateFCmpONE(Value->GetValue(),
        //                     llvm::ConstantFP::get(SrcLLVMType, 0.0 )), Target);
        //
        //     if (Cast<IntegerType>(Target))
        //         return Create<TypedValue>(Builder.CreateFPToSI(Value->GetValue(),
        //             TargetLLVMType), Target);
        //
        //     if (auto TargetFloatType = Cast<FloatingPointType>(Target))
        //     {
        //         if (SrcFloatType->BitWidth < TargetFloatType->BitWidth)
        //             return Create<TypedValue>(Builder.CreateFPExt(Value->GetValue(),
        //                 TargetLLVMType), Target);
        //
        //         return Create<TypedValue>(Builder.CreateFPTrunc(Value->GetValue(),
        //             TargetLLVMType), Target);
        //     }
        // }
        //
        // if (Cast<PointerType>(SrcType))
        //     if (auto DstPtrType = Cast<PointerType>(Target))
        //         if (DstPtrType->BaseType->GetCategory() == TypeCategory::VOID)
        //             return Value;
        //
        // // ERROR(std::format("Cannot convert '{}' to '{}'",
        // //     DataTypeUtils::TypeToString(SrcType), DataTypeUtils::TypeToString(Target)))
        //
        // return nullptr;
    }

    bool LLVMCompiler::CanImplicitCast(DataType* Src, DataType* Dst)
    {
        return false;
    }

    bool LLVMCompiler::GetIntegerValue(const ASTNode *Node, Int64 &Num)
    {
        if (const auto Int = Cast<const IntegerNode>(Node))
        {
            Num = Int->Value;
            return true;
        }

        return false;
    }

    void LLVMCompiler::FillArray(const ArrayNode *Array, llvm::AllocaInst *Alloca)
    {
        if (Array->Elements.empty())
            ERROR("Array empty")

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            TypedValue* El = CompileNode(Array->Elements[i]);

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Alloca, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }
    }
}
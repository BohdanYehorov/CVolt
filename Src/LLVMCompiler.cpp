//
// Created by bohdan on 03.01.26.
//

#include "Volt/Compiler/LLVMCompiler.h"

namespace Volt
{
    void LLVMCompiler::Compile()
    {
        if (CContext.HasErrors()) return;
        CompileNode(ASTTree);
    }

    IRValue *LLVMCompiler::GetCompileTimeValue(const ASTNode *Node)
    {
        if (!Node) return nullptr;

        if (auto Value = Cast<ExprResult>(Node->CompileTimeValue))
        {
            if (Value->IsEmpty()) return nullptr;

            DataType* Type = Value->GetType().GetType();

            switch (Type->GetCategory())
            {
                case TypeCategory::INTEGER:
                    return Create<IRValue>(llvm::ConstantInt::get(
                        CContext.GetLLVMType(Value->GetType().GetType()),
                        Value->GetType()->IsSignedIntegerType() ? Value->GetInt() : Value->GetUInt(),
                        Value->GetType()->IsSignedIntegerType()), Value->GetType().GetType());
                case TypeCategory::FLOATING_POINT:
                    return Create<IRValue>(llvm::ConstantFP::get(
                        CContext.GetLLVMType(Value->GetType().GetType()),
                        Value->GetFloat()), Value->GetType().GetType());
                case TypeCategory::BOOLEAN:
                    return Create<IRValue>(llvm::ConstantInt::get(
                        llvm::Type::getInt1Ty(Context),
                        Value->GetBool()), Value->GetType().GetType());
                default:
                    return nullptr;
            }
        }

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileNode(const ASTNode *Node)
    {
        if (IRValue* CompileTimeValue = GetCompileTimeValue(Node))
            return CompileTimeValue;

        if (auto Sequence = Cast<const SequenceNode>(Node))
        {
            for (auto Statement : Sequence->Statements)
                CompileNode(Statement);

            return nullptr;
        }
        if (const auto Block = Cast<const BlockNode>(Node))
            return CompileBlock(Block);
        if (const auto Char = Cast<const CharNode>(Node))
            return CompileChar(Char);
        if (const auto Int = Cast<const IntegerNode>(Node))
            return CompileInt(Int);
        if (const auto Bool = Cast<const BoolNode>(Node))
            return CompileBool(Bool);
        if (const auto Float = Cast<const FloatingPointNode>(Node))
            return CompileFloat(Float);
        if (const auto String = Cast<const StringNode>(Node))
            return CompileString(String);
        if (const auto Array = Cast<const ArrayNode>(Node))
            return CompileArray(Array);
        if (const auto Identifier = Cast<const IdentifierNode>(Node))
            return CompileIdentifier(Identifier);
        if (const auto Ref = Cast<const RefNode>(Node))
            return CompileRef(Ref);
        if (const auto Unref = Cast<const UnrefNode>(Node))
            return CompileUnref(Unref);
        if (const auto Prefix = Cast<const PrefixOpNode>(Node))
            return CompilePrefix(Prefix);
        if (const auto Suffix = Cast<const SuffixOpNode>(Node))
            return CompileSuffix(Suffix);
        if (const auto Unary = Cast<const UnaryOpNode>(Node))
            return CompileUnary(Unary);
        if (const auto Comparison = Cast<const ComparisonNode>(Node))
            return CompileComparison(Comparison);
        if (const auto Logical = Cast<const LogicalNode>(Node))
            return CompileLogical(Logical);
        if (const auto AssignOp = Cast<const AssignmentNode>(Node))
            return CompileAssignment(AssignOp);
        if (const auto BinaryOp = Cast<const BinaryOpNode>(Node))
            return CompileBinary(BinaryOp);
        if (const auto Call = Cast<const CallNode>(Node))
            return CompileCall(Call);
        if (const auto Subscript = Cast<const SubscriptNode>(Node))
            return CompileSubscript(Subscript);
        if (const auto ExplicitCast = Cast<const ExplicitCastNode>(Node))
            return CompileExplicitCast(ExplicitCast);
        if (const auto Var = Cast<const VariableNode>(Node))
            return CompileVariable(Var);
        if (const auto Function = Cast<const FunctionNode>(Node))
            return CompileFunction(Function);
        if (const auto Class = Cast<const ClassNode>(Node))
            return nullptr;
        if (const auto MemberAccess = Cast<const MemberAccessNode>(Node))
            return CompileMemberAccess(MemberAccess);
        if (const auto Return = Cast<const ReturnNode>(Node))
            return CompileReturn(Return);
        if (const auto If = Cast<const IfNode>(Node))
            return CompileIf(If);
        if (const auto While = Cast<const WhileNode>(Node))
            return CompileWhile(While);
        if (const auto For = Cast<const ForNode>(Node))
            return CompileFor(For);
        if (IsA<const BreakNode>(Node))
            return CompileBreak();
        if (IsA<const ContinueNode>(Node))
            return CompileContinue();

        VoltUnreachableFmt("Cannot resolve node: '{}'", Node->GetName());
    }

    IRValue *LLVMCompiler::CompileBlock(const BlockNode *Block)
    {
        EnterScope();

        if (CurrentFunction)
        {
            for (size_t i = 0; i < FunctionParams.size(); i++)
            {
                llvm::Argument* Arg = CurrentFunction->arg_begin() + i;
                llvm::Type* ArgType = Arg->getType();

                auto Type = FunctionParams[i];
                DeclareVariable(Arg->getName().str(), Create<IRValue>(Arg, Type, Builder));
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

    IRValue* LLVMCompiler::CompileInt(const IntegerNode *Int)
    {
        return Create<IRValue>(llvm::ConstantInt::get(
            CContext.GetLLVMType(Int->CompileTimeValue->GetType().GetType()), Int->Value, Int->IsSigned),
            Int->CompileTimeValue->GetType().GetType());
    }

    IRValue *LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        return Create<IRValue>(llvm::ConstantFP::get(
            CContext.GetLLVMType(Float->CompileTimeValue->GetType().GetType()), Float->Value),
            Float->CompileTimeValue->GetType().GetType());
    }

    IRValue *LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return Create<IRValue>(
            llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
            CContext.GetBoolType());
    }

    IRValue *LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return Create<IRValue>(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value), CContext.GetCharType());
    }

    IRValue *LLVMCompiler::CompileString(const StringNode *String)
    {
        return Create<IRValue>(Builder.CreateGlobalString(String->Value.str()),
            CContext.GetPointerType({ CContext.GetCharType(), 0 }));
    }

    IRValue *LLVMCompiler::CompileArray(const ArrayNode *Array)
    {
        if (Array->Elements.empty())
            VoltUnreachable("Array empty");

        llvm::Type* ArrType = nullptr;

        if (auto Type = Cast<ArrayType>(Array->CompileTimeValue->GetType().GetType()))
            ArrType = llvm::ArrayType::get(
                CContext.GetLLVMType(Type->BaseType.GetType()), Array->Elements.size());

        llvm::AllocaInst* Arr = Builder.CreateAlloca(ArrType);

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            IRValue* El = CompileToRValue(Array->Elements[i]);
            if (!El) return nullptr;

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }

        return Create<IRValue>(Arr, Array->CompileTimeValue->GetType().GetType());
    }

    IRValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.str();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
            return Iter->second;

        VoltUnreachableFmt("Cannot resolve symbol: '{}'", Value);
    }

    IRValue *LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        IRValue* Value = CompileNode(Ref->Target);
        if (!Value || !Value->IsLValue())
            VoltUnreachable("Cannot apply operator '$' to r-value");

       return Create<IRValue>(Value->GetValue(), CContext.GetPointerType({ Value->GetDataType(), 0 }));
    }

    IRValue *LLVMCompiler::CompileUnref(const UnrefNode *Unref)
    {
        IRValue *Value = CompileToRValue(Unref->Target);
        if (!Value) return nullptr;

        return Create<IRValue>(Value->GetValue(), Unref->CompileTimeValue->GetType().GetType(), true);
    }

    IRValue *LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        IRValue* Operand = CompileNode(Prefix->Operand);
        DataType* OperandType = Operand->GetDataType();
        IRValue* Value = Create<IRValue>(
            llvm::ConstantInt::get(CContext.GetLLVMType(OperandType), 1), OperandType);

        switch (Prefix->Type)
        {
            case OperatorType::INC:
                return Operand->CreateAssignment(Value, OperatorType::ADD_ASSIGN, Builder, CContext);
            case OperatorType::DEC:
                return Operand->CreateAssignment(Value, OperatorType::SUB_ASSIGN, Builder, CContext);
            default:
                VoltUnreachable("Invalid prefix operator");
        }
    }

    IRValue *LLVMCompiler::CompileSuffix(const SuffixOpNode *Suffix)
    {
        IRValue* Operand = CompileNode(Suffix->Operand);
        DataType* OperandType = Operand->GetDataType();
        IRValue* Value = Create<IRValue>(
            llvm::ConstantInt::get(CContext.GetLLVMType(OperandType), 1), OperandType);

        IRValue* Temp = Operand->GetRValue(Builder, CContext);
        switch (Suffix->Type)
        {
            case OperatorType::INC:
                Operand->CreateAssignment(Value, OperatorType::ADD_ASSIGN, Builder, CContext);
                return Temp;
            case OperatorType::DEC:
                Operand->CreateAssignment(Value, OperatorType::SUB_ASSIGN, Builder, CContext);
                return Temp;
            default:
                VoltUnreachable("Invalid prefix operator");
        }
    }

    IRValue *LLVMCompiler::CompileUnary(const UnaryOpNode *Unary)
    {
        using enum OperatorType;

        IRValue* Operand = CompileToRValue(Unary->Operand);
        if (!Operand)
            VoltUnreachable("Invalid operand");

        switch (Unary->Type)
        {
            case ADD:         return Operand;
            case SUB:         return Operand->CreateNeg(Builder, CContext);
            case BIT_NOT:     return Operand->CreateNot(Builder, CContext);
            case LOGICAL_NOT: return Operand->CreateLogicalNot(Builder, CContext);
            default: VoltUnreachable("Unknown unary operator");
        }
    }

    IRValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        IRValue* Left = CompileToRValue(Comparison->Left);
        IRValue* Right = CompileToRValue(Comparison->Right);

        if (!Right || !Left)
            VoltUnreachable("Invalid comparison operand");

        Left = Left->CastTo(Comparison->LeftOperandType, Builder, CContext);
        Right = Right->CastTo(Comparison->RightOperandType, Builder, CContext);

        if (!Right || !Left)
            VoltUnreachable("Invalid cast");

        return Left->CreateCmp(Right, Comparison->Type, Builder, CContext);
    }

    IRValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        IRValue* Left = CompileToRValue(Logical->Left);
        Left = Left->CastTo(Logical->LeftOperandType, Builder, CContext);
        if (!Left)
            VoltUnreachable("Invalid cast");

        llvm::BasicBlock* InsertBlock = Builder.GetInsertBlock();
        llvm::Function* Func = InsertBlock->getParent();

        switch (Logical->Type)
        {
            case OperatorType::LOGICAL_OR:
            {
                llvm::AllocaInst* Alloca = Builder.CreateAlloca(Builder.getInt1Ty());
                llvm::BasicBlock* OrFalseBlock = llvm::BasicBlock::Create(Context, "or.false", Func);
                llvm::BasicBlock* OrEndBlock = llvm::BasicBlock::Create(Context, "or.end", Func);

                Builder.CreateStore(Left->GetValue(), Alloca);

                Builder.CreateCondBr(Left->GetValue(), OrEndBlock, OrFalseBlock);

                Builder.SetInsertPoint(OrFalseBlock);
                IRValue* Right = CompileToRValue(Logical->Right);
                Right = Right->CastTo(Logical->RightOperandType, Builder, CContext);
                if (!Right)
                    VoltUnreachable("Invalid cast");
                Builder.CreateStore(Right->GetValue(), Alloca);

                Builder.CreateBr(OrEndBlock);
                Builder.SetInsertPoint(OrEndBlock);

                return Create<IRValue>(Builder.CreateLoad(
                    Alloca->getAllocatedType(), Alloca), Logical->CompileTimeValue->GetType().GetType());
            }
            case OperatorType::LOGICAL_AND:
            {
                llvm::AllocaInst* Alloca = Builder.CreateAlloca(Builder.getInt1Ty());
                llvm::BasicBlock* AndTrueBlock = llvm::BasicBlock::Create(Context, "and.true", Func);
                llvm::BasicBlock* AndEndBlock = llvm::BasicBlock::Create(Context, "and.end", Func);

                Builder.CreateStore(Left->GetValue(), Alloca);

                Builder.CreateCondBr(Left->GetValue(), AndTrueBlock, AndEndBlock);

                Builder.SetInsertPoint(AndTrueBlock);
                IRValue* Right = CompileToRValue(Logical->Right);
                Right = Right->CastTo(Logical->RightOperandType, Builder, CContext);
                if (!Right)
                    VoltUnreachable("Invalid cast");
                Builder.CreateStore(Right->GetValue(), Alloca);

                Builder.CreateBr(AndEndBlock);
                Builder.SetInsertPoint(AndEndBlock);

                return Create<IRValue>(Builder.CreateLoad(
                    Alloca->getAllocatedType(), Alloca), Logical->CompileTimeValue->GetType().GetType());
            }
            default:
                VoltUnreachable("Unknown logical operator");
        }
    }

    IRValue *LLVMCompiler::CompileAssignment(const AssignmentNode *Assignment)
    {
        IRValue* Value = CompileNode(Assignment->Left);
        IRValue* Right = CompileToRValue(Assignment->Right);

        if (!Value || !Right)
            VoltUnreachable("Invalid assignment values");

        Right = Right->CastTo(Value->GetDataType(), Builder, CContext);
        if (!Right)
            VoltUnreachable("Invalid cast");

        return Value->CreateAssignment(Right, Assignment->Type, Builder, CContext);
    }

    IRValue* LLVMCompiler::CompileBinary(const BinaryOpNode *BinaryOp)
    {
        using enum OperatorType;

        IRValue* Left = CompileToRValue(BinaryOp->Left);
        IRValue* Right = CompileToRValue(BinaryOp->Right);

        if (!Right || !Left)
            VoltUnreachable("invalid binary operand");

        Left = Left->CastTo(BinaryOp->LeftOperandType, Builder, CContext);
        Right = Right->CastTo(BinaryOp->RightOperandType, Builder, CContext);

        if (!Right || !Left)
            VoltUnreachable("invalid cast");

        switch (BinaryOp->Type)
        {
            case ADD:     return Left->CreateAdd(Right, Builder, CContext);
            case SUB:     return Left->CreateSub(Right, Builder, CContext);
            case MUL:     return Left->CreateMul(Right, Builder, CContext);
            case DIV:     return Left->CreateDiv(Right, Builder, CContext);
            case MOD:     return Left->CreateMod(Right, Builder, CContext);
            case BIT_AND: return Left->CreateBitAnd(Right, Builder, CContext);
            case BIT_OR:  return Left->CreateBitOr(Right, Builder, CContext);
            case BIT_XOR: return Left->CreateBitXor(Right, Builder, CContext);
            case RSHIFT:  return Left->CreateRShift(Right, Builder, CContext);
            case LSHIFT:  return Left->CreateLShift(Right, Builder, CContext);
            default: VoltUnreachable("Unknown binary operator");
        }
    }

    IRValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        const auto& Args = Call->Arguments;

        SmallVec8<llvm::Value*> LLVMArgs;

        LLVMArgs.reserve(Args.size());

        for (const auto Arg : Args)
        {
            IRValue* ArgValue = CompileNode(Arg)->CastOrBind(Arg->ExpectedType, Builder, CContext);
            if (!ArgValue)
                return nullptr;

            LLVMArgs.push_back(ArgValue->GetValue());
        }

        if (auto Func = Cast<FunctionCallee>(Call->ResolvedCallee))
            return Create<IRValue>(Builder.CreateCall(Func->Function, LLVMArgs),
                Func->ReturnType.GetType());

        if (auto BuiltinFunc = Cast<BuiltinFuncCallee>(Call->ResolvedCallee))
        {
            llvm::Function* LLVMFunc = Module->getFunction(BuiltinFunc->BaseName);
            return Create<IRValue>(Builder.CreateCall(LLVMFunc, LLVMArgs),
                BuiltinFunc->ReturnType.GetType() );
        }

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileMemberAccess(const MemberAccessNode *MemberAccess)
    {
        IRValue* Target = CompileNode(MemberAccess->Target);
        if (!Target) return nullptr;
        if (!Target->IsLValue())
            VoltUnreachable("Cannot access to r-value value");

        llvm::Value* Value = Target->GetValue();
        DataType* Type = Target->GetDataType();

        if (auto PtrType = Cast<PointerType>(Type))
        {
            Value = Builder.CreateLoad(Value->getType(), Value);
            Type = PtrType->BaseType.GetType();
        }

        return Create<IRValue>(Builder.CreateStructGEP(CContext.GetLLVMType(Type),
            Value, MemberAccess->ResolvedMemberIndex),
            MemberAccess->CompileTimeValue->GetType().GetType(), true);
    }

    IRValue *LLVMCompiler::CompileSubscript(const SubscriptNode *Subscript)
    {
        if (auto PtrType = Cast<PointerType>(Subscript->TargetType))
        {
            IRValue* Value = CompileToRValue(Subscript->Target);
            if (!Value) return nullptr;

            llvm::Value* LLVMValue = Value->GetValue();
            llvm::Type* ElType = CContext.GetLLVMType(PtrType->BaseType.GetType());
            IRValue* Index = CompileToRValue(Subscript->Index);
            if (!Index) return nullptr;
            llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
            return Create<IRValue>(ElPtr, PtrType->BaseType.GetType(), true);
        }

        if (auto ArrType = Cast<ArrayType>(Subscript->TargetType))
        {
            IRValue* Value = CompileNode(Subscript->Target);

            if (!Value || !Value->IsLValue())
                return nullptr;

            llvm::Value* LLVMValue = Value->GetValue();
            IRValue* Index = CompileToRValue(Subscript->Index);
            if (!Index) return nullptr;
            llvm::Value* ElPtr = Builder.CreateGEP(CContext.GetLLVMType(ArrType), LLVMValue,
                { Builder.getInt32(0), Index->GetValue() });
            return Create<IRValue>(ElPtr, ArrType->BaseType.GetType(), true);
        }

        return nullptr;
    }

    IRValue* LLVMCompiler::CompileExplicitCast(const ExplicitCastNode *ExplicitCast)
    {
        DataType* DstType = ExplicitCast->CompileTimeValue->GetType().GetType();
        IRValue* Target = CompileToRValue(ExplicitCast->Target);
        if (!Target) return nullptr;

        if (IRValue* Value = Target->CastTo(DstType, Builder, CContext))
            return Value;

        return Create<IRValue>(
            Builder.CreateBitCast(Target->GetValue(), CContext.GetLLVMType(DstType)), DstType);
    }

    IRValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        DataType* VarType = Var->Type->ResolvedType;

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::Type* Type = CContext.GetLLVMType(VarType);

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type);

        if (VarType->IsReferenceType())
        {
            IRValue* Value = CompileNode(Var->Value);
            if (!Value || !Value->IsLValue())
                return nullptr;

            DeclareVariable(Var->Name.str(), Value);
            return nullptr;
        }

        auto ArrType = Cast<ArrayType>(VarType);
        if (ArrType && Var->Value)
        {
            if (auto Arr = Cast<ArrayNode>(Var->Value))
            {
                if (ArrType->Length < Arr->Elements.size())
                    VoltUnreachable("Too many elements in array initializer");
                FillArray(Arr, Alloca);
            }
        }
        else if (Var->Value)
        {
            IRValue* Value = CompileToRValue(Var->Value);
            if (!Value) return nullptr;

            if (auto CastedValue = Value->CastTo(VarType, Builder, CContext))
                Builder.CreateStore(CastedValue->GetValue(), Alloca);
            else
                return nullptr;
        }

        DeclareVariable(Var->Name.str(),
            Create<IRValue>(Alloca, VarType, true));

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
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
            VoltUnreachableFmt("Function definition '{}' is unknown", FuncName);

        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        Builder.SetInsertPoint(Entry);
        CompileBlock(Cast<BlockNode>(Function->Body));

        llvm::BasicBlock* Bb = Builder.GetInsertBlock();

        if (!Bb->getTerminator())
        {
            if (RetType->isVoidTy())
                Builder.CreateRetVoid();
            else
                VoltUnreachableFmt("Function '{}' must return value", FuncName);
        }

        return nullptr;
    }

    IRValue* LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            IRValue* RetVal = CompileToRValue(Return->ReturnValue);
            if (!RetVal) return nullptr;

            Builder.CreateRet(RetVal->GetValue());
            return nullptr;
        }

        Builder.CreateRetVoid();
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileIf(const IfNode *If)
    {
        IRValue* Cond = CompileToRValue(If->Condition);
        if (!Cond) return nullptr;

        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond) return nullptr;

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

    IRValue *LLVMCompiler::CompileWhile(const WhileNode *While)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        llvm::BasicBlock* LoopHeader = llvm::BasicBlock::Create(Context, "loop.header", Func);
        Builder.CreateBr(LoopHeader);
        Builder.SetInsertPoint(LoopHeader);
        IRValue* Cond = CompileToRValue(While->Condition);
        if (!Cond) return nullptr;

        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond) return nullptr;

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

    IRValue *LLVMCompiler::CompileFor(const ForNode *For)
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
        IRValue* Cond = CompileToRValue(For->Condition);
        if (!Cond) return nullptr;

        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond) return nullptr;

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

    IRValue *LLVMCompiler::CompileBreak()
    {
        if (LoopEndStack.empty())
            VoltUnreachable("'break' used outside loop");

        Builder.CreateBr(LoopEndStack.top());
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileContinue()
    {
        if (LoopHeaderStack.empty())
            VoltUnreachable("'continue' used outside loop");

        Builder.CreateBr(LoopHeaderStack.top());
        return nullptr;
    }

    void LLVMCompiler::DeclareVariable(const std::string &Name, IRValue *Var)
    {
        if (auto Iter = std::find_if(
            ScopeStack.Back().Begin(), ScopeStack.Back().End(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.Back().End())
            VoltUnreachableFmt("This variable: '{}' has already declared in this scope", Name);

        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            ScopeStack.Back().Emplace(Name, Iter->second);
        else
            ScopeStack.Back().Emplace(Name, nullptr);

        SymbolTable[Name] = Var;
    }

    IRValue *LLVMCompiler::GetVariable(const std::string &Name)
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
            VoltUnreachable("Array empty");

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            IRValue* El = CompileToRValue(Array->Elements[i]);
            if (!El) return;

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Alloca, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }
    }
}
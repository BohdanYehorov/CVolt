//
// Created by bohdan on 03.01.26.
//

#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Utils/IRNameBuilder.h"

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
                case TypeCategory::Integer:
                    return Create<IRValue>(llvm::ConstantInt::get(
                        CContext.GetLLVMType(Value->GetType().GetType()),
                        Value->GetType()->IsSignedIntegerType() ? Value->GetInt() : Value->GetUInt(),
                        Value->GetType()->IsSignedIntegerType()), Value->GetType().GetType());
                case TypeCategory::FloatingPoint:
                    return Create<IRValue>(llvm::ConstantFP::get(
                        CContext.GetLLVMType(Value->GetType().GetType()),
                        Value->GetFloat()), Value->GetType().GetType());
                case TypeCategory::Boolean:
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
        if (const auto NullPointer = Cast<const NullPointerNode>(Node))
            return CompileNullPointer(NullPointer);
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
        if (const auto Construct = Cast<const ConstructNode>(Node))
            return CompileConstruct(Construct);
        if (const auto Var = Cast<const VariableNode>(Node))
            return CompileVariable(Var);
        if (const auto VarConstruct = Cast<const VariableConstructNode>(Node))
            return CompileVariableConstruct(VarConstruct);
        if (const auto Function = Cast<const FunctionNode>(Node))
            return CompileFunction(Function);
        if (const auto Class = Cast<const ClassNode>(Node))
            return CompileClass(Class);
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
        return Create<IRValue>(Builder.CreateGlobalString(String->Value),
            CContext.GetPointerType({ CContext.GetCharType(), 0 }));
    }

    IRValue *LLVMCompiler::CompileArray(const ArrayNode *Array)
    {
        if (Array->Elements.empty())
            VoltUnreachable("Array empty");

        llvm::Type* ArrType = nullptr;

        auto Type = Cast<ArrayType>(Array->CompileTimeValue->GetType().GetType());
        if (Type)
            ArrType = llvm::ArrayType::get(
                CContext.GetLLVMType(Type->GetBaseType().GetType()), Array->Elements.size());

        llvm::AllocaInst* Arr = Builder.CreateAlloca(Type);

        llvm::Value* Idx[2] = {
            Builder.GetInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            IRValue* El = CompileToRValue(Array->Elements[i]);
            if (!El) return nullptr;

            Idx[1] = Builder.GetInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            Builder.CreateStore(El, ElPtr);
        }

        return Create<IRValue>(Arr, Array->CompileTimeValue->GetType().GetType());
    }

    IRValue *LLVMCompiler::CompileNullPointer(const NullPointerNode *NullPtr)
    {
        return Create<IRValue>(llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(CContext.GetLLVMType(
            NullPtr->CompileTimeValue->GetType().GetType()))), NullPtr->CompileTimeValue->GetType().GetType());
    }

    IRValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        llvm::StringRef Value = Identifier->Value;

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
            return Iter->second;

        if (auto Iter = GlobalVariables.find(Value); Iter != GlobalVariables.end())
            return Iter->second;

        VoltUnreachableFmt("Cannot resolve symbol: '{}'", Value.str());
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
            case OperatorType::Inc:
                return Builder.CreateAssignment(Operand, Value, OperatorType::AddAssign);
            case OperatorType::Dec:
                return Builder.CreateAssignment(Operand, Value, OperatorType::SubAssign);
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

        IRValue* Temp = Builder.CreateLoadIfLValue(Operand);
        switch (Suffix->Type)
        {
            case OperatorType::Inc:
                Builder.CreateAssignment(Operand, Value, OperatorType::AddAssign);
                return Temp;
            case OperatorType::Dec:
                Builder.CreateAssignment(Operand, Value, OperatorType::SubAssign);
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
            case UnPlus:     return Operand;
            case UnMinus:    return Builder.CreateNeg(Operand);
            case BitNot:     return Builder.CreateNot(Operand);
            case LogicalNot: return Builder.CreateLogicalNot(Operand);
            default: VoltUnreachable("Unknown unary operator");
        }
    }

    IRValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        IRValue* Left = CompileToRValue(Comparison->Left);
        IRValue* Right = CompileToRValue(Comparison->Right);

        if (!Right || !Left)
            VoltUnreachable("Invalid comparison operand");

        Left = Builder.CreateCast(Left, Comparison->LeftOperandType);
        Right = Builder.CreateCast(Right, Comparison->RightOperandType);

        if (!Right || !Left)
            VoltUnreachable("Invalid cast");

        return Builder.CreateCmp(Left, Right, Comparison->Type);
    }

    IRValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        IRValue* Left = CompileToRValue(Logical->Left);
        Left = Builder.CreateCast(Left, Logical->LeftOperandType);
        if (!Left)
            VoltUnreachable("Invalid cast");

        llvm::BasicBlock* InsertBlock = Builder.GetInsertBlock();
        llvm::Function* Func = InsertBlock->getParent();

        switch (Logical->Type)
        {
            case OperatorType::LogicalOr:
            {
                llvm::AllocaInst* Alloca = Builder.CreateAlloca(CContext.GetBoolType());
                llvm::BasicBlock* OrFalseBlock = llvm::BasicBlock::Create(Context, "or.false", Func);
                llvm::BasicBlock* OrEndBlock = llvm::BasicBlock::Create(Context, "or.end", Func);

                Builder.CreateStore(Left, Alloca);

                Builder.CreateCondBr(Left->GetValue(), OrEndBlock, OrFalseBlock);

                Builder.SetInsertPoint(OrFalseBlock);
                IRValue* Right = CompileToRValue(Logical->Right);
                Right = Builder.CreateCast(Right, Logical->RightOperandType);
                if (!Right)
                    VoltUnreachable("Invalid cast");
                Builder.CreateStore(Right, Alloca);

                Builder.CreateBr(OrEndBlock);
                Builder.SetInsertPoint(OrEndBlock);

                return Builder.CreateLoad(CContext.GetBoolType(), Alloca);
            }
            case OperatorType::LogicalAnd:
            {
                llvm::AllocaInst* Alloca = Builder.CreateAlloca(CContext.GetBoolType());
                llvm::BasicBlock* AndTrueBlock = llvm::BasicBlock::Create(Context, "and.true", Func);
                llvm::BasicBlock* AndEndBlock = llvm::BasicBlock::Create(Context, "and.end", Func);

                Builder.CreateStore(Left, Alloca);

                Builder.CreateCondBr(Left->GetValue(), AndTrueBlock, AndEndBlock);

                Builder.SetInsertPoint(AndTrueBlock);
                IRValue* Right = CompileToRValue(Logical->Right);
                Right = Builder.CreateCast(Right, Logical->RightOperandType);
                if (!Right)
                    VoltUnreachable("Invalid cast");
                Builder.CreateStore(Right, Alloca);

                Builder.CreateBr(AndEndBlock);
                Builder.SetInsertPoint(AndEndBlock);

                return Builder.CreateLoad(CContext.GetBoolType(), Alloca);
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

        Right = Builder.CreateCast(Right, Value->GetDataType());
        if (!Right)
            VoltUnreachable("Invalid cast");

        return Builder.CreateAssignment(Value, Right, Assignment->Type);
    }

    IRValue* LLVMCompiler::CompileBinary(const BinaryOpNode *BinaryOp)
    {
        using enum OperatorType;

        IRValue* Left = CompileToRValue(BinaryOp->Left);
        IRValue* Right = CompileToRValue(BinaryOp->Right);

        if (!Right || !Left)
            VoltUnreachable("invalid binary operand");

        Left = Builder.CreateCast(Left, BinaryOp->LeftOperandType);
        Right = Builder.CreateCast(Right, BinaryOp->RightOperandType);

        if (!Right || !Left)
            VoltUnreachable("invalid cast");

        switch (BinaryOp->Type)
        {
            case Add:     return Builder.CreateAdd(Left, Right);
            case Sub:     return Builder.CreateSub(Left, Right);
            case Mul:     return Builder.CreateMul(Left, Right);
            case Div:     return Builder.CreateDiv(Left, Right);
            case Mod:     return Builder.CreateMod(Left, Right);
            case BitAnd:  return Builder.CreateAnd(Left, Right);
            case BitOr:   return Builder.CreateOr(Left, Right);
            case BitXor:  return Builder.CreateXor(Left, Right);
            case RShift:  return Builder.CreateRShift(Left, Right);
            case LShift:  return Builder.CreateLShift(Left, Right);
            default: VoltUnreachable("Unknown binary operator");
        }
    }

    IRValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        const auto& Args = Call->Arguments;

        SmallVec8<llvm::Value*> LLVMArgs;

        if (auto MemberAccess = Cast<MemberAccessNode>(Call->Callee))
        {
            IRValue* Res = CompileNode(MemberAccess);
            if (!Res || !Res->IsLValue()) return nullptr;
            VoltAssert(Res->GetDataType()->IsClassType());

            LLVMArgs.reserve(Args.size() + 1);
            LLVMArgs.push_back(Res->GetValue());
        }
        else
            LLVMArgs.reserve(Args.size());

        for (const auto Arg : Args)
        {
            IRValue* ArgValue = Builder.CreateCastOrBind(CompileNode(Arg), Arg->ExpectedType);
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
            Value = Builder.CreateLoad(Target)->GetValue();
            Type = PtrType->GetBaseType().GetType();
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
            llvm::Type* ElType = CContext.GetLLVMType(PtrType->GetBaseType().GetType());
            IRValue* Index = CompileToRValue(Subscript->Index);
            if (!Index) return nullptr;
            llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
            return Create<IRValue>(ElPtr, PtrType->GetBaseType().GetType(), true);
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
                { Builder.GetInt32(0), Index->GetValue() });
            return Create<IRValue>(ElPtr, ArrType->GetBaseType().GetType(), true);
        }

        return nullptr;
    }

    IRValue* LLVMCompiler::CompileExplicitCast(const ExplicitCastNode *ExplicitCast)
    {
        DataType* DstType = ExplicitCast->CompileTimeValue->GetType().GetType();
        IRValue* Target = CompileToRValue(ExplicitCast->Target);
        if (!Target) return nullptr;

        if (IRValue* Value = Builder.CreateCast(Target, DstType))
            return Value;

        return Create<IRValue>(
            Builder.CreateBitCast(Target->GetValue(), CContext.GetLLVMType(DstType)), DstType);
    }

    IRValue *LLVMCompiler::CompileConstruct(const ConstructNode *Construct)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());

        DataType* ClassTy = Construct->CompileTimeValue->GetType().GetType();
        auto StructTy = llvm::cast<llvm::StructType>(CContext.GetLLVMType(ClassTy));
        if (!StructTy)
            VoltUnreachable("Cannot construct non-struct type");
        llvm::AllocaInst* ClassAlloca = TmpBuilder.CreateAlloca(StructTy);

        ArgsVector<llvm::Value*> LLVMArgs;
        LLVMArgs.reserve(Construct->Args.size() + 1);
        LLVMArgs.push_back(ClassAlloca);
        for (const auto Arg : Construct->Args)
        {
            IRValue* ArgValue = Builder.CreateCastOrBind(CompileNode(Arg), Arg->ExpectedType);
            if (!ArgValue)
                return nullptr;

            LLVMArgs.push_back(ArgValue->GetValue());
        }

        if (auto Func = Cast<FunctionCallee>(Construct->ResolvedCallee))
            Builder.CreateCall(Func->Function, LLVMArgs);
        else
            VoltUnreachable("Invalid Callee");

        return Create<IRValue>(ClassAlloca, ClassTy, true);
    }

    IRValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        DataType* VarType = Var->Type->ResolvedType;
        llvm::Type* Type = CContext.GetLLVMType(VarType);

        if (!InFunction)
        {
            llvm::Constant* Constant = nullptr;

            if (Var->Value)
            {
                IRValue* Value = CompileToRValue(Var->Value);
                if (!Value) return nullptr;

                Value = Builder.CreateCast(Value, VarType);// Value->CastTo(VarType, Builder, CContext);
                if (!Value) return nullptr;

                Constant = llvm::cast<llvm::Constant>(Value->GetValue());
                if (!Constant)
                    VoltUnreachable("Cannot init global variable with non-constant value");
            }
            else
                Constant = llvm::Constant::getNullValue(Type);

            auto GlobalVar = new llvm::GlobalVariable(*Module, Type,
                false, llvm::GlobalVariable::ExternalLinkage, Constant, Var->Name);

            GlobalVariables[Var->Name] = Create<IRValue>(GlobalVar, VarType, true);
            return nullptr;
        }

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type);

        if (VarType->IsReferenceType())
        {
            IRValue* Value = CompileNode(Var->Value);
            if (!Value || !Value->IsLValue())
                return nullptr;

            DeclareVariable(Var->Name, Value);
            return nullptr;
        }

        auto ArrType = Cast<ArrayType>(VarType);
        if (ArrType && Var->Value)
        {
            if (auto Arr = Cast<ArrayNode>(Var->Value))
            {
                if (ArrType->GetLength() < Arr->Elements.size())
                    VoltUnreachable("Too many elements in array initializer");
                FillArray(Arr, Alloca);
            }
        }
        else if (Var->Value)
        {
            IRValue* Value = CompileToRValue(Var->Value);
            if (!Value) return nullptr;

            if (auto CastedValue = Builder.CreateCast(Value, VarType))
                Builder.CreateStore(CastedValue, Alloca);
            else
                return nullptr;
        }
        else if (Var->ResolvedConstructor)
            Builder.CreateCall(Var->ResolvedConstructor->Function, { Alloca });

        DeclareVariable(Var->Name,
            Create<IRValue>(Alloca, VarType, true));

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileVariableConstruct(const VariableConstructNode *Construct)
    {
        DataType* VarType = Construct->Type->ResolvedType;
        VoltAssert(VarType->IsClassType());
        llvm::Type* Type = CContext.GetLLVMType(VarType);

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type);

        ArgsVector<llvm::Value*> LLVMArgs;
        LLVMArgs.reserve(Construct->Arguments.size() + 1);
        LLVMArgs.push_back(Alloca);
        for (const auto Arg : Construct->Arguments)
        {
            IRValue* ArgValue = CompileNode(Arg);
            if (!ArgValue) return nullptr;
            ArgValue = Builder.CreateCastOrBind(ArgValue, Arg->ExpectedType);
            if (!ArgValue) return nullptr;

            LLVMArgs.push_back(ArgValue->GetValue());
        }

        if (Construct->ResolvedCallee)
            Builder.CreateCall(Construct->ResolvedCallee->Function, LLVMArgs);
        else
            VoltUnreachable("Invalid Callee");

        DeclareVariable(Construct->Name,
            Create<IRValue>(Alloca, VarType, true));

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        SmallVec8<llvm::Type*> Params;
        Params.reserve(Function->Params.size());

        IRNameBuilder NameBuilder(IRNameKind::Function);

        NameBuilder.AddName(Function->Name);
        for (const auto Param : Function->Params)
        {
            DataType* ParamType = Param->Type->ResolvedType;
            NameBuilder.AddParam(ParamType);
            Params.push_back(CContext.GetLLVMType(ParamType));
        }

        llvm::Type* RetType = CContext.GetLLVMType(Function->ReturnType->ResolvedType);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        llvm::StringRef FuncName = Function->Name;
        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, NameBuilder.GetIRName(), Module.get());

        const auto& FuncParams = Function->Params;

        EnterScope();
        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        Builder.SetInsertPoint(Entry);

        for (size_t i = 0; i < FuncParams.size(); i++)
        {
            DataType* ParamType = FuncParams[i]->Type->ResolvedType;
            auto Arg = Func->args().begin() + i;
            Arg->setName(FuncParams[i]->Name);
            DeclareVariable(FuncParams[i]->Name,
                Create<IRValue>(Arg, ParamType, Builder.Get()));
        }

        if (auto FuncCallee = Cast<FunctionCallee>(Function->ResolvedCallee))
            FuncCallee->Function = Func;
        else
            VoltUnreachableFmt("Function definition '{}' is unknown", FuncName.str());

        FunctionReturnType = Function->ReturnType->ResolvedType;
        InFunction = true;
        CompileBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = nullptr;
        InFunction = false;

        llvm::BasicBlock* Bb = Builder.GetInsertBlock();

        if (!Bb->getTerminator())
        {
            if (RetType->isVoidTy())
                Builder.CreateRetVoid();
            else
                VoltUnreachableFmt("Function '{}' must return value", FuncName.str());
        }

        ExitScope();
        return nullptr;
    }

    IRValue* LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            IRValue* RetVal = CompileToRValue(Return->ReturnValue);
            if (!RetVal) return nullptr;

            RetVal = Builder.CreateCastOrBind(RetVal, FunctionReturnType);
            if (!RetVal)
                VoltUnreachable("Invalid Cast");

            Builder.CreateRet(RetVal->GetValue());
            return nullptr;
        }

        Builder.CreateRetVoid();
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileMethod(const FunctionNode *Method, ClassType *Type)
    {
        DataType* ThisType = CContext.GetPointerType(Type);

        SmallVec8<llvm::Type*> Params;
        Params.reserve(Method->Params.size() + 1);
        Params.push_back(CContext.GetLLVMType(ThisType));

        IRNameBuilder NameBuilder(IRNameKind::Method);
        NameBuilder.AddName(Type->Name);
        NameBuilder.AddName(Method->Name);

        NameBuilder.AddParam(ThisType);

        for (const auto Param : Method->Params)
        {
            DataType* ParamType = Param->Type->ResolvedType;
            NameBuilder.AddParam(ParamType);
            Params.push_back(CContext.GetLLVMType(ParamType));
        }

        llvm::Type* RetType = CContext.GetLLVMType(Method->ReturnType->ResolvedType);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, NameBuilder.GetIRName(), Module.get());

        const auto& FuncParams = Method->Params;

        EnterScope();
        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        Builder.SetInsertPoint(Entry);

        DeclareVariable("this", Create<IRValue>(Func->args().begin(), ThisType, Builder.Get()));
        for (size_t i = 0; i < FuncParams.size() + 0; i++)
        {
            DataType* ParamType = FuncParams[i]->Type->ResolvedType;
            auto Arg = Func->args().begin() + i + 1;
            Arg->setName(FuncParams[i]->Name);
            DeclareVariable(FuncParams[i]->Name,
                Create<IRValue>(Arg, ParamType, Builder.Get()));
        }

        std::string FuncName = std::format("{}.{}", Type->Name.str(), Method->Name.str());
        if (auto FuncCallee = Cast<FunctionCallee>(Method->ResolvedCallee))
            FuncCallee->Function = Func;
        else
            VoltUnreachableFmt("Function definition '{}' is unknown", FuncName);

        FunctionReturnType = Method->ReturnType->ResolvedType;
        InFunction = true;
        CompileBlock(Cast<BlockNode>(Method->Body));
        FunctionReturnType = nullptr;
        InFunction = false;

        llvm::BasicBlock* Bb = Builder.GetInsertBlock();

        if (!Bb->getTerminator())
        {
            if (RetType->isVoidTy())
                Builder.CreateRetVoid();
            else
                VoltUnreachableFmt("Function '{}' must return value", FuncName);
        }

        ExitScope();
        return nullptr;
    }

    IRValue * LLVMCompiler::CompileConstructor(const ConstructorNode *Constructor, ClassType *Type)
    {
        DataType* ThisType = CContext.GetPointerType(Type);

        SmallVec8<llvm::Type*> Params;
        Params.reserve(Constructor->Params.size() + 1);
        Params.push_back(CContext.GetLLVMType(ThisType));

        IRNameBuilder NameBuilder(IRNameKind::Method);
        NameBuilder.AddName(Type->Name);
        NameBuilder.AddName(Type->Name);

        NameBuilder.AddParam(ThisType);

        for (const auto Param : Constructor->Params)
        {
            DataType* ParamType = Param->Type->ResolvedType;
            NameBuilder.AddParam(ParamType);
            Params.push_back(CContext.GetLLVMType(ParamType));
        }

        llvm::Type* RetType = llvm::Type::getVoidTy(Context);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, NameBuilder.GetIRName(), Module.get());

        const auto& FuncParams = Constructor->Params;

        EnterScope();
        llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        Builder.SetInsertPoint(Entry);

        DeclareVariable("this", Create<IRValue>(Func->args().begin(), ThisType, Builder.Get()));
        for (size_t i = 0; i < FuncParams.size() + 0; i++)
        {
            DataType* ParamType = FuncParams[i]->Type->ResolvedType;
            auto Arg = Func->args().begin() + i + 1;
            Arg->setName(FuncParams[i]->Name);
            DeclareVariable(FuncParams[i]->Name,
                Create<IRValue>(Arg, ParamType, Builder.Get()));
        }

        std::string FuncName = std::format("{}", Type->Name.str());
        if (auto FuncCallee = Cast<FunctionCallee>(Constructor->ResolvedCallee))
            FuncCallee->Function = Func;
        else
            VoltUnreachableFmt("Function definition '{}' is unknown", FuncName);

        FunctionReturnType = Constructor->ResolvedCallee->ReturnType.GetType();
        InFunction = true;
        CompileBlock(Cast<BlockNode>(Constructor->Body));
        FunctionReturnType = nullptr;
        InFunction = false;

        llvm::BasicBlock* Bb = Builder.GetInsertBlock();

        if (!Bb->getTerminator())
        {
            if (RetType->isVoidTy())
                Builder.CreateRetVoid();
            else
                VoltUnreachableFmt("Function '{}' must return value", FuncName);
        }

        ExitScope();
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileClass(const ClassNode *Class)
    {
        for (auto Method : Class->Methods)
            CompileMethod(Method, CContext.GetClassType(Class->Name));

        for (auto Constructor : Class->Constructors)
            CompileConstructor(Constructor, CContext.GetClassType(Class->Name));

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileIf(const IfNode *If)
    {
        IRValue* Cond = CompileToRValue(If->Condition);
        if (!Cond) return nullptr;

        Cond = Builder.CreateCast(Cond, CContext.GetBoolType());
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

        Cond = Builder.CreateCast(Cond, CContext.GetBoolType());
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

        Cond = Builder.CreateCast(Cond, CContext.GetBoolType());
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

    void LLVMCompiler::DeclareVariable(llvm::StringRef Name, IRValue *Var)
    {
        if (auto Iter = std::find_if(
            ScopeStack.Back().Begin(), ScopeStack.Back().End(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.Back().End())
            VoltUnreachableFmt("This variable: '{}' has already declared in this scope", Name.str());

        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            ScopeStack.Back().Emplace(Name, Iter->second);
        else
            ScopeStack.Back().Emplace(Name, nullptr);

        SymbolTable[Name] = Var;
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

    void LLVMCompiler::FillArray(const ArrayNode *Array, llvm::AllocaInst *Alloca)
    {
        if (Array->Elements.empty())
            VoltUnreachable("Array empty");

        llvm::Value* Idx[2] = {
            Builder.GetInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            IRValue* El = CompileToRValue(Array->Elements[i]);
            if (!El) return;

            Idx[1] = Builder.GetInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Alloca, Idx);
            Builder.CreateStore(El, ElPtr);
        }
    }
}

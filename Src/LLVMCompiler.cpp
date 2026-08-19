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
        CompileFunctionBodies();
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

        switch (Node->GetNodeKind())
        {
            case NodeKind::SequenceNode:
                for (auto Statement : StaticCast<SequenceNode>(Node)->Statements)
                    CompileNode(Statement);
                return nullptr;
            case NodeKind::BlockNode:
                CompileBlock(StaticCast<BlockNode>(Node));
                return nullptr;
            case NodeKind::IntegerNode:
                return CompileInt(StaticCast<IntegerNode>(Node));
            case NodeKind::FloatingPointNode:
                return CompileFloat(StaticCast<FloatingPointNode>(Node));
            case NodeKind::BoolNode:
                return CompileBool(StaticCast<BoolNode>(Node));
            case NodeKind::CharNode:
                return CompileChar(StaticCast<CharNode>(Node));
            case NodeKind::StringNode:
                return CompileString(StaticCast<StringNode>(Node));
            case NodeKind::ArrayNode:
                return CompileArray(StaticCast<ArrayNode>(Node));
            case NodeKind::NullPointerNode:
                return CompileNullPointer(StaticCast<NullPointerNode>(Node));
            case NodeKind::IdentifierNode:
                return CompileIdentifier(StaticCast<IdentifierNode>(Node));
            case NodeKind::RefNode:
                return CompileRef(StaticCast<RefNode>(Node));
            case NodeKind::UnrefNode:
                return CompileUnref(StaticCast<UnrefNode>(Node));
            case NodeKind::SuffixOpNode:
                return CompileSuffix(StaticCast<SuffixOpNode>(Node));
            case NodeKind::PrefixOpNode:
                return CompilePrefix(StaticCast<PrefixOpNode>(Node));
            case NodeKind::UnaryOpNode:
                return CompileUnary(StaticCast<UnaryOpNode>(Node));
            case NodeKind::AssignmentNode:
                return CompileAssignment(StaticCast<AssignmentNode>(Node));
            case NodeKind::ComparisonNode:
                return CompileComparison(StaticCast<ComparisonNode>(Node));
            case NodeKind::LogicalNode:
                return CompileLogical(StaticCast<LogicalNode>(Node));
            case NodeKind::BinaryOpNode:
                return CompileBinary(StaticCast<BinaryOpNode>(Node));
            case NodeKind::CallNode:
                return CompileCall(StaticCast<CallNode>(Node));
            case NodeKind::SubscriptNode:
                return CompileSubscript(StaticCast<SubscriptNode>(Node));
            case NodeKind::ExplicitCastNode:
                return CompileExplicitCast(StaticCast<ExplicitCastNode>(Node));
            case NodeKind::VariableNode:
                return CompileVariable(StaticCast<VariableNode>(Node));
            case NodeKind::VariableConstructNode:
                return CompileVariableConstruct(StaticCast<VariableConstructNode>(Node));
            case NodeKind::FunctionNode:
                return CompileFunction(StaticCast<FunctionNode>(Node));
            case NodeKind::ClassNode:
                return CompileClass(StaticCast<ClassNode>(Node));
            case NodeKind::MemberAccessNode:
                return CompileMemberAccess(StaticCast<MemberAccessNode>(Node));
            case NodeKind::IfNode:
                return CompileIf(StaticCast<IfNode>(Node));
            case NodeKind::WhileNode:
                return CompileWhile(StaticCast<WhileNode>(Node));
            case NodeKind::ForNode:
                return CompileFor(StaticCast<ForNode>(Node));
            case NodeKind::ReturnNode:
                return CompileReturn(StaticCast<ReturnNode>(Node));
            case NodeKind::BreakNode:
                return CompileBreak();
            case NodeKind::ContinueNode:
                return CompileContinue();
        }

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

        OperatorType Op = Assignment->Type;

        if (Op == OperatorType::Assign)
            return Assign(Value, Assignment->Right);

        IRValue* Right = CompileToRValue(Assignment->Right);

        Right = Builder.CreateCast(Right, Value->GetDataType());
        if (!Right) VoltUnreachable("Invalid cast");

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
        const auto& ArgNodes = Call->Arguments;

        if (auto MemberAccess = Cast<MemberAccessNode>(Call->Callee))
            return CallMethod(MemberAccess, StaticCast<MethodCallee>(Call->ResolvedCallee), ArgNodes);

        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
            if (auto FuncCallee = Cast<FunctionCallee>(Call->ResolvedCallee))
                if (auto RetVal = CallConstructor(Identifier, FuncCallee, ArgNodes))
                    return RetVal;

        ArgsVector<llvm::Value*> Args;
        Args.reserve(ArgNodes.size() + Call->ResolvedCallee->ReturnType->IsAggregateType());
        llvm::Value* RetAlloca = CreateRetValueForAggregateType(
            Call->ResolvedCallee->ReturnType.GetType(), Args);
        FillArgs(ArgNodes, Args);
        llvm::Value* RetVal = Builder.CreateCall(Call->ResolvedCallee, Args, Module);
        return Create<IRValue>(RetAlloca == nullptr ? RetVal : RetAlloca,
            Call->ResolvedCallee->ReturnType.GetType());

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileMemberAccess(const MemberAccessNode *MemberAccess)
    {
        llvm::Value* Value;
        ClassType* ClassTy;
        if (!GetClassFromMemberAccess(MemberAccess, Value, ClassTy)) return nullptr;

        const Field& F = ClassTy->GetField(MemberAccess->ResolvedMemberIndex);

        llvm::Value* Res = Builder.CreateGEP(CContext.GetLLVMType(ClassTy),
            Value, { Builder.GetInt64(0), Builder.GetInt64(F.Offset) });

        return Create<IRValue>(Builder.CreateBitCast(Res,
            llvm::PointerType::get(Context, 0)),
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

        if (ExplicitCast->IsBitCast)
        {
            if (Target->GetDataType()->IsArrayType() || Target->GetDataType()->IsPointerType() ||
                DstType->IsArrayType()               || DstType->IsPointerType())
                return Create<IRValue>(Target->GetValue(), DstType);
            return Create<IRValue>(Builder.CreateBitCast(
               Target->GetValue(), CContext.GetLLVMType(DstType)), DstType);
        }

        if (IRValue* Value = Builder.CreateCast(Target, DstType))
            return Value;

        VoltUnreachableFmt("Cannot convert {} to {}", Target->GetDataType()->ToString(), DstType->ToString());
    }

    IRValue *LLVMCompiler::CompileConstruct(const ConstructNode *Construct)
    {
        DataType* ClassTy = Construct->CompileTimeValue->GetType().GetType();
        llvm::AllocaInst* ClassAlloca = Builder.CreateAlloca(ClassTy);

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

                Value = Builder.CreateCast(Value, VarType);
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

        if (VarType->IsReferenceType())
        {
            IRValue* Value = CompileNode(Var->Value);
            if (!Value || !Value->IsLValue())
                return nullptr;

            DeclareVariable(Var->Name, Value);
            return nullptr;
        }

        llvm::AllocaInst* Alloca = Builder.CreateAlloca(VarType);
        IRValue* VarValue = Create<IRValue>(Alloca, VarType, true);

        if (Var->ResolvedConstructor)
            Builder.CreateCall(Var->ResolvedConstructor->Function, { Alloca });
        else if (Var->Value)
            Assign(VarValue, Var->Value);

        DeclareVariable(Var->Name, VarValue);
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileVariableConstruct(const VariableConstructNode *Construct)
    {
        DataType* VarType = Construct->Type->ResolvedType;
        VoltAssert(VarType->IsClassType());
        llvm::AllocaInst* Alloca = Builder.CreateAlloca(VarType);

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
        ArgsVector<llvm::Type*> Params;
        CreateFunction(Function->Name, Function->Params, Function->ReturnType->ResolvedType,
            Function->Body, Function->ResolvedCallee, Params);
        return nullptr;
    }

    IRValue* LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            if (FunctionReturnType->IsArrayType() || FunctionReturnType->IsClassType())
            {
                IRValue* RetVal = CompileNode(Return->ReturnValue);
                if (!RetVal) return nullptr;

                llvm::Function* Func = Builder.GetInsertBlock()->getParent();

                llvm::Argument* Arg = Func->arg_begin();

                Builder.CreateMemCpy(Arg, Arg->getParamAlign(),
                    RetVal->GetValue(), Arg->getParamAlign(), 8);
                Builder.CreateRetVoid();
                return nullptr;
            }

            IRValue* RetVal = CompileNode(Return->ReturnValue);
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

        ArgsVector<llvm::Type*> Params;
        CreateFunction(Method->Name, Method->Params, Method->ReturnType->ResolvedType,
            Method->Body, Method->ResolvedCallee, Params, ThisType);

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileConstructor(const ConstructorNode *Constructor, ClassType *Type)
    {
        DataType* ThisType = CContext.GetPointerType(Type);

        ArgsVector<llvm::Type*> Params;
        CreateFunction(Type->GetName(), Constructor->Params, CContext.GetVoidType(),
            Constructor->Body, Constructor->ResolvedCallee, Params, ThisType);

        return nullptr;
    }

    IRValue *LLVMCompiler::CompileClass(const ClassNode *Class)
    {
        ClassType* ClassTy = CContext.GetClassType(Class->Name);

        for (auto Method : Class->Methods)
            CompileMethod(Method, ClassTy);

        for (auto Constructor : Class->Constructors)
            CompileConstructor(Constructor, ClassTy);

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

    bool LLVMCompiler::GetClassFromMemberAccess(const MemberAccessNode* MemberAccess,
        llvm::Value *&Value, ClassType *&ClassTy)
    {
        IRValue* Target = CompileNode(MemberAccess->Target);
        if (!Target) return false;
        VoltAssert(Target->IsLValue() && "Cannot access to r-value value");

        Value = Target->GetValue();
        DataType* Type = Target->GetDataType();

        if (auto PtrType = Cast<PointerType>(Type))
        {
            Value = Builder.CreateLoad(Target)->GetValue();
            Type = PtrType->GetBaseType().GetType();
        }

        ClassTy = StaticCast<ClassType>(Type);
        return true;
    }

    void LLVMCompiler::CreateFunction(llvm::StringRef Name, llvm::ArrayRef<ParamNode *> Params,
        DataType *ReturnType, BlockNode* Body, CalleeBase* Callee,
        ArgsVector<llvm::Type *> &LLVMParams, DataType *ThisType)
    {
        IRNameBuilder NameBuilder(ThisType == nullptr ? IRNameKind::Function : IRNameKind::Method);

        bool IsAggregateRetType = ReturnType->IsAggregateType();
        llvm::Type* RetType = CContext.GetLLVMType(ReturnType);
        LLVMParams.reserve(Params.size() + IsAggregateRetType + (ThisType != nullptr));

        if (IsAggregateRetType)
            LLVMParams.push_back(llvm::PointerType::get(Context, 0));

        if (ThisType)
        {
            LLVMParams.push_back(CContext.GetLLVMType(ThisType));
            NameBuilder.AddParam(ThisType);
        }
        else
            LLVMParams.reserve(Params.size());

        NameBuilder.AddName(Name);
        for (const auto Param : Params)
        {
            DataType* ParamType = Param->Type->ResolvedType;
            NameBuilder.AddParam(ParamType);
            LLVMParams.push_back(CContext.GetLLVMType(ParamType));
        }

        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            IsAggregateRetType ? llvm::Type::getVoidTy(Context) : RetType, LLVMParams, false);

        llvm::Function* Func = llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage,
            NameBuilder.GetIRName(), Module.get());

        if (auto FuncCallee = Cast<FunctionCallee>(Callee))
            FuncCallee->Function = Func;
        else
            VoltUnreachableFmt("Function definition '{}' is unknown", Name.str());

        FunctionBlocks.Emplace(Name, Body, Func, ReturnType, ThisType, Params);
    }

    IRValue* LLVMCompiler::Assign(IRValue *Var, ASTNode *Value)
    {
        DataType* VarType = Var->GetDataType();

        if (auto ArrayTy = Cast<ArrayType>(VarType))
        {
            if (auto Arr = Cast<ArrayNode>(Value))
            {
                if (ArrayTy->GetLength() < Arr->Elements.size())
                    VoltUnreachable("Too many elements in array initializer");
                FillArray(Arr, cast<llvm::AllocaInst>(Var->GetValue()));

                return Var;
            }
        }

        if (auto ClassTy = Cast<ClassType>(VarType))
        {
            if (auto Call = Cast<CallNode>(Value))
            {
                if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
                {
                    if (Identifier->Value == ClassTy->GetName())
                    {
                        ArgsVector<llvm::Value*> Args;
                        Args.reserve(Call->Arguments.size() + 1);
                        Args.push_back(Var->GetValue());

                        for (const auto Arg : Call->Arguments)
                        {
                            IRValue* ArgValue = Builder.CreateCastOrBind(CompileNode(Arg), Arg->ExpectedType);
                            if (!ArgValue)
                                return nullptr;

                            Args.push_back(ArgValue->GetValue());
                        }

                        if (auto Func = Cast<FunctionCallee>(Call->ResolvedCallee))
                        {
                            Builder.CreateCall(Func->Function, Args);
                            return Var;
                        }
                    }
                }
            }
        }

        if (VarType->IsArrayType() || VarType->IsClassType())
        {
            IRValue* Val = CompileNode(Value);
            if (!Val) return nullptr;

            VoltAssert(Var->IsLValue() && "Cannot assign r-value to this type");

            if (auto CastedValue = Builder.CreateCast(Val, VarType))
                Builder.CreateMemCpy(Var, CastedValue);
            else
                VoltUnreachableFmt("Cannot convert {} to {}",
                    Val->GetDataType()->ToString(), VarType->ToString());

            return Val;
        }

        IRValue* Val = CompileToRValue(Value);
        if (!Val) return nullptr;

        if (auto CastedValue = Builder.CreateCast(Val, VarType))
            Builder.CreateStore(CastedValue, Var->GetValue());
        else
            VoltUnreachableFmt("Cannot convert {} to {}",
                Val->GetDataType()->ToString(), VarType->ToString());

        return Val;
    }

    IRValue *LLVMCompiler::CallMethod(MemberAccessNode *Target, MethodCallee *Callee, llvm::ArrayRef<ASTNode*> ArgNodes)
    {
        ArgsVector<llvm::Value*> Args;

        llvm::Value* Value;
        ClassType* ClassTy;
        if (!GetClassFromMemberAccess(Target, Value, ClassTy)) return nullptr;
        if (Callee->Owner != ClassTy)
        {
            size_t Offset = ClassTy->GetImplementedFieldOffset(Callee->Owner);
            if (Offset != 0)
                Value = Builder.CreateGEP(CContext.GetLLVMType(Callee->Owner), Value,
        { Builder.GetInt64(0), Builder.GetInt64(Offset) });
        }

        Args.reserve(Args.size() + 1 + Callee->ReturnType->IsAggregateType());
        llvm::Value* RetAlloca = CreateRetValueForAggregateType(Callee->ReturnType.GetType(), Args);
        Args.push_back(Value);
        FillArgs(ArgNodes, Args);
        llvm::Value* RetVal = Builder.CreateCall(Callee->Function, Args);

        return Create<IRValue>(RetAlloca == nullptr ? RetVal : RetAlloca, Callee->ReturnType.GetType());
    }

    IRValue * LLVMCompiler::CallConstructor(IdentifierNode *Target, FunctionCallee *Callee,
        llvm::ArrayRef<ASTNode *> ArgNodes)
    {
        ClassType* ClassTy = CContext.GetClassType(Target->Value);
        if (!ClassTy) return nullptr;

        ArgsVector<llvm::Value*> Args;

        llvm::AllocaInst* Alloca = Builder.CreateAlloca(ClassTy);
        Args.reserve(Args.size() + 1);
        Args.push_back(Alloca);
        FillArgs(ArgNodes, Args);
        Builder.CreateCall(Callee->Function, Args);
        return Create<IRValue>(Alloca, ClassTy, true);
    }

    llvm::AllocaInst *LLVMCompiler::CreateRetValueForAggregateType(
        DataType *RetType, ArgsVector<llvm::Value *> &Args)
    {
        if (!RetType->IsAggregateType()) return nullptr;
        llvm::AllocaInst* Alloca = Builder.CreateAlloca(RetType);
        Args.push_back(Alloca);
        return Alloca;
    }

    void LLVMCompiler::CompileFunctionBodies()
    {
        for (const auto& Data : FunctionBlocks)
        {
            EnterScope();
            llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Data.Func);
            Builder.SetInsertPoint(Entry);

            bool IsAggregateRetType = Data.ReturnType->IsAggregateType();
            if (Data.ThisType)
            {
                DeclareVariable("this", Create<IRValue>(Data.Func->args().begin() + IsAggregateRetType,
                    Data.ThisType, Builder.Get()));
            }

            size_t Offset = (Data.ThisType != nullptr) + IsAggregateRetType;

            if (IsAggregateRetType)
            {
                Data.Func->addParamAttr(0, llvm::Attribute::getWithStructRetType(
                    Context, CContext.GetLLVMType(Data.ReturnType)));
                Data.Func->addParamAttr(0, llvm::Attribute::AttrKind::NoAlias);
                Data.Func->addParamAttr(0, llvm::Attribute::getWithAlignment(
                    Context, llvm::Align(8)));
            }

            for (size_t i = 0; i < Data.Params.size(); i++)
            {
                DataType* ParamType = Data.Params[i]->Type->ResolvedType;
                auto Arg = Data.Func->args().begin() + i + Offset;
                Arg->setName(Data.Params[i]->Name);
                DeclareVariable(Data.Params[i]->Name,
                    Create<IRValue>(Arg, ParamType, Builder.Get()));
            }

            FunctionReturnType = Data.ReturnType;
            InFunction = true;
            CompileBlock(Data.BodyNode);
            FunctionReturnType = nullptr;
            InFunction = false;

            llvm::BasicBlock* Bb = Builder.GetInsertBlock();

            if (!Bb->getTerminator())
            {
                if (Data.Func->getReturnType()->isVoidTy())
                    Builder.CreateRetVoid();
                else
                    VoltUnreachableFmt("Function '{}' must return value", Data.Name.str());
            }

            ExitScope();
        }
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

    void LLVMCompiler::FillArgs(llvm::ArrayRef<ASTNode *> ParamNodes, ArgsVector<llvm::Value *> &Params)
    {
        for (const auto Param : ParamNodes)
        {
            IRValue* ArgValue = Builder.CreateCastOrBind(CompileNode(Param), Param->ExpectedType);
            if (!ArgValue) return;
            Params.push_back(ArgValue->GetValue());
        }
    }
}

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

    IRValue *LLVMCompiler::CompileNode(const ASTNode *Node)
    {
        if (Node->CompileTimeValue && !Node->CompileTimeValue->IsEmpty)
        {
            ExprResult* Value = Node->CompileTimeValue;
            DataType* Type = Value->Type.GetType();

            switch (Type->GetCategory())
            {
                case TypeCategory::INTEGER:
                    return Create<IRValue>(llvm::ConstantInt::get(
                    CContext.GetLLVMType(Value->Type.GetType()), Value->Int), Value->Type.GetType());
                case TypeCategory::FLOATING_POINT:
                    return Create<IRValue>(llvm::ConstantFP::get(
                        CContext.GetLLVMType(Value->Type.GetType()), Value->Float), Value->Type.GetType());
                case TypeCategory::BOOLEAN:
                    return Create<IRValue>(llvm::ConstantInt::get(
                        llvm::Type::getInt1Ty(Context), Value->Bool), Value->Type.GetType());
                default:
                    ERROR(std::format("Invalid compile time value type: {}", Value->Type->ToString()));
            }
        }

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
        if (const auto Return = Cast<const ReturnNode>(Node))
            return CompileReturn(Return);
        if (const auto If = Cast<const IfNode>(Node))
            return CompileIf(If);
        if (const auto While = Cast<const WhileNode>(Node))
            return CompileWhile(While);
        if (const auto For = Cast<const ForNode>(Node))
            return CompileFor(For);
        if (Cast<const BreakNode>(Node))
            return CompileBreak();
        if (Cast<const ContinueNode>(Node))
            return CompileContinue();

        ERROR("Cannot resolve node: '" + Node->GetName() + "'");
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
                // if (auto RefType = Cast<ReferenceType>(Type))
                // {
                //     DeclareVariable(Arg->getName().str(),
                //         Create<IRValue>(Arg, RefType->BaseType.GetType(), true));
                //     continue;
                // }
                //
                // llvm::AllocaInst* Alloca = Builder.CreateAlloca(ArgType, nullptr, Arg->getName());
                //
                // Builder.CreateStore(Arg, Alloca);
                // DeclareVariable(Arg->getName().str(),
                //     Create<IRValue>(Alloca, FunctionParams[i], true));

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
            CContext.GetLLVMType(Int->CompileTimeValue->Type.GetType()), Int->Value),
            Int->CompileTimeValue->Type.GetType());
    }

    IRValue *LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        return Create<IRValue>(llvm::ConstantFP::get(
            CContext.GetLLVMType(Float->CompileTimeValue->Type.GetType()), Float->Value),
            Float->CompileTimeValue->Type.GetType());
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
            ERROR("Array empty")

        llvm::Type* ArrType = nullptr;

        if (auto Type = Cast<ArrayType>(Array->CompileTimeValue->Type.GetType()))
            ArrType = llvm::ArrayType::get(
                CContext.GetLLVMType(Type->BaseType.GetType()), Array->Elements.size());

        llvm::AllocaInst* Arr = Builder.CreateAlloca(ArrType);

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            IRValue* El = CompileNode(Array->Elements[i]);
            if (!El)
                return nullptr;

            El = El->GetRValue(Builder, CContext);

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }

        return Create<IRValue>(Arr, Array->CompileTimeValue->Type.GetType());
    }

    IRValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.str();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        {
            /* IRValue* Var = */ return Iter->second;
            // return Create<IRValue>(Builder.CreateLoad(CContext.GetLLVMType(Var->GetDataType()),
            //             Var->GetValue(), Value + "_val"), Var->GetDataType());
        }

        ERROR("Cannot resolve symbol: '" + Value + "'")
    }

    IRValue *LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        IRValue* LValue = CompileNode(Ref->Target);// GetLValue(Ref->Target);
        if (!LValue || !LValue->IsLValue())
            ERROR("Cannot apply operator '$' to r-value")

       return Create<IRValue>(LValue->GetValue(), CContext.GetPointerType({LValue->GetDataType(), 0}));
    }

    IRValue *LLVMCompiler::CompileUnref(const UnrefNode *Unref)
    {
        IRValue *TValue = CompileNode(Unref->Target);
        if (!TValue)
            return nullptr;

        // llvm::Value* Value = TValue->GetValue();

        return Create<IRValue>(TValue->GetValue(), Unref->CompileTimeValue->Type.GetType(), true);
        // return Create<IRValue>(Builder.CreateLoad(
        //         CContext.GetLLVMType(Unref->CompileTimeValue->Type.GetType()), Value),
        //         Unref->CompileTimeValue->Type.GetType());
    }

    IRValue *LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        IRValue* LValue = CompileNode(Prefix->Operand);
        if (!LValue || !LValue->IsLValue())
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

        return Create<IRValue>(Value, LValue->GetDataType());
    }

    IRValue *LLVMCompiler::CompileSuffix(const SuffixOpNode *Suffix)
    {
        IRValue* LValue = CompileNode(Suffix->Operand);
        if (!LValue || !LValue->IsLValue())
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

        return Create<IRValue>(Temp, LValue->GetDataType());
    }

    IRValue *LLVMCompiler::CompileUnary(const UnaryOpNode *Unary)
    {
        IRValue* TValue = CompileNode(Unary->Operand);
        TValue = TValue->GetRValue(Builder, CContext);

        llvm::Value* Value = TValue->GetValue();
        DataType* Type = TValue->GetDataType();

        DataType* BoolType = CContext.GetBoolType();

        bool IsFP = Cast<FloatingPointType>(Type);

        switch (Unary->Type)
        {
            case OperatorType::ADD:         return TValue;
            case OperatorType::SUB:         return Create<IRValue>(IsFP ?
                                            Builder.CreateFNeg(Value) :
                                            Builder.CreateNeg(Value), Unary->CompileTimeValue->Type.GetType());
            case OperatorType::LOGICAL_NOT: return Create<IRValue>(Builder.CreateNot(
                                               /*ImplicitCast(TValue, BoolType)->GetValue())*/
                                               TValue->CastLLVM(BoolType, Builder, CContext)), Unary->CompileTimeValue->Type.GetType());
            case OperatorType::BIT_NOT:     return Create<IRValue>(Builder.CreateNot(Value), Unary->CompileTimeValue->Type.GetType());
            default: ERROR("Unknown unary operator")
        }
    }

    IRValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        IRValue* Left = CompileNode(Comparison->Left);
        Left = Left->GetRValue(Builder, CContext);

        IRValue* Right = CompileNode(Comparison->Right);
        Right = Right->GetRValue(Builder, CContext);

        Left = Left->CastTo(Comparison->LeftOperandType, Builder, CContext);
        Right = Right->CastTo(Comparison->RightOperandType, Builder, CContext);

        if (!Right || !Left)
            return nullptr;

        // if (!Left->CastTo(Comparison->LeftOperandType, Builder, CContext))
        //     return nullptr;
        //
        // if (!Right->CastTo(Comparison->RightOperandType, Builder, CContext))
        //     return nullptr;

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
            case OperatorType::EQ:  return Create<IRValue>(IsFP ?
                                Builder.CreateFCmpOEQ(LeftVal, RightVal) :
                                Builder.CreateICmpEQ(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            case OperatorType::NEQ: return Create<IRValue>(IsFP ?
                                Builder.CreateFCmpONE(LeftVal, RightVal) :
                                Builder.CreateICmpNE(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            case OperatorType::LT:  return Create<IRValue>(IsFP ?
                                Builder.CreateFCmpOLT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLT(LeftVal, RightVal) :
                                Builder.CreateICmpULT(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            case OperatorType::LTE: return Create<IRValue>( IsFP ?
                                Builder.CreateFCmpOLE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLE(LeftVal, RightVal) :
                                Builder.CreateICmpULE(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            case OperatorType::GT:  return Create<IRValue>(IsFP ?
                                Builder.CreateFCmpOGT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGT(LeftVal, RightVal) :
                                Builder.CreateICmpUGT(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            case OperatorType::GTE: return Create<IRValue>(IsFP ?
                                Builder.CreateFCmpOGE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGE(LeftVal, RightVal) :
                                Builder.CreateICmpUGE(LeftVal, RightVal),
                                Comparison->CompileTimeValue->Type.GetType());
            default: ERROR("Unknown comparison operator")
        }
    }

    IRValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        IRValue* Left = CompileNode(Logical->Left);
        Left = Left->GetRValue(Builder, CContext);

        IRValue* Right = CompileNode(Logical->Right);
        Right = Right->GetRValue(Builder, CContext);

        Left = Left->CastTo(Logical->LeftOperandType, Builder, CContext);
        Right = Right->CastTo(Logical->RightOperandType, Builder, CContext);

        if (!Right || !Left)
            return nullptr;

        // if (!Left->CastTo(Logical->LeftOperandType, Builder, CContext))
        //     return nullptr;
        //
        // if (!Right->CastTo(Logical->RightOperandType, Builder, CContext))
        //     return nullptr;

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
                // IRValue* Right = CompileNode(Logical->Right);
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
                // return Create<IRValue>(Phi, Logical->CompileTimeValue->Type);

                return Create<IRValue>(
                    Builder.CreateOr(Left->GetValue(), Right->GetValue()), Logical->CompileTimeValue->Type.GetType());
            }
            case OperatorType::LOGICAL_AND:
            {
                // auto* AndRhsBB = llvm::BasicBlock::Create(Context, "and.rhs", Func);
                // auto* AndFalseBB= llvm::BasicBlock::Create(Context, "and.false", Func);
                //
                // Builder.CreateCondBr(Left->GetValue(), AndRhsBB, AndFalseBB);
                //
                // Builder.SetInsertPoint(AndRhsBB);
                // IRValue* Right = CompileNode(Logical->Right);
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
                // return Create<IRValue>(Phi, Logical->CompileTimeValue->Type);

                return Create<IRValue>(
                    Builder.CreateAnd(Left->GetValue(), Right->GetValue()), Logical->CompileTimeValue->Type.GetType());
            }
            default:
                ERROR("Unknown logical operator")
        }
    }

    IRValue *LLVMCompiler::CompileAssignment(const AssignmentNode *Assignment)
    {
        IRValue* LValue = CompileNode(Assignment->Left);

        if (!LValue || !LValue->IsLValue())
            ERROR("Cannot apply assignment operator to r-value")

        llvm::Value* Value = LValue->GetValue();

        DataType* Type = LValue->GetDataType();
        IRValue* Right = CompileNode(Assignment->Right)->GetRValue(
            Builder, CContext)->CastTo(Type, Builder, CContext); //ImplicitCast(CompileNode(Assignment->Right), Type);
        llvm::Value* RightVal = Right->GetValue();

        if (Assignment->Type == OperatorType::ASSIGN)
            return Create<IRValue>(Builder.CreateStore(RightVal, Value), Right->GetDataType());

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

        return Create<IRValue>(Builder.CreateStore(Left, Value), Type);
    }

    IRValue* LLVMCompiler::CompileBinary(const BinaryOpNode *BinaryOp)
    {
        IRValue* Left = CompileNode(BinaryOp->Left);
        Left = Left->GetRValue(Builder, CContext);
        IRValue* Right = CompileNode(BinaryOp->Right);
        Right = Right->GetRValue(Builder, CContext);

        Left = Left->CastTo(BinaryOp->LeftOperandType, Builder, CContext);
        Right = Right->CastTo(BinaryOp->RightOperandType, Builder, CContext);

        if (!Right || !Left)
            return nullptr;

        // if (!Left->CastTo(BinaryOp->LeftOperandType, Builder, CContext))
        //     return nullptr;
        //
        // if (!Right->CastTo(BinaryOp->RightOperandType, Builder, CContext))
        //     return nullptr;

        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        bool IsFP = Cast<FloatingPointType>(Type);
        // auto PtrType = Cast<PointerType>(Type);
        bool IsSigned = false;
        if (auto IntType = Cast<IntegerType>(Type))
            IsSigned = IntType->IsSigned;

        switch (BinaryOp->Type)
        {
            case OperatorType::ADD:     return Create<IRValue>(IsFP ?
                                    Builder.CreateFAdd(LeftVal, RightVal) :
                                    Builder.CreateAdd(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::SUB:     return Create<IRValue>(IsFP ?
                                    Builder.CreateFSub(LeftVal, RightVal) :
                                    Builder.CreateSub(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::MUL:     return Create<IRValue>(IsFP ?
                                    Builder.CreateFMul(LeftVal, RightVal) :
                                    Builder.CreateMul(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::DIV:     return Create<IRValue>(IsFP ?
                                    Builder.CreateFDiv(LeftVal, RightVal) : IsSigned ?
                                    Builder.CreateSDiv(LeftVal, RightVal) :
                                    Builder.CreateUDiv(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::MOD:     return Create<IRValue>(IsSigned ?
                                    Builder.CreateSRem(LeftVal, RightVal) :
                                    Builder.CreateURem(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::BIT_AND: return Create<IRValue>(
                                    Builder.CreateAnd(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::BIT_OR:  return Create<IRValue>(
                                    Builder.CreateOr(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::BIT_XOR: return Create<IRValue>(
                                    Builder.CreateXor(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::LSHIFT:  return  Create<IRValue>(
                                    Builder.CreateShl(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            case OperatorType::RSHIFT:  return Create<IRValue>(IsSigned ?
                                    Builder.CreateAShr(LeftVal, RightVal) :
                                    Builder.CreateLShr(LeftVal, RightVal),
                                    BinaryOp->CompileTimeValue->Type.GetType());
            default: ERROR("Unknown binary operator")
        }
    }

    IRValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        const auto& Args = Call->Arguments;

        SmallVec8<llvm::Value*> LLVMArgs;

        LLVMArgs.reserve(Args.size());

        for (const auto Arg : Args)
        {
            if (Arg->ExpectedType->IsReferenceType())
            {
                IRValue* ArgValue = CompileNode(Arg);
                if (!ArgValue || !ArgValue->IsLValue())
                    return nullptr;

                LLVMArgs.push_back(ArgValue->GetValue());
                continue;
            }

            IRValue* ArgValue = CompileNode(Arg);

            if (!ArgValue)
                return nullptr;

            ArgValue = ArgValue->GetRValue(Builder, CContext);

            // if (Arg->ExpectedType)
            //     if (!ArgValue->CastTo(Arg->ExpectedType, Builder, CContext))
            //         return nullptr;

            if (Arg->ExpectedType)
            {
                if (auto CastedValue = ArgValue->CastTo(Arg->ExpectedType, Builder, CContext))
                {
                    LLVMArgs.push_back(CastedValue->GetValue());
                    continue;
                }

                return nullptr;
            }

            return nullptr;

            // IRValue* ArgValue = CompileNode(Arg)->CastOrBind(Arg->ExpectedType, Builder, CContext);
            // if (!ArgValue)
            //     return nullptr;
            //
            // LLVMArgs.push_back(ArgValue->GetValue());
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

    IRValue *LLVMCompiler::CompileSubscript(const SubscriptNode *Subscript)
    {
        if (auto PtrType = Cast<PointerType>(Subscript->TargetType))
        {
            IRValue* Value = CompileNode(Subscript->Target);
            if (!Value)
                return nullptr;

            Value = Value->GetRValue(Builder, CContext);

            llvm::Value* LLVMValue = Value->GetValue();
            llvm::Type* ElType = CContext.GetLLVMType(PtrType->BaseType.GetType());
            IRValue* Index = CompileNode(Subscript->Index);
            Index = Index->GetRValue(Builder, CContext);
            llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
            // llvm::Value* El = Builder.CreateLoad(ElType, ElPtr);
            return Create<IRValue>(ElPtr, PtrType->BaseType.GetType(), true);
        }

        if (auto ArrType = Cast<ArrayType>(Subscript->TargetType))
        {
            IRValue* Value = CompileNode(Subscript->Target);

            if (!Value || !Value->IsLValue())
                return nullptr;

            llvm::Value* LLVMValue = Value->GetValue();
            // llvm::Type* ElType = CContext.GetLLVMType(ArrType->BaseType.GetType());
            IRValue* Index = CompileNode(Subscript->Index);
            Index = Index->GetRValue(Builder, CContext);
            llvm::Value* ElPtr = Builder.CreateGEP(CContext.GetLLVMType(ArrType), LLVMValue,
                { Builder.getInt32(0), Index->GetValue() });
            // llvm::Value* El = Builder.CreateLoad(ElType, ElPtr);
            return Create<IRValue>(ElPtr, ArrType->BaseType.GetType(), true);
        }

        return nullptr;
    }

    IRValue* LLVMCompiler::CompileExplicitCast(const ExplicitCastNode *ExplicitCast)
    {
        DataType* DstType = ExplicitCast->CompileTimeValue->Type.GetType();
        IRValue* Target = CompileNode(ExplicitCast->Target);
        Target = Target->GetRValue(Builder, CContext);

        if (IRValue* Value = Target->CastTo(DstType, Builder, CContext))//ImplicitCast(Target, DstType))
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

            // Builder.CreateStore(Value->GetValue(), Alloca);
            DeclareVariable(Var->Name.str(),
                /* Create<IRValue>(Alloca, VarType, true)*/ Value);
            return nullptr;
        }

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
            IRValue* Value = CompileNode(Var->Value);
            Value = Value->GetRValue(Builder, CContext);
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

    IRValue* LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            IRValue* RetVal = CompileNode(Return->ReturnValue);
            RetVal = RetVal->GetRValue(Builder, CContext);
            Builder.CreateRet(RetVal->GetValue());
            return nullptr;
        }

        Builder.CreateRetVoid();
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileIf(const IfNode *If)
    {
        IRValue* Cond = CompileNode(If->Condition);
        Cond = Cond->GetRValue(Builder, CContext);
        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond)
            return nullptr;

        // if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
        //     return nullptr;

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
        IRValue* Cond = CompileNode(While->Condition);
        Cond = Cond->GetRValue(Builder, CContext);
        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond)
            return nullptr;
        // if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
        //     return nullptr;

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
        IRValue* Cond = CompileNode(For->Condition);
        Cond = Cond->GetRValue(Builder, CContext);
        Cond = Cond->CastTo(CContext.GetBoolType(), Builder, CContext);
        if (!Cond)
            return nullptr;

        // if (!Cond->CastTo(CContext.GetBoolType(), Builder, CContext))
        //     return nullptr;

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
            ERROR("'break' used outside loop")

        Builder.CreateBr(LoopEndStack.top());
        return nullptr;
    }

    IRValue *LLVMCompiler::CompileContinue()
    {
        if (LoopHeaderStack.empty())
            ERROR("'continue' used outside loop")

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
            ERROR("This variable: '" + Name + "' has already declared in this scope");

        ScopeEntry Entry;
        Entry.Name = Name;

        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            Entry.Previous = Iter->second;

        ScopeStack.Back().Add(Entry);
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

    // IRValue *LLVMCompiler::GetLValue(const ASTNode *Node)
    // {
    //     if (const auto Identifier = Cast<const IdentifierNode>(Node))
    //         return ResolveReference(GetVariable(Identifier->Value.str()));
    //
    //     if (const auto Subscript = Cast<const SubscriptNode>(Node))
    //     {
    //         if (auto PtrType = Cast<PointerType>(Subscript->TargetType))
    //         {
    //             IRValue* Value = CompileNode(Subscript->Target);
    //
    //             if (!Value)
    //                 return nullptr;
    //
    //             llvm::Value* LLVMValue = Value->GetValue();
    //             llvm::Type* ElType = CContext.GetLLVMType(PtrType->BaseType.GetType());
    //             IRValue* Index = CompileNode(Subscript->Index);
    //             llvm::Value* ElPtr = Builder.CreateGEP(ElType, LLVMValue, Index->GetValue());
    //             return Create<IRValue>(ElPtr, PtrType->BaseType.GetType());
    //         }
    //
    //         if (auto ArrType = Cast<ArrayType>(Subscript->TargetType))
    //         {
    //             IRValue* Value = GetLValue(Subscript->Target);
    //
    //             if (!Value)
    //                 return nullptr;
    //
    //             llvm::Value* LLVMValue = Value->GetValue();
    //             llvm::Type* ElType = CContext.GetLLVMType(ArrType->BaseType.GetType());
    //             IRValue* Index = CompileNode(Subscript->Index);
    //             llvm::Value* ElPtr = Builder.CreateGEP(CContext.GetLLVMType(ArrType), LLVMValue,
    //                 { Builder.getInt32(0), Index->GetValue() });
    //             return Create<IRValue>(ElPtr, ArrType->BaseType.GetType());
    //         }
    //
    //         return nullptr;
    //     }
    //
    //     if (auto Unref = Cast<const UnrefNode>(Node))
    //     {
    //         IRValue *TValue = CompileNode(Unref->Target);
    //         if (!TValue)
    //             return nullptr;
    //
    //         return Create<IRValue>(TValue->GetValue(), Unref->CompileTimeValue->Type.GetType());
    //     }
    //
    //     return nullptr;
    // }

    // IRValue *LLVMCompiler::ResolveReference(IRValue *Value)
    // {
    //     if (auto RefType = Cast<ReferenceType>(Value->GetDataType()))
    //     {
    //         llvm::Type* LLVMType = CContext.GetLLVMType(RefType);
    //         llvm::Value* LoadValue = Builder.CreateLoad(LLVMType, Value->GetValue());
    //         return Create<IRValue>(LoadValue, RefType->BaseType.GetType());
    //     }
    //
    //     return Value;
    // }

    IRValue *LLVMCompiler::ImplicitCast(IRValue *Value, DataType* Target)
    {
        if (Value->CastTo(Target, Builder, CContext))
            return Value;

        return nullptr;
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
            IRValue* El = CompileNode(Array->Elements[i]);

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Alloca, Idx);
            Builder.CreateStore(El->GetValue(), ElPtr);
        }
    }
}
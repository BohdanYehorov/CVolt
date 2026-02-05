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

    int LLVMCompiler::Run()
    {
        static bool JITInit = false;
        if (!JITInit)
        {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();
            JITInit = true;
        }

        auto NewContext = std::make_unique<llvm::LLVMContext>();

        auto JIT = llvm::orc::LLJITBuilder().create();
        if (!JIT)
            return 1;

        llvm::orc::SymbolMap Symbols;

        BuiltinFuncTable.GenSymbolMap(JIT->get(), Symbols);

        cantFail(JIT->get()->getMainJITDylib().define(
            llvm::orc::absoluteSymbols(Symbols)
        ));

        llvm::orc::ThreadSafeModule TSM{std::move(Module), std::move(NewContext)};

        if (auto Err = JIT->get()->addIRModule(std::move(TSM)))
            return 1;
        auto MainSymOrErr = JIT->get()->lookup("Main");
        if (!MainSymOrErr)
        {
            llvm::logAllUnhandledErrors(MainSymOrErr.takeError(), llvm::errs(), "Error: ");
            return 1;
        }

        const auto MainFunc = reinterpret_cast<int(*)()>(MainSymOrErr->getValue());
        return MainFunc();
    }

    TypedValue *LLVMCompiler::CompileNode(ASTNode *Node)
    {
        if (Node->CompileTimeValue && Node->CompileTimeValue->IsValid)
        {
            CTimeValue* Value = Node->CompileTimeValue;
            switch (DataTypeUtils::GetTypeCategory(Value->Type))
            {
                case TypeCategory::INTEGER:
                    return Create<TypedValue>(llvm::ConstantInt::get(
                    CompilerBuilder.GetLLVMType(Value->Type), Value->Int), Value->Type);
                case TypeCategory::FLOATING_POINT:
                    return Create<TypedValue>(llvm::ConstantFP::get(
                        CompilerBuilder.GetLLVMType(Value->Type), Value->Float), Value->Type);
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
            CompilerBuilder.GetLLVMType(Int->ResolvedType), Int->Value), Int->ResolvedType);
    }

    TypedValue *LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        return Create<TypedValue>(llvm::ConstantFP::get(
            CompilerBuilder.GetLLVMType(Float->ResolvedType), Float->Value), Float->ResolvedType);
    }

    TypedValue *LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
            CompilerBuilder.GetBoolType());
    }

    TypedValue *LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value), CompilerBuilder.GetCharType());
    }

    TypedValue *LLVMCompiler::CompileString(const StringNode *String)
    {
        return Create<TypedValue>(Builder.CreateGlobalString(String->Value.ToString()),
            CompilerBuilder.GetPointerType(CompilerBuilder.GetCharType()));
    }

    TypedValue *LLVMCompiler::CompileArray(const ArrayNode *Array)
    {
        if (Array->Elements.empty())
            ERROR("Array empty")

        llvm::Type* ArrType = nullptr;

        if (auto Type = Cast<ArrayType>(Array->ResolvedType))
            ArrType = llvm::ArrayType::get(
                CompilerBuilder.GetLLVMType(Type->BaseType), Array->Elements.size());

        llvm::AllocaInst* Arr = Builder.CreateAlloca(ArrType);
        llvm::Value* FirstElPtr = nullptr;

        llvm::Value* Idx[2] = {
            Builder.getInt32(0),
            nullptr
        };

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            TypedValue* El = CompileNode(Array->Elements[i]);

            Idx[1] = Builder.getInt32(i);

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            if (!FirstElPtr)
                FirstElPtr = ElPtr;

            Builder.CreateStore(El->GetValue(), ElPtr);
        }

        return Create<TypedValue>(Arr, Array->ResolvedType);
    }

    TypedValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.ToString();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        {
            TypedValue* Var = Iter->second;
            return Create<TypedValue>(Builder.CreateLoad(CompilerBuilder.GetLLVMType(Var->GetDataType()),
                        Var->GetValue(), Value + "_val"), Var->GetDataType());
        }

        ERROR("Cannot resolve symbol: '" + Value + "'")
    }

    TypedValue *LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        TypedValue* LValue = GetLValue(Ref->Target);
        if (!LValue)
            ERROR("Cannot apply operator '$' to l-value")

       return Create<TypedValue>(LValue->GetValue(), CompilerBuilder.GetPointerType(LValue->GetDataType()));
    }

    TypedValue *LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        TypedValue* LValue = GetLValue(Prefix->Operand);
        if (!LValue)
            ERROR("Cannot apply prefix operator to r-value")

        llvm::Value* Value = LValue->GetValue();
        Value = Builder.CreateLoad(CompilerBuilder.GetLLVMType(LValue->GetDataType()), Value);
        switch (Prefix->Type)
        {
            case Operator::INC:
                Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            case Operator::DEC:
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
        Value = Builder.CreateLoad(CompilerBuilder.GetLLVMType(LValue->GetDataType()), Value);
        llvm::Value* Temp = Value;
        switch (Suffix->Type)
        {
            case Operator::INC:
                Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
                break;
            case Operator::DEC:
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

        DataType* BoolType = CompilerBuilder.GetBoolType();

        bool IsFP = Cast<FloatingPointType>(Type);

        switch (Unary->Type)
        {
            case Operator::ADD:         return TValue;
            case Operator::SUB:         return Create<TypedValue>(IsFP ?
                                        Builder.CreateNeg(Value) :
                                        Builder.CreateFNeg(Value), Unary->ResolvedType);
            case Operator::LOGICAL_NOT: return Create<TypedValue>(Builder.CreateNot(
                                               ImplicitCast(TValue, BoolType)->GetValue()), Unary->ResolvedType);
            case Operator::BIT_NOT:     return Create<TypedValue>(Builder.CreateNot(Value), Unary->ResolvedType);
            default: ERROR("Unknown unary operator")
        }
    }

    TypedValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        TypedValue* Left = CompileNode(Comparison->Left);
        TypedValue* Right = CompileNode(Comparison->Right);

        Left = ImplicitCast(Left, Comparison->OperandsType);
        Right = ImplicitCast(Right, Comparison->OperandsType);

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
            case Operator::EQ:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOEQ(LeftVal, RightVal) :
                                Builder.CreateICmpEQ(LeftVal, RightVal), Comparison->ResolvedType);
            case Operator::NEQ: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpONE(LeftVal, RightVal) :
                                Builder.CreateICmpNE(LeftVal, RightVal), Comparison->ResolvedType);
            case Operator::LT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOLT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLT(LeftVal, RightVal) :
                                Builder.CreateICmpULT(LeftVal, RightVal), Comparison->ResolvedType);
            case Operator::LTE: return Create<TypedValue>( IsFP ?
                                Builder.CreateFCmpOLE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLE(LeftVal, RightVal) :
                                Builder.CreateICmpULE(LeftVal, RightVal), Comparison->ResolvedType);
            case Operator::GT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGT(LeftVal, RightVal) :
                                Builder.CreateICmpUGT(LeftVal, RightVal), Comparison->ResolvedType);
            case Operator::GTE: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGE(LeftVal, RightVal) :
                                Builder.CreateICmpUGE(LeftVal, RightVal), Comparison->ResolvedType);
            default: ERROR("Unknown comparison operator")
        }
    }

    TypedValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        TypedValue* Left = CompileNode(Logical->Left);
        Left = ImplicitCast(Left, Logical->OperandsType);

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();

        switch (Logical->Type)
        {
            case Operator::LOGICAL_OR:
            {
                llvm::BasicBlock* OrRhsBB = llvm::BasicBlock::Create(Context, "or.rhs", Func);
                llvm::BasicBlock* OrTrueBB = llvm::BasicBlock::Create(Context, "or.true", Func);
                llvm::BasicBlock* OrEndBB = llvm::BasicBlock::Create(Context, "or.end", Func);

                Builder.CreateCondBr(Left->GetValue(), OrTrueBB, OrRhsBB);

                Builder.SetInsertPoint(OrRhsBB);
                TypedValue* Right = CompileNode(Logical->Right);
                Right = ImplicitCast(Right, Logical->OperandsType);
                Builder.CreateCondBr(Right->GetValue(), OrTrueBB, OrEndBB);

                Builder.SetInsertPoint(OrTrueBB);
                Builder.CreateBr(OrEndBB);

                Builder.CreateBr(OrEndBB);

                Builder.SetInsertPoint(OrEndBB);
                llvm::PHINode* Phi = Builder.CreatePHI(llvm::Type::getInt1Ty(Context), 2);
                Phi->addIncoming(Builder.getTrue(), OrTrueBB);
                Phi->addIncoming(Builder.getFalse(), OrRhsBB);

                return Create<TypedValue>(
                      Phi, Logical->ResolvedType);
            }
            case Operator::LOGICAL_AND:
            {
                auto* AndRhsBB   = llvm::BasicBlock::Create(Context, "and.rhs", Func);
                auto* AndFalseBB= llvm::BasicBlock::Create(Context, "and.false", Func);
                auto* AndEndBB  = llvm::BasicBlock::Create(Context, "and.end", Func);

                Builder.CreateCondBr(Left->GetValue(), AndRhsBB, AndFalseBB);

                Builder.SetInsertPoint(AndRhsBB);
                TypedValue* Right = CompileNode(Logical->Right);
                Right = ImplicitCast(Right, Logical->OperandsType);
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndFalseBB);
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndEndBB);
                llvm::PHINode* Phi =
                    Builder.CreatePHI(Builder.getInt1Ty(), 2);

                Phi->addIncoming(Right->GetValue(), AndRhsBB);
                Phi->addIncoming(Builder.getFalse(), AndFalseBB);

                return Create<TypedValue>(
                      Phi, Logical->ResolvedType);
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

        if (Assignment->Type == Operator::ASSIGN)
            return Create<TypedValue>(Builder.CreateStore(RightVal, Value), Right->GetDataType());

        llvm::Value* Left = Builder.CreateLoad(CompilerBuilder.GetLLVMType(Type), Value);

        bool IsFP = Cast<FloatingPointType>(Type);

        bool IsSigned = false;
        if (!IsFP)
            if (auto IntType = Cast<IntegerType>(Type))
                IsSigned = IntType->IsSigned;

        switch (Assignment->Type)
        {
            case Operator::ADD_ASSIGN:
                Left = IsFP ? Builder.CreateFAdd(Left, RightVal) :
                              Builder.CreateAdd(Left, RightVal);
                break;
            case Operator::SUB_ASSIGN:
                Left = IsFP ? Builder.CreateFSub(Left, RightVal) :
                              Builder.CreateSub(Left, RightVal);
                break;
            case Operator::MUL_ASSIGN:
                Left = IsFP ? Builder.CreateFMul(Left, RightVal) :
                              Builder.CreateMul(Left, RightVal);
                break;
            case Operator::DIV_ASSIGN:
                Left = IsFP     ? Builder.CreateFDiv(Left, RightVal) :
                       IsSigned ? Builder.CreateSDiv(Left, RightVal) :
                                  Builder.CreateUDiv(Left, RightVal);
                break;
            case Operator::MOD_ASSIGN:
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

        Left = ImplicitCast(Left, BinaryOp->OperandsType);
        Right = ImplicitCast(Right, BinaryOp->OperandsType);

        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        bool IsFP = Cast<FloatingPointType>(Type);
        bool IsSigned = false;
        if (auto IntType = Cast<IntegerType>(Type))
            IsSigned = IntType->IsSigned;

        switch (BinaryOp->Type)
        {
            case Operator::ADD:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFAdd(LeftVal, RightVal) :
                                    Builder.CreateAdd(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::SUB:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFSub(LeftVal, RightVal) :
                                    Builder.CreateSub(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::MUL:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFMul(LeftVal, RightVal) :
                                    Builder.CreateMul(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::DIV:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFDiv(LeftVal, RightVal) : IsSigned ?
                                    Builder.CreateSDiv(LeftVal, RightVal) :
                                    Builder.CreateUDiv(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::MOD:     return Create<TypedValue>(IsSigned ?
                                    Builder.CreateSRem(LeftVal, RightVal) :
                                    Builder.CreateURem(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::BIT_AND: return Create<TypedValue>(
                                    Builder.CreateAnd(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::BIT_OR:  return Create<TypedValue>(
                                    Builder.CreateOr(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::BIT_XOR: return Create<TypedValue>(
                                    Builder.CreateXor(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::LSHIFT:  return  Create<TypedValue>(
                                    Builder.CreateShl(LeftVal, RightVal), BinaryOp->ResolvedType);
            case Operator::RSHIFT:  return Create<TypedValue>(IsSigned ?
                                    Builder.CreateAShr(LeftVal, RightVal) :
                                    Builder.CreateLShr(LeftVal, RightVal), BinaryOp->ResolvedType);
            default: ERROR("Unknown binary operator")
        }
    }

    TypedValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& FuncName = Identifier->Value.ToString();

            llvm::SmallVector<llvm::Value*, 8> LLVMArgs;
            llvm::SmallVector<DataType*, 8> ArgTypes;
            llvm::SmallVector<TypedValue*, 8> ArgValues;

            const auto& Args = Call->Arguments;
            LLVMArgs.reserve(Args.size());
            ArgTypes.reserve(Args.size());
            ArgValues.reserve(Args.size());

            for (const auto Arg : Args)
            {
                TypedValue* ArgValue = CompileNode(Arg);

                LLVMArgs.push_back(ArgValue->GetValue());
                ArgTypes.push_back(ArgValue->GetDataType());
                ArgValues.push_back(ArgValue);
            }

            FunctionSignature Signature{ FuncName, ArgTypes };

            if (auto Iter = FunctionSignatures.find(Signature); Iter != FunctionSignatures.end())
                return Create<TypedValue>(Builder.CreateCall(
                            Iter->second->GetFunction(), LLVMArgs), Iter->second->GetReturnType());

            if (auto FuncData = BuiltinFuncTable.Get(Signature))
            {
                llvm::Function* OutFunc = Module->getFunction(FuncData->BaseName);
                return Create<TypedValue>(Builder.CreateCall(OutFunc,LLVMArgs),
                FuncData->ReturnType );
            }

            // int MinDiffRank = std::numeric_limits<int>::max();
            // TypedFunction* Function = nullptr;
            // const FunctionSignature* BestFunctionSignature = nullptr;
            // for (const auto& [FuncSignature, Func] : FunctionSignatures)
            // {
            //     if (FuncSignature.Name != Signature.Name)
            //         continue;
            //
            //     if (FuncSignature.Params.size() != Signature.Params.size())
            //         continue;
            //
            //     int DiffRank = 0;
            //     bool HasNonImplicitCastTypes = false;
            //     for (size_t i = 0; i < FuncSignature.Params.size(); i++)
            //     {
            //         DataType* ParamType1 = FuncSignature.Params[i];
            //         DataType* ParamType2 = Signature.Params[i];
            //         if (!CanImplicitCast(ParamType1, ParamType2))
            //         {
            //             HasNonImplicitCastTypes = true;
            //             break;
            //         }
            //
            //         int Rank1 = DataType::GetPrimitiveTypeRank(Cast<PrimitiveDataType>(ParamType1)); //ParamType1.GetPrimitiveTypeRank();
            //         int Rank2 = DataType::GetPrimitiveTypeRank(Cast<PrimitiveDataType>(ParamType1));
            //
            //         if (Rank1 == -1 || Rank2 == -1)
            //         {
            //             HasNonImplicitCastTypes = true;
            //             break;
            //         }
            //
            //         DiffRank += std::abs(Rank1 - Rank2);
            //     }
            //
            //     if (HasNonImplicitCastTypes)
            //         continue;
            //
            //     if (MinDiffRank > DiffRank)
            //     {
            //         Function = Func;
            //         MinDiffRank = DiffRank;
            //         BestFunctionSignature = &FuncSignature;
            //     }
            // }

            // if (BestFunctionSignature && Function)
            // {
            //     for (size_t i = 0; i < BestFunctionSignature->Params.size(); i++)
            //     {
            //         DataType* ParamType = BestFunctionSignature->Params[i];
            //
            //         TypedValue* Value = ArgValues[i];
            //         Value = ImplicitCast(Value, ParamType);
            //
            //         LLVMArgs[i] = Value->GetValue();
            //     }
            //
            //     return Create<TypedValue>(Builder.CreateCall(
            //                Function->GetFunction(), LLVMArgs), Function->GetReturnType());
            // }

            ERROR("Function '" + FuncName + "' not found");
        }

        ERROR("Called object is not a function")
    }

    TypedValue *LLVMCompiler::CompileSubscript(const SubscriptNode *Subscript)
    {
        TypedValue* Ptr = GetLValue(Subscript->Target);
        llvm::Value* Value = Ptr->GetValue();
        auto Type = Cast<ArrayType>(Ptr->GetDataType());

        auto Alloca = llvm::cast<llvm::AllocaInst>(Value);

        TypedValue* Index = CompileNode(Subscript->Index);
        llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Value,
            { Builder.getInt32(0), Index->GetValue() });
        llvm::Value* El = Builder.CreateLoad(Alloca->getAllocatedType()->getArrayElementType(), ElPtr);

        Int64 IndexInt;
        if (GetIntegerValue(Subscript->Index, IndexInt))
            if (IndexInt >= Type->Length || IndexInt < 0)
                ERROR("Index out of array range");

        // PointerType* ElPtrType = Ptr->GetDataType().GetPtrType();
        // if (!ElPtrType)
        //     ERROR("Cannot apply subscript to non-pointer type");
        //
        // llvm::AllocaInst* Inst = llvm::cast<llvm::AllocaInst>(Ptr->GetValue());
        // llvm::Type* ElType = DataType::GetLLVMType(ElPtrType->BaseType, Context);
        // TypedValue* Index = CompileNode(Subscript->Index);
        // Index = ImplicitCast(Index, Subscript->Index->ResolvedType);
        //
        // //std::cout << Ptr->GetDataType().ToString() << std::endl;
        //
        // llvm::Value* ElPtr = Builder.CreateGEP(Inst->getAllocatedType(), Inst, { Builder.getInt32(0), Index->GetValue()});
        // llvm::Value* El = Builder.CreateLoad(ElPtr->getType(), getLoadStorePointerOperand(ElPtr));
        // return Create<TypedValue>(El, ElPtrType->BaseType);
        return Create<TypedValue>(El, Type->BaseType);
    }

    TypedValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        DataType* VarType = Var->Type->ResolvedType;

        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::Type* Type = DataTypeUtils::GetLLVMType(VarType, Context);

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
            Value = ImplicitCast(Value, VarType);
            Builder.CreateStore(Value->GetValue(), Alloca);
        }

        DeclareVariable(Var->Name.ToString(),
            Create<TypedValue>(Alloca, VarType, true));

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        llvm::SmallVector<llvm::Type*, 8> Params;
        Params.reserve(Function->Params.size());

        for (const auto Param : Function->Params)
        {
            DataType* ParamType = Param->Type->ResolvedType; //DataType::CreateFromAST(Param->Type, CompilerArena);
            Params.push_back(DataTypeUtils::GetLLVMType(ParamType, Context));
        }

        llvm::Type* RetType = DataTypeUtils::GetLLVMType(Function->ReturnType->ResolvedType
            /*DataType::CreateFromAST(Function->ReturnType, CompilerArena)*/, Context);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        const std::string& FuncName = Function->Name.ToString();
        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());

        llvm::SmallVector<DataType*, 8> ParamsTypes;

        const auto& FuncParams = Function->Params;
        ParamsTypes.reserve(FuncParams.size());
        for (size_t i = 0; i < FuncParams.size(); i++)
        {
            DataType* ParamType = FuncParams[i]->Type->ResolvedType; //DataType::CreateFromAST(FuncParams[i]->Type, CompilerArena);
            auto Arg = Func->args().begin() + i;
            Arg->setName(FuncParams[i]->Name.ToString());
            ParamsTypes.push_back(ParamType);
        }

        CurrentFunction = Func;
        FunctionParams = ParamsTypes;

        FunctionSignature Signature{ FuncName, ParamsTypes };
        //FunctionSignatures[Signature] = Create<TypedFunction>(Func, Function->ReturnType->Type);

        if (auto Iter = FunctionSignatures.find(Signature); Iter != FunctionSignatures.end())
            Iter->second->InitFunction(Func);
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
        Cond = ImplicitCast(Cond,CompilerBuilder.GetBoolType());

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
        Cond = ImplicitCast(Cond, CompilerBuilder.GetBoolType());

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
        Cond = ImplicitCast(Cond, CompilerBuilder.GetBoolType());

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
        if (auto Iter = std::find_if(ScopeStack.back().begin(), ScopeStack.back().end(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.back().end())
            ERROR("This variable: '" + Name + "' has already declared in this scope");

        ScopeEntry Entry;
        Entry.Name = Name;

        if (auto Iter = SymbolTable.find(Name); Iter != SymbolTable.end())
            Entry.Previous = Iter->second;

        ScopeStack.back().push_back(Entry);
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
        ScopeStack.emplace_back();
    }

    void LLVMCompiler::ExitScope()
    {
        for (const auto& Entry : ScopeStack.back())
        {
            if (Entry.Previous)
                SymbolTable[Entry.Name] = Entry.Previous;
            else
                SymbolTable.erase(Entry.Name);
        }

        ScopeStack.pop_back();
    }

    TypedValue *LLVMCompiler::GetLValue(const ASTNode *Node)
    {
        if (const auto Identifier = Cast<const IdentifierNode>(Node))
            return GetVariable(Identifier->Value.ToString());

        if (const auto Subscript = Cast<const SubscriptNode>(Node))
        {
            // TypedValue* Ptr = CompileNode(Subscript->Target);
            // PointerType* ElPtrType = Ptr->GetDataType().GetPtrType();
            // if (!ElPtrType)
            //     ERROR("Cannot apply subscript to non-pointer type");
            //
            // llvm::Type* ElType = DataType::GetLLVMType(ElPtrType->BaseType, Context);
            // TypedValue* Index = CompileNode(Subscript->Index);
            //
            // llvm::Value* ElPtr = Builder.CreateGEP( ElType, Ptr->GetValue(), Index->GetValue());

            TypedValue* Ptr = GetLValue(Subscript->Target);
            llvm::Value* Value = Ptr->GetValue();

            auto Type = Cast<ArrayType>(Ptr->GetDataType());

            auto Alloca = llvm::cast<llvm::AllocaInst>(Value);

            Int64 IndexInt;
            if (GetIntegerValue(Subscript->Index, IndexInt))
                if (IndexInt >= Type->Length || IndexInt < 0)
                    ERROR("Index out of array range");

            TypedValue* Index = CompileNode(Subscript->Index);
            llvm::Value* ElPtr = Builder.CreateGEP(Alloca->getAllocatedType(), Value,
                { Builder.getInt32(0), Index->GetValue() });

            return Create<TypedValue>(ElPtr, Type->BaseType, true);
        }

        return nullptr;
    }

    TypedValue *LLVMCompiler::ImplicitCast(TypedValue *Value, DataType* Target)
    {
        DataType* SrcType = Value->GetDataType();
        llvm::Type* TargetLLVMType = CompilerBuilder.GetLLVMType(Target);
        llvm::Type* SrcLLVMType = CompilerBuilder.GetLLVMType(SrcType);

        if (DataTypeUtils::IsEqual(SrcType, Target))
            return Value;

        if (Cast<BoolType>(SrcType))
        {
            if (Cast<IntegerType>(Target))
                return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                    TargetLLVMType), Target);

            if (Cast<FloatingPointType>(Target))
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                    TargetLLVMType), Target);

            if (Cast<PointerType>(Target))
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantPointerNull::get(
                            llvm::cast<llvm::PointerType>(SrcLLVMType))), Target);
        }

        if (auto SrcIntType = Cast<IntegerType>(SrcType))
        {
            if (Cast<BoolType>(Target))
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantInt::get(SrcLLVMType, 0)), Target);

            if (auto TargetIntType = Cast<IntegerType>(Target))
            {
                if (SrcIntType->BitWidth < TargetIntType->BitWidth)
                    return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                        TargetLLVMType), Target);

                return Create<TypedValue>(Builder.CreateTrunc(Value->GetValue(),
                    TargetLLVMType), Target);
            }

            if (Cast<FloatingPointType>(Target))
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                    TargetLLVMType), Target);
        }

        if (auto SrcFloatType = Cast<FloatingPointType>(SrcType))
        {
            if (Cast<BoolType>(Target))
                return Create<TypedValue>(Builder.CreateFCmpONE(Value->GetValue(),
                            llvm::ConstantFP::get(SrcLLVMType, 0.0 )), Target);

            if (Cast<IntegerType>(Target))
                return Create<TypedValue>(Builder.CreateFPToSI(Value->GetValue(),
                    TargetLLVMType), Target);

            if (auto TargetFloatType = Cast<FloatingPointType>(Target))
            {
                if (SrcFloatType->BitWidth < TargetFloatType->BitWidth)
                    return Create<TypedValue>(Builder.CreateFPExt(Value->GetValue(),
                        TargetLLVMType), Target);

                return Create<TypedValue>(Builder.CreateFPTrunc(Value->GetValue(),
                    TargetLLVMType), Target);
            }
        }

        ERROR(std::format("Cannot convert '{}' to '{}'",
            DataTypeUtils::TypeToString(SrcType), DataTypeUtils::TypeToString(Target)))
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

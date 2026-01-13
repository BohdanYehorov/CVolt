//
// Created by bohdan on 03.01.26.
//

#include "Volt/Compiler/LLVMCompiler.h"
#include "Volt/Compiler/Functions/DefaultFunctions.h"
#include <llvm/Support/TargetSelect.h>

#define ERROR(Message) throw CompilerError(Message);

namespace Volt
{
    void LLVMCompiler::Compile()
    {
        CreateDefaultFunction("Out", "OutBool", &OutBool);
        CreateDefaultFunction("Out", "OutChar", &OutChar);
        CreateDefaultFunction("Out", "OutByte", &OutByte);
        CreateDefaultFunction("Out", "OutInt", &OutInt);
        CreateDefaultFunction("Out", "OutLong", &OutLong);
        CreateDefaultFunction("Out", "OutStr", &OutStr);
        CreateDefaultFunction("Out", "OutFloat", &OutFloat);
        CreateDefaultFunction("Out", "OutDouble", &OutDouble);
        CreateDefaultFunction("Time", "Time", &Time);
        CreateDefaultFunction("Sin", "Sin", &Sin);
        CreateDefaultFunction("Cos", "Cos", &Cos);
        CreateDefaultFunction("Tan", "Tan", &Tan);

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

        for (const auto& [Name, ExeSymbolDef] : DefaultSymbols)
            Symbols[JIT->get()->mangleAndIntern(Name)] = ExeSymbolDef;

        cantFail(JIT->get()->getMainJITDylib().define(
            llvm::orc::absoluteSymbols(Symbols)
        ));

        llvm::orc::ThreadSafeModule TSM{std::move(Module), std::move(NewContext)};

        if (auto Err = JIT->get()->addIRModule(std::move(TSM)))
            return 1;
        auto MainSymOrErr = JIT->get()->lookup("Main");
        if (!MainSymOrErr) {
            llvm::logAllUnhandledErrors(MainSymOrErr.takeError(), llvm::errs(), "Error: ");
            return 1;
        }

        const auto MainFunc = reinterpret_cast<int(*)()>(MainSymOrErr->getValue());
        return MainFunc();
    }

    TypedValue *LLVMCompiler::CompileNode(ASTNode *Node)
    {
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
                    Create<TypedValue>(Alloca, DataType::Create(FunctionParams[i], CompilerArena)));
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
        switch (Int->Type)
        {
            case IntegerNode::BYTE:
                return Create<TypedValue>(
                    llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Int->Value),
                    DataType::CreateInteger(8, CompilerArena));
            case IntegerNode::INT:
                return Create<TypedValue>(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Int->Value),
                    DataType::CreateInteger(32, CompilerArena));
            case IntegerNode::LONG:
                return Create<TypedValue>(
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), Int->Value),
                    DataType::CreateInteger(64, CompilerArena));
            default:
                ERROR("Unknown integer type")
        }
    }

    TypedValue *LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        switch (Float->Type)
        {
            case FloatingPointNode::DOUBLE:
                return Create<TypedValue>(llvm::ConstantFP::get(
                    llvm::Type::getDoubleTy(Context), Float->Value),
                    DataType::CreateFloatingPoint(64, CompilerArena));
            case FloatingPointNode::FLOAT:
                return Create<TypedValue>(
                    llvm::ConstantFP::get(llvm::Type::getFloatTy(Context), Float->Value),
                    DataType::CreateFloatingPoint(32, CompilerArena));
            default:
                ERROR("Unknown float type")
        }
    }

    TypedValue *LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
            DataType::CreateBoolean(CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value),
            DataType::CreateChar(CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileString(const StringNode *String)
    {
        return Create<TypedValue>(Builder.CreateGlobalString(String->Value.ToString()),
            DataType::CreatePtr(DataType::CreateChar(CompilerArena)->GetTypeBase(), CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileArray(const ArrayNode *Array)
    {
        if (Array->Elements.empty())
            ERROR("Array empty")

        TypedValue* FirstEl = nullptr;
        llvm::ArrayType* ArrType = nullptr;
        llvm::AllocaInst* Arr = nullptr;
        llvm::Value* FirstElPtr = nullptr;

        for (size_t i = 0; i < Array->Elements.size(); i++)
        {
            TypedValue* El = CompileNode(Array->Elements[i]);
            if (!FirstEl)
            {
                FirstEl = El;
                ArrType = llvm::ArrayType::get(
                    FirstEl->GetDataType()->GetLLVMType(Context), Array->Elements.size());
                Arr = Builder.CreateAlloca(ArrType);
            }
            else if (!El->GetDataType()->IsEqual(FirstEl->GetDataType()))
                ERROR("Cannot init array with different types")

            llvm::Value* Idx[2] = {
                Builder.getInt32(0),
                Builder.getInt32(i)
            };

            llvm::Value* ElPtr = Builder.CreateGEP(Arr->getAllocatedType(), Arr, Idx);
            if (!FirstElPtr)
                FirstElPtr = ElPtr;

            Builder.CreateStore(El->GetValue(), ElPtr);
        }

        return Create<TypedValue>(FirstElPtr, DataType::CreatePtr(
            FirstEl->GetDataType()->GetTypeBase(), CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.ToString();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        {
            TypedValue* Var = Iter->second;
            return Create<TypedValue>(Builder.CreateLoad(Var->GetDataType()->GetLLVMType(Context),
                        Var->GetValue(), Value + "_val"), Var->GetDataType());
        }

        ERROR("Cannot resolve symbol: '" + Value + "'")
    }

    TypedValue *LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        TypedValue* LValue = GetLValue(Ref->Target);
        if (!LValue)
            ERROR("Cannot apply operator '$' to l-value")

       return LValue;
    }

    TypedValue *LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        TypedValue* LValue = GetLValue(Prefix->Operand);
        if (!LValue)
            ERROR("Cannot apply prefix operator to r-value")

        llvm::Value* Value = LValue->GetValue();
        Value = Builder.CreateLoad(LValue->GetDataType()->GetLLVMType(Context), Value);
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
        Value = Builder.CreateLoad(LValue->GetDataType()->GetLLVMType(Context), Value);
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

        DataType* BoolType = DataType::CreateBoolean(CompilerArena);

        bool IsFP = Type->GetFloatingPointType();

        switch (Unary->Type)
        {
            case Operator::ADD:         return TValue;
            case Operator::SUB:         return Create<TypedValue>(IsFP ?
                                        Builder.CreateNeg(Value) :
                                        Builder.CreateFNeg(Value), Type);
            case Operator::LOGICAL_NOT: return Create<TypedValue>(Builder.CreateNot(
                                               ImplicitCast(TValue, BoolType)->GetValue()), BoolType);
            case Operator::BIT_NOT:     return Create<TypedValue>(Builder.CreateNot(Value), Type);
            default: ERROR("Unknown unary operator")
        }
    }

    TypedValue *LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        TypedValue* Left = CompileNode(Comparison->Left);
        TypedValue* Right = CompileNode(Comparison->Right);

        CastToJointType(Left, Right);
        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        DataType* BoolType = DataType::CreateBoolean(CompilerArena);

        bool IsFP = Type->GetFloatingPointType();

        bool IsSigned = false;
        if (!IsFP)
            if (auto IntType = Type->GetIntegerType())
                IsSigned = IntType->IsSigned;

        switch (Comparison->Type)
        {
            case Operator::EQ:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOEQ(LeftVal, RightVal) :
                                Builder.CreateICmpEQ(LeftVal, RightVal), BoolType);
            case Operator::NEQ: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpONE(LeftVal, RightVal) :
                                Builder.CreateICmpNE(LeftVal, RightVal), BoolType);
            case Operator::LT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOLT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLT(LeftVal, RightVal) :
                                Builder.CreateICmpULT(LeftVal, RightVal), BoolType);
            case Operator::LTE: return Create<TypedValue>( IsFP ?
                                Builder.CreateFCmpOLE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSLE(LeftVal, RightVal) :
                                Builder.CreateICmpULE(LeftVal, RightVal), BoolType);
            case Operator::GT:  return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGT(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGT(LeftVal, RightVal) :
                                Builder.CreateICmpUGT(LeftVal, RightVal), BoolType);
            case Operator::GTE: return Create<TypedValue>(IsFP ?
                                Builder.CreateFCmpOGE(LeftVal, RightVal) : IsSigned ?
                                Builder.CreateICmpSGE(LeftVal, RightVal) :
                                Builder.CreateICmpUGE(LeftVal, RightVal), BoolType);
            default: ERROR("Unknown comparison operator")
        }
    }

    TypedValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        TypedValue* Left = CompileNode(Logical->Left);
        Left = ImplicitCast(Left, DataType::CreateBoolean(CompilerArena));

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
                Right = ImplicitCast(Right, DataType::CreateBoolean(CompilerArena));
                Builder.CreateCondBr(Right->GetValue(), OrTrueBB, OrEndBB);

                Builder.SetInsertPoint(OrTrueBB);
                Builder.CreateBr(OrEndBB);

                Builder.CreateBr(OrEndBB);

                Builder.SetInsertPoint(OrEndBB);
                llvm::PHINode* Phi = Builder.CreatePHI(llvm::Type::getInt1Ty(Context), 2);
                Phi->addIncoming(Builder.getTrue(), OrTrueBB);
                Phi->addIncoming(Builder.getFalse(), OrRhsBB);

                return Create<TypedValue>(
                      Phi, DataType::CreateBoolean(CompilerArena));
            }
            case Operator::LOGICAL_AND:
            {
                auto* AndRhsBB   = llvm::BasicBlock::Create(Context, "and.rhs", Func);
                auto* AndFalseBB= llvm::BasicBlock::Create(Context, "and.false", Func);
                auto* AndEndBB  = llvm::BasicBlock::Create(Context, "and.end", Func);

                Builder.CreateCondBr(Left->GetValue(), AndRhsBB, AndFalseBB);

                Builder.SetInsertPoint(AndRhsBB);
                TypedValue* Right = CompileNode(Logical->Right);
                Right = ImplicitCast(Right, DataType::CreateBoolean(CompilerArena));
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndFalseBB);
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndEndBB);
                llvm::PHINode* Phi =
                    Builder.CreatePHI(Builder.getInt1Ty(), 2);

                Phi->addIncoming(Right->GetValue(), AndRhsBB);
                Phi->addIncoming(Builder.getFalse(), AndFalseBB);

                return Create<TypedValue>(
                      Phi, DataType::CreateBoolean(CompilerArena));
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

        llvm::Value* Left = Builder.CreateLoad(Type->GetLLVMType(Context), Value);

        bool IsFP = Type->GetFloatingPointType();

        bool IsSigned = false;
        if (!IsFP)
            if (auto IntType = Type->GetIntegerType())
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

        CastToJointType(Left, Right);
        DataType* Type = Left->GetDataType();

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        bool IsFP = Type->GetFloatingPointType();
        bool IsSigned = false;
        if (auto IntType = Type->GetIntegerType())
            IsSigned = IntType->IsSigned;

        switch (BinaryOp->Type)
        {
            case Operator::ADD:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFAdd(LeftVal, RightVal) :
                                    Builder.CreateAdd(LeftVal, RightVal), Type);
            case Operator::SUB:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFSub(LeftVal, RightVal) :
                                    Builder.CreateSub(LeftVal, RightVal), Type);
            case Operator::MUL:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFMul(LeftVal, RightVal) :
                                    Builder.CreateMul(LeftVal, RightVal), Type);
            case Operator::DIV:     return Create<TypedValue>(IsFP ?
                                    Builder.CreateFDiv(LeftVal, RightVal) : IsSigned ?
                                    Builder.CreateSDiv(LeftVal, RightVal) :
                                    Builder.CreateUDiv(LeftVal, RightVal), Type);
            case Operator::MOD:     return Create<TypedValue>(IsSigned ?
                                    Builder.CreateSRem(LeftVal, RightVal) :
                                    Builder.CreateURem(LeftVal, RightVal), Type);
            case Operator::BIT_AND: return Create<TypedValue>(
                                    Builder.CreateAnd(LeftVal, RightVal), Type);
            case Operator::BIT_OR:  return Create<TypedValue>(
                                    Builder.CreateOr(LeftVal, RightVal), Type);
            case Operator::BIT_XOR: return Create<TypedValue>(
                                    Builder.CreateXor(LeftVal, RightVal), Type);
            case Operator::LSHIFT:  return  Create<TypedValue>(
                                    Builder.CreateShl(LeftVal, RightVal), Type);
            case Operator::RSHIFT:  return Create<TypedValue>(IsSigned ?
                                    Builder.CreateAShr(LeftVal, RightVal) :
                                    Builder.CreateLShr(LeftVal, RightVal), Type);
            default: ERROR("Unknown binary operator")
        }
    }

    TypedValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& FuncName = Identifier->Value.ToString();

            llvm::SmallVector<llvm::Value*, 8> LLVMArgs;
            llvm::SmallVector<DataTypeBase*, 8> ArgTypes;
            llvm::SmallVector<TypedValue*> ArgValues;

            const auto& Args = Call->Arguments;
            LLVMArgs.reserve(Args.size());
            ArgTypes.reserve(Args.size());
            ArgValues.reserve(Args.size());

            for (const auto Arg : Args)
            {
                TypedValue* ArgValue = CompileNode(Arg);

                LLVMArgs.push_back(ArgValue->GetValue());
                ArgTypes.push_back(ArgValue->GetDataType()->GetTypeBase());
                ArgValues.push_back(ArgValue);
            }

            FunctionSignature Signature{ FuncName, ArgTypes };

            if (auto Iter = FunctionSignatures.find(Signature); Iter != FunctionSignatures.end())
                return Create<TypedValue>(Builder.CreateCall(
                            Iter->second->GetFunction(), LLVMArgs), Iter->second->GetReturnType());

            if (auto Iter = DefaultFunctionSignatures.find(Signature); Iter != DefaultFunctionSignatures.end())
            {
                llvm::Function* OutFunc = Module->getFunction(Iter->second.first);
                return Create<TypedValue>(Builder.CreateCall(OutFunc,LLVMArgs),
                Iter->second.second );
            }

            int MinDiffRank = std::numeric_limits<int>::max();
            TypedFunction* Function = nullptr;
            const FunctionSignature* BestFunctionSignature = nullptr;
            for (const auto& [FuncSignature, Func] : FunctionSignatures)
            {
                if (FuncSignature.Name != Signature.Name)
                    continue;

                if (FuncSignature.Params.size() != Signature.Params.size())
                    continue;

                int DiffRank = 0;
                bool HasNonImplicitCastTypes = false;
                for (size_t i = 0; i < FuncSignature.Params.size(); i++)
                {
                    DataType* ParamType1 = DataType::Create(FuncSignature.Params[i], CompilerArena);
                    DataType* ParamType2 = DataType::Create(Signature.Params[i], CompilerArena);
                    if (!CanImplicitCast(ParamType1, ParamType2))
                    {
                        HasNonImplicitCastTypes = true;
                        break;
                    }

                    int Rank1 = ParamType1->GetPrimitiveTypeRank();
                    int Rank2 = ParamType2->GetPrimitiveTypeRank();

                    if (Rank1 == -1 || Rank2 == -1)
                    {
                        HasNonImplicitCastTypes = true;
                        break;
                    }

                    DiffRank += std::abs(Rank1 - Rank2);
                }

                if (HasNonImplicitCastTypes)
                    continue;

                if (MinDiffRank > DiffRank)
                {
                    Function = Func;
                    MinDiffRank = DiffRank;
                    BestFunctionSignature = &FuncSignature;
                }
            }

            if (BestFunctionSignature && Function)
            {
                for (size_t i = 0; i < BestFunctionSignature->Params.size(); i++)
                {
                    DataType* ParamType = DataType::Create(BestFunctionSignature->Params[i], CompilerArena);

                    TypedValue* Value = ArgValues[i];
                    Value = ImplicitCast(Value, ParamType);

                    LLVMArgs[i] = Value->GetValue();
                }

                return Create<TypedValue>(Builder.CreateCall(
                           Function->GetFunction(), LLVMArgs), Function->GetReturnType());
            }

            ERROR("Function '" + FuncName + "' not found");
        }

        ERROR("Called object is not a function")
    }

    TypedValue *LLVMCompiler::CompileSubscript(const SubscriptNode *Subscript)
    {
        TypedValue* Ptr = CompileNode(Subscript->Target);
        PointerType* ElPtrType = Ptr->GetDataType()->GetPtrType();
        if (!ElPtrType)
            ERROR("Cannot apply subscript to non-pointer type");


        llvm::Type* ElType = DataType::GetLLVMType(ElPtrType->BaseType, Context);
        TypedValue* Index = CompileNode(Subscript->Index);

        llvm::Value* ElPtr = Builder.CreateGEP( ElType, Ptr->GetValue(), Index->GetValue());
        llvm::Value* El = Builder.CreateLoad(ElType, ElPtr);
        return Create<TypedValue>(El, DataType::Create(ElPtrType->BaseType, CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::Type* Type = DataType::GetLLVMType(Var->Type->Type, Context);

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type, nullptr, Var->Name.ToString());

        DeclareVariable(Var->Name.ToString(),
            Create<TypedValue>(Alloca, DataType::Create(Var->Type->Type, CompilerArena), true));

        if (Var->Value)
        {
            TypedValue* Value = CompileNode(Var->Value);
            Value = ImplicitCast(Value, DataType::Create(Var->Type->Type, CompilerArena));
            Builder.CreateStore(Value->GetValue(), Alloca);
        }

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        llvm::SmallVector<llvm::Type*, 8> Params;
        Params.reserve(Function->Params.size());

        for (const auto Param : Function->Params)
            Params.push_back(DataType::GetLLVMType(Param->Type->Type, Context));

        llvm::Type* RetType = DataType::GetLLVMType(Function->ReturnType->Type, Context);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        const std::string& FuncName = Function->Name.ToString();
        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());

        llvm::SmallVector<DataTypeBase*, 8> ParamsTypes;

        const auto& FuncParams = Function->Params;
        ParamsTypes.reserve(FuncParams.size());
        for (size_t i = 0; i < FuncParams.size(); i++)
        {
            auto Arg = Func->args().begin() + i;
            Arg->setName(FuncParams[i]->Name.ToString());
            ParamsTypes.push_back(FuncParams[i]->Type->Type);
        }

        CurrentFunction = Func;
        FunctionParams = ParamsTypes;

        FunctionSignature Signature{ FuncName, ParamsTypes };
        FunctionSignatures[Signature] = Create<TypedFunction>(Func,
                                        DataType::Create(Function->ReturnType->Type, CompilerArena));

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
        }
        else
            Builder.CreateRetVoid();
        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileIf(const IfNode *If)
    {
        TypedValue* Cond = CompileNode(If->Condition);
        Cond = ImplicitCast(Cond,DataType::CreateBoolean(CompilerArena));

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
        Cond = ImplicitCast(Cond, DataType::CreateBoolean(CompilerArena));

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

        ERROR("Unsupported operation: while")
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
        Cond = ImplicitCast(Cond, DataType::CreateBoolean(CompilerArena));

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

        ERROR("Unsupported operation: for");
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
            ERROR("This variable: '" + Name + "' has been declared in this scope");

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
            TypedValue* Ptr = CompileNode(Subscript->Target);
            PointerType* ElPtrType = Ptr->GetDataType()->GetPtrType();
            if (!ElPtrType)
                ERROR("Cannot apply subscript to non-pointer type");


            llvm::Type* ElType = DataType::GetLLVMType(ElPtrType->BaseType, Context);
            TypedValue* Index = CompileNode(Subscript->Index);

            llvm::Value* ElPtr = Builder.CreateGEP( ElType, Ptr->GetValue(), Index->GetValue());
            return Create<TypedValue>(ElPtr, DataType::Create(ElPtrType->BaseType, CompilerArena), true);
        }

        return nullptr;
    }

    void LLVMCompiler::CastToJointType(TypedValue *&Left, TypedValue *&Right)
    {
        DataType* LeftType = Left->GetDataType();
        DataType* RightType = Right->GetDataType();

        int LeftRank = LeftType->GetPrimitiveTypeRank();
        int RightRank = RightType->GetPrimitiveTypeRank();

        if (LeftRank == -1 || RightRank == -1)
            ERROR("Invalid cast");

        if (LeftRank == RightRank)
            return;

        if (LeftRank > RightRank)
            Right = ImplicitCast(Right, LeftType);
        else
            Left = ImplicitCast(Left, RightType);
    }

    TypedValue *LLVMCompiler::ImplicitCast(TypedValue *Value, DataType *Target)
    {
        DataType* SrcType = Value->GetDataType();

        if (SrcType->IsEqual(Target))
            return Value;

        if (SrcType->GetBooleanType())
        {
            if (Target->GetIntegerType())
                return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                            Target->GetLLVMType(Context)), Target);

            if (Target->GetFloatingPointType())
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                            Target->GetLLVMType(Context)), Target);

            if (Target->GetPtrType())
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantPointerNull::get(
                            llvm::cast<llvm::PointerType>(
                            SrcType->GetLLVMType(Context)))), Target);
        }

        if (SrcType->GetIntegerType())
        {
            if (Target->GetBooleanType())
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantInt::get(SrcType->GetLLVMType(Context), 0)), Target);

            if (Target->GetIntegerType())
            {
                if (SrcType->GetTypeBitWidth() < Target->GetTypeBitWidth())
                    return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                        Target->GetLLVMType(Context)), Target);

                return Create<TypedValue>(Builder.CreateTrunc(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);
            }

            if (Target->GetFloatingPointType())
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);
        }

        if (SrcType->GetFloatingPointType())
        {
            if (Target->GetBooleanType())
                return Create<TypedValue>(Builder.CreateFCmpONE(Value->GetValue(),
                            llvm::ConstantFP::get(SrcType->GetLLVMType(Context), 0.0 )), Target);

            if (Target->GetIntegerType())
                return Create<TypedValue>(Builder.CreateFPToSI(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);

            if (Target->GetFloatingPointType())
            {
                if (SrcType->GetTypeBitWidth() < Target->GetTypeBitWidth())
                    return Create<TypedValue>(Builder.CreateFPExt(Value->GetValue(),
                        Target->GetLLVMType(Context)), Target);

                return Create<TypedValue>(Builder.CreateFPTrunc(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);
            }
        }

        ERROR("Invalid cast")
    }

    bool LLVMCompiler::CanImplicitCast(DataType *Src, DataType *Dst)
    {
        return Src->GetPrimitiveType() && !Src->GetVoidType() &&
               Dst->GetPrimitiveType() && !Dst->GetVoidType();
    }
}
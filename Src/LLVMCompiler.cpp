//
// Created by bohdan on 03.01.26.
//

#include "LLVMCompiler.h"
#include "DefaultFunction.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#define ERROR(Message) throw CompilerError(Message);

namespace Volt
{
    void LLVMCompiler::Compile()
    {
        llvm::FunctionType* OutIntTy = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        { llvm::Type::getInt32Ty(Context) },
        false
        );

        llvm::FunctionType* OutStrTy = llvm::FunctionType::get(
            llvm::Type::getVoidTy(Context),
            { llvm::PointerType::get(Context, 0) },
            false
        );

        llvm::Function::Create(
            OutIntTy,
            llvm::Function::ExternalLinkage,
            "OutInt",
            Module.get()
        );

        llvm::Function::Create(
            OutStrTy,
            llvm::Function::ExternalLinkage,
            "OutStr",
            Module.get()
        );

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

        Symbols[JIT->get()->mangleAndIntern("OutInt")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(&OutInt),
                llvm::JITSymbolFlags::Exported
            );

        Symbols[JIT->get()->mangleAndIntern("OutStr")] =
            llvm::orc::ExecutorSymbolDef(
                llvm::orc::ExecutorAddr::fromPtr(&OutStr),
                    llvm::JITSymbolFlags::Exported
            );

        cantFail(JIT->get()->getMainJITDylib().define(
            llvm::orc::absoluteSymbols(Symbols)
        ));

        llvm::orc::ThreadSafeModule TSM{std::move(Module),
            std::move(NewContext)};

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
        if (const auto Int = Cast<IntegerNode>(Node))
            return CompileInt(Int);
        if (const auto Bool = Cast<BoolNode>(Node))
            return CompileBool(Bool);
        if (const auto Float = Cast<FloatingPointNode>(Node))
            return CompileFloat(Float);
        if (const auto String = Cast<StringNode>(Node))
            return CompileString(String);
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
                DeclareVariable(Arg.getName().str(), Create<TypedValue>(Alloca, Create<DataType>(FunctionParams[i])));
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
                    DataType::CreatePrimitive(PrimitiveDataType::BYTE, CompilerArena));
            case IntegerNode::INT:
                return Create<TypedValue>(
                    llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Int->Value),
                    DataType::CreatePrimitive(PrimitiveDataType::INT, CompilerArena));
            case IntegerNode::LONG:
                return Create<TypedValue>(
                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), Int->Value),
                    DataType::CreatePrimitive(PrimitiveDataType::LONG, CompilerArena));
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
                    DataType::CreatePrimitive(PrimitiveDataType::DOUBLE, CompilerArena));
            case FloatingPointNode::FLOAT:
                return Create<TypedValue>(
                    llvm::ConstantFP::get(llvm::Type::getFloatTy(Context), Float->Value),
                    DataType::CreatePrimitive(PrimitiveDataType::FLOAT, CompilerArena));
            default:
                ERROR("Unknown float type")
        }
    }

    TypedValue *LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
            DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return Create<TypedValue>(
            llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value),
            DataType::CreatePrimitive(PrimitiveDataType::CHAR, CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileString(const StringNode *String)
    {
        return Create<TypedValue>(Builder.CreateGlobalString(String->Value.ToString()),
            DataType::CreatePtr(Create<PrimitiveDataTypeNode>(PrimitiveDataType::CHAR ), CompilerArena));
    }

    TypedValue *LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        const std::string Value = Identifier->Value.ToString();

        if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        {
            TypedValue* Var = Iter->second;
            return  Create<TypedValue>(Builder.CreateLoad(Var->GetDataType()->GetLLVMType(Context),
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

        DataType* BoolType = DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena);

        switch (Unary->Type)
        {
            case Operator::ADD:         return TValue;
            case Operator::SUB:         return Create<TypedValue>(Builder.CreateNeg(Value), Type);
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

        DataType* BoolType = DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena);

        if (Type->IsIntegerType())
        {
            switch (Comparison->Type)
            {
                case Operator::EQ:  return Create<TypedValue>(
                                    Builder.CreateICmpEQ(LeftVal, RightVal),  BoolType);
                case Operator::NEQ: return Create<TypedValue>(
                                    Builder.CreateICmpNE(LeftVal, RightVal),  BoolType);
                case Operator::LT:  return Create<TypedValue>(
                                    Builder.CreateICmpSLT(LeftVal, RightVal), BoolType);
                case Operator::LTE: return Create<TypedValue>(
                                    Builder.CreateICmpSLE(LeftVal, RightVal), BoolType);
                case Operator::GT:  return Create<TypedValue>(
                                    Builder.CreateICmpSGT(LeftVal, RightVal), BoolType);
                case Operator::GTE: return Create<TypedValue>(
                                    Builder.CreateICmpSGE(LeftVal, RightVal), BoolType);
                default: ERROR("Unknown comparison operator")
            }
        }

        ERROR("Unsupported operation with non-integer types");
    }

    TypedValue *LLVMCompiler::CompileLogical(const LogicalNode *Logical)
    {
        TypedValue* Left = CompileNode(Logical->Left);

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
                Builder.CreateCondBr(Right->GetValue(), OrTrueBB, OrEndBB);

                Builder.SetInsertPoint(OrTrueBB);
                Builder.CreateBr(OrEndBB);

                Builder.CreateBr(OrEndBB);

                Builder.SetInsertPoint(OrEndBB);
                llvm::PHINode* Phi = Builder.CreatePHI(llvm::Type::getInt1Ty(Context), 2);
                Phi->addIncoming(Builder.getTrue(), OrTrueBB);
                Phi->addIncoming(Builder.getFalse(), OrRhsBB);

                return Create<TypedValue>(
                      Phi, DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena));
            }
            case Operator::LOGICAL_AND:
            {
                auto* AndRhsBB   = llvm::BasicBlock::Create(Context, "and.rhs", Func);
                auto* AndFalseBB= llvm::BasicBlock::Create(Context, "and.false", Func);
                auto* AndEndBB  = llvm::BasicBlock::Create(Context, "and.end", Func);

                Builder.CreateCondBr(Left->GetValue(), AndRhsBB, AndFalseBB);

                Builder.SetInsertPoint(AndRhsBB);
                TypedValue* Right = CompileNode(Logical->Right);
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndFalseBB);
                Builder.CreateBr(AndEndBB);

                Builder.SetInsertPoint(AndEndBB);
                llvm::PHINode* Phi =
                    Builder.CreatePHI(Builder.getInt1Ty(), 2);

                Phi->addIncoming(Right->GetValue(), AndRhsBB);
                Phi->addIncoming(Builder.getFalse(), AndFalseBB);

                return Create<TypedValue>(
                      Phi, DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena));
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

        bool IsFP = Type->IsFloatingPointType();

        switch (Assignment->Type)
        {
            case Operator::ADD_ASSIGN:
                Left = Builder.CreateAdd(Left, RightVal);
                break;
            case Operator::SUB_ASSIGN:
                Left = Builder.CreateSub(Left, RightVal);
                break;
            case Operator::MUL_ASSIGN:
                Left = Builder.CreateMul(Left, RightVal);
                break;
            case Operator::DIV_ASSIGN:
                Left = IsFP ? Builder.CreateFDiv(Left, RightVal) :
                Builder.CreateSDiv(Left, RightVal);
                break;
            case Operator::MOD_ASSIGN:
                Left = Builder.CreateSRem(Left, RightVal);
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

        llvm::Value* LeftVal = Left->GetValue();
        llvm::Value* RightVal = Right->GetValue();

        CastToJointType(Left, Right);
        DataType* Type = Left->GetDataType();
        bool IsFP = Type->IsFloatingPointType();

        switch (BinaryOp->Type)
        {
            case Operator::ADD:     return Create<TypedValue>(
                                    Builder.CreateAdd(LeftVal, RightVal), Type);
            case Operator::SUB:     return Create<TypedValue>(
                                    Builder.CreateSub(LeftVal, RightVal), Type);
            case Operator::MUL:     return Create<TypedValue>(
                                    Builder.CreateMul(LeftVal, RightVal), Type);
            case Operator::DIV:     return Create<TypedValue>(IsFP ? Builder.CreateFDiv(LeftVal, RightVal) :
            Builder.CreateSDiv(LeftVal, RightVal), Type);
            case Operator::MOD:     return Create<TypedValue>(
                                    Builder.CreateSRem(LeftVal, RightVal), Type);
            case Operator::BIT_AND: return Create<TypedValue>(
                                    Builder.CreateAnd(LeftVal, RightVal), Type);
            case Operator::BIT_OR:  return Create<TypedValue>(
                                    Builder.CreateOr(LeftVal, RightVal), Type);
            case Operator::BIT_XOR: return Create<TypedValue>(
                                    Builder.CreateXor(LeftVal, RightVal), Type);
            case Operator::LSHIFT:  return  Create<TypedValue>(
                                    Builder.CreateShl(LeftVal, RightVal), Type);
            case Operator::RSHIFT:  return Create<TypedValue>(
                                    Builder.CreateAShr(LeftVal, RightVal), Type);
            default: ERROR("Unknown binary operator")
        }
    }

    TypedValue* LLVMCompiler::CompileCall(const CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& FuncName = Identifier->Value.ToString();

            if (FuncName == "OutInt" || FuncName == "OutStr")
            {
                llvm::Function* OutFunc = Module->getFunction(FuncName);
                return Create<TypedValue>(Builder.CreateCall(
                    OutFunc,{ CompileNode(Call->Arguments[0])->GetValue() }),
                    DataType::CreatePrimitive(PrimitiveDataType::VOID, CompilerArena) );
            }

            llvm::SmallVector<llvm::Value*, 8> LLVMArgs;
            llvm::SmallVector<DataTypeNodeBase*, 8> ArgTypes;

            const auto& Args = Call->Arguments;
            LLVMArgs.reserve(Args.size());
            ArgTypes.reserve(Args.size());

            for (const auto Arg : Args)
            {
                TypedValue* ArgValue = CompileNode(Arg);

                LLVMArgs.push_back(ArgValue->GetValue());
                ArgTypes.push_back(ArgValue->GetDataType()->GetTypeBase());
            }

            FunctionSignature Signature{ FuncName, ArgTypes };

            if (auto Iter = FunctionSignatures.find(Signature); Iter != FunctionSignatures.end())
                return Create<TypedValue>(Builder.CreateCall(
                            Iter->second->GetFunction(), LLVMArgs), Iter->second->GetReturnType());

            ERROR("Function '" + FuncName + "' not found");
        }

        ERROR("Called object is not a function")
    }

    TypedValue *LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        llvm::Type* Type = DataType::GetLLVMType(Var->Type, Context);

        llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type, nullptr, Var->Name.ToString());

        DeclareVariable(Var->Name.ToString(),
            Create<TypedValue>(Alloca, Create<DataType>(Var->Type), true));

        if (Var->Value)
        {
            TypedValue* Value = CompileNode(Var->Value);
            Builder.CreateStore(Value->GetValue(), Alloca);
        }

        return nullptr;
    }

    TypedValue *LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        llvm::SmallVector<llvm::Type*, 8> Params;
        Params.reserve(Function->Params.size());

        for (const auto Param : Function->Params)
            Params.push_back(DataType::GetLLVMType(Param->Type, Context));

        llvm::Type* RetType = DataType::GetLLVMType(Function->ReturnType, Context);
        llvm::FunctionType* FuncType = llvm::FunctionType::get(
            RetType, Params, false);

        const std::string& FuncName = Function->Name.ToString();
        llvm::Function* Func = llvm::Function::Create(
            FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());

        llvm::SmallVector<DataTypeNodeBase*, 8> ParamsTypes;

        const auto& FuncParams = Function->Params;
        ParamsTypes.reserve(FuncParams.size());
        for (size_t i = 0; i < FuncParams.size(); i++)
        {
            auto Arg = Func->args().begin() + i;
            Arg->setName(FuncParams[i]->Name.ToString());
            ParamsTypes.push_back(FuncParams[i]->Type);
        }

        CurrentFunction = Func;
        FunctionParams = ParamsTypes;

        FunctionSignature Signature{ FuncName, ParamsTypes };
        FunctionSignatures[Signature] = Create<TypedFunction>(Func, Create<DataType>(Function->ReturnType));

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
        Cond = ImplicitCast(Cond,DataType::CreatePrimitive(PrimitiveDataType::BOOL, CompilerArena));

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
        //Cond = CastToBool(Cond);

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

        llvm::BasicBlock* InitializationBB = llvm::BasicBlock::Create(Context, "for.initialization", Func);
        Builder.CreateBr(InitializationBB);
        Builder.SetInsertPoint(InitializationBB);
        CompileNode(For->Initialization);

        llvm::BasicBlock* ForHeader = llvm::BasicBlock::Create(Context, "for.header", Func);
        Builder.CreateBr(ForHeader);
        Builder.SetInsertPoint(ForHeader);
        TypedValue* Cond = CompileNode(For->Condition);

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
        {
            Entry.HadPrevious = true;
            Entry.Previous = Iter->second;
        }

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
            if (Entry.HadPrevious)
                SymbolTable[Entry.Name] = Entry.Previous;
            else
                SymbolTable.erase(Entry.Name);
        }
    }

    TypedValue *LLVMCompiler::GetLValue(const ASTNode *Node)
    {
        if (const auto Identifier = Cast<const IdentifierNode>(Node))
            return GetVariable(Identifier->Value.ToString());

        return nullptr;
    }

    void LLVMCompiler::CreateDefaultFunction(const std::string &Name, llvm::Type *RetType,
        const llvm::SmallVector<llvm::Type*, 8> &Params) const
    {
        llvm::FunctionType* FuncType = llvm::FunctionType::get(RetType, Params, false);
        llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, Name, Module.get());
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

        if (SrcType->IsBooleanType())
        {
            if (Target->IsIntegerType())
                return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                            Target->GetLLVMType(Context)), Target);

            if (Target->IsFloatingPointType())
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                            Target->GetLLVMType(Context)), Target);

            if (Target->GetPtrType())
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantPointerNull::get(
                            llvm::cast<llvm::PointerType>(
                            SrcType->GetLLVMType(Context)))), Target);
        }

        if (SrcType->IsIntegerType())
        {
            if (Target->IsBooleanType())
                return Create<TypedValue>(Builder.CreateICmpNE(Value->GetValue(),
                            llvm::ConstantInt::get(SrcType->GetLLVMType(Context), 0)), Target);

            if (Target->IsIntegerType())
            {
                if (SrcType->GetTypeBitWidth() < Target->GetTypeBitWidth())
                    return Create<TypedValue>(Builder.CreateSExt(Value->GetValue(),
                        Target->GetLLVMType(Context)), Target);

                return Create<TypedValue>(Builder.CreateTrunc(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);
            }

            if (Target->IsFloatingPointType())
                return Create<TypedValue>(Builder.CreateSIToFP(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);
        }

        if (SrcType->IsFloatingPointType())
        {
            if (Target->IsBooleanType())
                return Create<TypedValue>(Builder.CreateFCmpONE(Value->GetValue(),
                            llvm::ConstantFP::get(SrcType->GetLLVMType(Context), 0.0 )), Target);

            if (Target->IsIntegerType())
                return Create<TypedValue>(Builder.CreateFPToSI(Value->GetValue(),
                    Target->GetLLVMType(Context)), Target);

            if (Target->IsFloatingPointType())
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
}
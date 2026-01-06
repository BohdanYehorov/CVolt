//
// Created by bohdan on 03.01.26.
//

#include "LLVMCompiler.h"
#include "DefaultFunction.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#define ERROR(Message) throw CompilerError(Message);

void LLVMCompiler::Compile()
{
    llvm::FunctionType* OutTy = llvm::FunctionType::get(
    llvm::Type::getVoidTy(Context),
    { llvm::Type::getInt32Ty(Context) },
    false
);

    llvm::Function::Create(
        OutTy,
        llvm::Function::ExternalLinkage,
        "Out",
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

    Symbols[JIT->get()->mangleAndIntern("Out")] =
        llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(&Out),
            llvm::JITSymbolFlags::Exported
        );

    cantFail(JIT->get()->getMainJITDylib().define(
        llvm::orc::absoluteSymbols(Symbols)
    ));

    llvm::orc::ThreadSafeModule TSM{std::move(Module),
        std::move(NewContext)};

    if (auto Err = JIT->get()->addIRModule(std::move(TSM)))
        return 1;
    auto MainSymOrErr = JIT->get()->lookup("main");
    if (!MainSymOrErr) {
        llvm::logAllUnhandledErrors(MainSymOrErr.takeError(), llvm::errs(), "Error: ");
        return 1;
    }

    const auto MainFunc = reinterpret_cast<int(*)()>(MainSymOrErr->getValue());
    return MainFunc();
}

llvm::Value* LLVMCompiler::CompileNode(ASTNode *Node)
{
    if (auto Sequence = Cast<SequenceNode>(Node))
    {
        for (auto Statement : Sequence->Statements)
            CompileNode(Statement);

        return nullptr;
    }
    if (const auto Block = Cast<BlockNode>(Node))
        return CompileBlock(Block);
    if (const auto Int = Cast<IntNode>(Node))
        return CompileIntNode(Int);
    if (const auto Bool = Cast<BoolNode>(Node))
        return CompileBool(Bool);
    if (const auto Identifier = Cast<IdentifierNode>(Node))
        return CompileIdentifier(Identifier);
    if (const auto Prefix = Cast<PrefixOpNode>(Node))
        return CompilePrefix(Prefix);
    if (const auto Suffix = Cast<SuffixOpNode>(Node))
        return CompileSufix(Suffix);
    if (const auto Comparison = Cast<ComparisonNode>(Node))
        return CompileComparison(Comparison);
    if (const auto AssignOp = Cast<AssignmentNode>(Node))
        return CompileAssignment(AssignOp);
    if (const auto BinaryOp = Cast<BinaryOpNode>(Node))
        return CompileBinaryOpNode(BinaryOp);
    if (const auto Call = Cast<CallNode>(Node))
        return CompileCallNode(Call);
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

    ERROR("Cannot resolve node: '" + Node->GetName() + "'");
    return nullptr;
}

llvm::Value* LLVMCompiler::CompileBlock(const BlockNode *Block)
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

llvm::Value* LLVMCompiler::CompileIntNode(const IntNode *Int)
{
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Int->Value);
}

llvm::Value* LLVMCompiler::CompileBool(const BoolNode *Bool)
{
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value);
}

llvm::Value* LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
{
    const std::string Value = Identifier->Value.ToString();

    if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
    {
        llvm::AllocaInst* Alloca = Iter->second;
        return Builder.CreateLoad(Alloca->getAllocatedType(), Alloca, Value + "_val");
    }

    ERROR("Cannot resolve symbol: '" + Value + "'")
    return nullptr;
}

llvm::Value* LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
{
    llvm::Value* Left = CompileNode(Comparison->Left);
    llvm::Value* Right = CompileNode(Comparison->Right);

    CastToJointType(Left, Right);

    llvm::Type* Type = Left->getType();

    if (Type->isIntegerTy())
    {
        switch (Comparison->Type)
        {
            case Operator::EQ:  return Builder.CreateICmpEQ(Left, Right);
            case Operator::NEQ: return Builder.CreateICmpNE(Left, Right);
            case Operator::LT:  return Builder.CreateICmpSLT(Left, Right);
            case Operator::LTE: return Builder.CreateICmpSLE(Left, Right);
            case Operator::GT:  return Builder.CreateICmpSGT(Left, Right);
            case Operator::GTE: return Builder.CreateICmpSGE(Left, Right);
            default:
                ERROR("Unknown comparison operator")
        }
    }

    return nullptr;
}

llvm::Value* LLVMCompiler::CompileAssignment(const AssignmentNode *Assignment)
{
    llvm::AllocaInst* LValue = GetLValue(Assignment->Left);
    if (!LValue)
        ERROR("Cannot apply assignment operator to r-value")

    llvm::Value* Right = CompileNode(Assignment->Right);

    if (Assignment->Type == Operator::ASSIGN)
        return Builder.CreateStore(Right, LValue);

    llvm::Value* Left = Builder.CreateLoad(LValue->getAllocatedType(), LValue);
    switch (Assignment->Type)
    {
        case Operator::ADD_ASSIGN:
            Left = Builder.CreateAdd(Left, Right);
            break;
        case Operator::SUB_ASSIGN:
            Left = Builder.CreateSub(Left, Right);
            break;
        case Operator::MUL_ASSIGN:
            Left = Builder.CreateMul(Left, Right);
            break;
        default:
            ERROR("Unknown assignment operator")
    }

    return Builder.CreateStore(Left, LValue);
}

llvm::Value* LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
{
    llvm::AllocaInst* LValue = GetLValue(Prefix->Operand);
    if (!LValue)
        ERROR("Cannot apply prefix operator to r-value")

    llvm::Value* Value = Builder.CreateLoad(LValue->getAllocatedType(), LValue);
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

    Builder.CreateStore(Value, LValue);
    return Value;
}

llvm::Value* LLVMCompiler::CompileSufix(const SuffixOpNode *Suffix)
{
    llvm::AllocaInst* LValue = GetLValue(Suffix->Operand);
    if (!LValue)
        ERROR("Cannot apply suffix operator to r-value")

    llvm::Value* Value = Builder.CreateLoad(LValue->getAllocatedType(), LValue);
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

    Builder.CreateStore(Value, LValue);
    return Temp;
}

llvm::Value* LLVMCompiler::CompileUnary(const UnaryOpNode *Unary)
{
}

llvm::Value* LLVMCompiler::CompileBinaryOpNode(const BinaryOpNode *BinaryOp)
{
    llvm::Value* Left = CompileNode(BinaryOp->Left);
    llvm::Value* Right = CompileNode(BinaryOp->Right);

    CastToJointType(Left, Right);

    switch (BinaryOp->Type)
    {
        case Operator::ADD: return Builder.CreateAdd(Left, Right);
        case Operator::SUB: return Builder.CreateSub(Left, Right);
        case Operator::MUL: return Builder.CreateMul(Left, Right);
        default:
            ERROR("Unknown binary operator")
    }

    return nullptr;
}

llvm::Value * LLVMCompiler::CompileCallNode(const CallNode* Call)
{
    if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
    {
        const std::string& FuncName = Identifier->Value.ToString();
        if (auto Iter = Functions.find(FuncName); Iter != Functions.end())
        {
            llvm::SmallVector<llvm::Value*, 16> Args;
            Args.reserve(Call->Arguments.size());

            for (auto& Arg : Call->Arguments)
                Args.push_back(CompileNode(Arg));

            return Builder.CreateCall(Iter->second);
        }

        if (FuncName == "Out")
        {
            llvm::Function* OutFunc = Module->getFunction("Out");
            return Builder.CreateCall(OutFunc, { CompileNode(Call->Arguments[0]) });
        }

        ERROR("Function '" + FuncName + "' not found");
    }

    ERROR("Called object is not a function")
    return nullptr;
}

llvm::Value* LLVMCompiler::CompileVariable(const VariableNode *Var)
{
    llvm::Function* Func = Builder.GetInsertBlock()->getParent();
    llvm::Type* Type = ToLLVMType(Var->Type->TypeInfo.BaseType);

    llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
    llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type, nullptr, Var->Name.ToString());

    DeclareVariable(Var->Name.ToString(), Alloca);

    if (Var->Value)
        Builder.CreateStore(CompileNode(Var->Value), Alloca);

    return nullptr;
}

llvm::Value* LLVMCompiler::CompileFunction(const FunctionNode *Function)
{
    std::vector<llvm::Type*> Params;
    Params.reserve(Function->Params.size());

    for (const auto Param : Function->Params)
        Params.push_back(ToLLVMType(Param->Type->TypeInfo.BaseType));

    llvm::FunctionType* FuncType = llvm::FunctionType::get(
        ToLLVMType(Function->ReturnType->TypeInfo.BaseType), Params, false);

    const std::string& FuncName = Function->Name.ToString();
    llvm::Function* Func = llvm::Function::Create(
        FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());

    llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
    Builder.SetInsertPoint(Entry);
    CompileBlock(Cast<BlockNode>(Function->Body));

    llvm::BasicBlock* Bb = Builder.GetInsertBlock();
    if (!Bb->getTerminator()) {
        if (Function->ReturnType->TypeInfo.BaseType == DataType::VOID)
            Builder.CreateRetVoid();
        else
            Builder.CreateRet(
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(Context),
                    0
                )
            );
    }

    Functions[FuncName] = Func;

    return nullptr;
}

llvm::Value* LLVMCompiler::CompileReturn(const ReturnNode *Return)
{
    llvm::Value* RetVal = CompileNode(Return->ReturnValue);
    Builder.CreateRet(RetVal);
    return nullptr;
}

llvm::Value* LLVMCompiler::CompileIf(const IfNode* If)
{
    llvm::Value* Cond = CompileNode(If->Condition);

    // if (Cond->getType()->isIntegerTy(1)) {
    //     std::cout << "Bool\n";
    // } else if (Cond->getType()->isIntegerTy()) {
    //     // всі інші інт-типи
    //     Cond = Builder.CreateICmpNE(
    //         Cond,
    //         llvm::ConstantInt::get(Cond->getType(), 0),
    //         "ifcond"
    //     );
    // } else if (Cond->getType()->isPointerTy()) {
    //     Cond = Builder.CreateICmpNE(
    //         Cond,
    //         llvm::ConstantPointerNull::get(
    //             llvm::cast<llvm::PointerType>(Cond->getType())
    //         ),
    //         "ifcond"
    //     );
    // } else {
    //     assert(false && "Unsupported type for if condition");
    // }

    Cond = CastInteger(Cond, llvm::Type::getInt1Ty(Context));

    Cond = Builder.CreateICmpNE(
        Cond, llvm::ConstantInt::get(Cond->getType(), 0), "ifcond");

    llvm::Function* Func = Builder.GetInsertBlock()->getParent();

    auto ThenBB  = llvm::BasicBlock::Create(Context, "then", Func);
    auto ElseBB  = llvm::BasicBlock::Create(Context, "else", Func);
    auto MergeBB = llvm::BasicBlock::Create(Context, "ifcont"); // ще без parent

    Builder.CreateCondBr(Cond, ThenBB, ElseBB);

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

llvm::Value * LLVMCompiler::CompileWhile(const WhileNode *While)
{
    llvm::Function* Func = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* LoopHeader = llvm::BasicBlock::Create(Context, "loop.header", Func);
    Builder.CreateBr(LoopHeader);
    Builder.SetInsertPoint(LoopHeader);
    llvm::Value* Cond = CompileNode(While->Condition);
    Cond = Builder.CreateICmpNE(
        Cond, llvm::ConstantInt::get(Cond->getType(), 0));

    llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "loop.body", Func);
    llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
    Builder.CreateCondBr(Cond, ThenBB, EndBB);

    Builder.SetInsertPoint(ThenBB);
    CompileNode(While->Branch);
    if (!Builder.GetInsertBlock()->getTerminator())
        Builder.CreateBr(LoopHeader);

    if (!EndBB->getParent())
        EndBB->insertInto(Func);

    Builder.SetInsertPoint(EndBB);
    return nullptr;
}

llvm::Value* LLVMCompiler::CompileFor(const ForNode *For)
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
    llvm::Value* Cond = CompileNode(For->Condition);
    Cond = Builder.CreateICmpNE(Cond, llvm::ConstantInt::get(Cond->getType(), 0));

    llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "for.body", Func);
    llvm::BasicBlock* LatchBB = llvm::BasicBlock::Create(Context, "for.latch", Func);
    llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
    Builder.CreateCondBr(Cond, ThenBB, EndBB);

    Builder.SetInsertPoint(ThenBB);
    CompileNode(For->Body);
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

llvm::Type* LLVMCompiler::ToLLVMType(DataType Type)
{
    switch (Type)
    {
        case DataType::INT:
            return llvm::Type::getInt32Ty(Context);
        case DataType::FLOAT:
            return llvm::Type::getDoubleTy(Context);
        case DataType::BOOL:
        case DataType::CHAR:
            return llvm::Type::getInt1Ty(Context);
        default:
            ERROR("Unknown data type");
            return nullptr;
    }
}

llvm::Value* LLVMCompiler::CastInteger(llvm::Value *Value, llvm::Type *Target, bool Signed)
{
    llvm::Type* Src = Value->getType();
    if (Src == Target) return Value;

    if (Src->isIntegerTy() && Target->isIntegerTy())
    {
        if (Src->getIntegerBitWidth() < Target->getIntegerBitWidth())
            return Signed ? Builder.CreateSExt(Value, Target) : Builder.CreateZExt(Value, Target);
        return Builder.CreateTrunc(Value, Target);
    }

    ERROR("Cannot cast this type");
    return nullptr;
}

void LLVMCompiler::CastToJointType(llvm::Value*& Left, llvm::Value*& Right, bool Signed)
{
    llvm::Type* LeftType = Left->getType();
    llvm::Type* RightType = Right->getType();

    int LeftRank = GetTypeRank(LeftType);
    int RightRank = GetTypeRank(RightType);

    if (LeftRank == RightRank)
        return;

    if (LeftRank > RightRank)
        Right = ImplicitCast(Right, LeftType, Signed);
    else
        Left = ImplicitCast(Left, RightType, Signed);
}

int LLVMCompiler::GetTypeRank(llvm::Type *Type)
{
    if (Type->isIntegerTy(1)) return 0;

    if (Type->isIntegerTy(8)) return 1;
    if (Type->isIntegerTy(16)) return 2;
    if (Type->isIntegerTy(32)) return 3;
    if (Type->isIntegerTy(64)) return 4;

    if (Type->isFloatTy()) return 5;
    if (Type->isDoubleTy()) return 6;

    return -1;
}

llvm::Value* LLVMCompiler::ImplicitCast(llvm::Value* Value, llvm::Type* Target, bool Signed)
{
    llvm::Type* Src = Value->getType();

    if (Src == Target)
        return Value;

    if (Src->isIntegerTy())
    {
        if (Target->isIntegerTy())
        {
            if (Src->getIntegerBitWidth() < Target->getIntegerBitWidth())
                return Signed ? Builder.CreateSExt(Value, Target) : Builder.CreateZExt(Value, Target);
            return Builder.CreateTrunc(Value, Target);
        }

        if (Target->isFloatTy())
            return Signed ? Builder.CreateSIToFP(Value, Target) : Builder.CreateUIToFP(Value, Target);
    }

    if (Src->isFloatingPointTy())
    {
        if (Target->isIntegerTy())
            return Signed ? Builder.CreateFPToSI(Value, Target) : Builder.CreateFPToUI(Value, Target);

        if (Target->isFloatingPointTy())
        {
            if (Src->getPrimitiveSizeInBits() < Target->getPrimitiveSizeInBits())
                return Builder.CreateFPExt(Value, Target);
            return Builder.CreateFPTrunc(Value, Target);
        }
    }

    ERROR("Cannot cast this type")
    return nullptr;
}

void LLVMCompiler::DeclareVariable(const std::string &Name, llvm::AllocaInst *AllocaInst)
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
    SymbolTable[Name] = AllocaInst;
}

llvm::AllocaInst* LLVMCompiler::GetVariable(const std::string &Name)
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

llvm::AllocaInst* LLVMCompiler::GetLValue(ASTNode* Node)
{
    if (const auto Identifier = Cast<IdentifierNode>(Node))
        return GetVariable(Identifier->Value.ToString());

    return nullptr;
}
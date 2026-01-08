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

    TypedValue LLVMCompiler::CompileNode(ASTNode *Node)
    {
        if (auto Sequence = Cast<SequenceNode>(Node))
        {
            for (auto Statement : Sequence->Statements)
                CompileNode(Statement);

            return { };
        }
        if (const auto Block = Cast<BlockNode>(Node))
            return CompileBlock(Block);
        if (const auto Int = Cast<IntegerNode>(Node))
            return CompileInt(Int);
        if (const auto Bool = Cast<BoolNode>(Node))
            return CompileBool(Bool);
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

    TypedValue LLVMCompiler::CompileBlock(const BlockNode *Block)
    {
        EnterScope();

        if (CurrentFunction)
        {
            for (llvm::Argument& Arg : CurrentFunction->args())
            {
                llvm::Type* ArgType = Arg.getType();
                llvm::AllocaInst* Alloca = Builder.CreateAlloca(ArgType, nullptr, Arg.getName());

                Builder.CreateStore(&Arg, Alloca);
                DeclareVariable(Arg.getName().str(), Alloca);
            }
            CurrentFunction = nullptr;
        }

        for (auto Stmt : Block->Statements)
        {
            CompileNode(Stmt);

            if (Builder.GetInsertBlock()->getTerminator())
                break;
        }

        ExitScope();

        return { };
    }

    TypedValue LLVMCompiler::CompileInt(const IntegerNode *Int)
    {
        switch (Int->Type)
        {
            case IntegerNode::BYTE:
                return { llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Int->Value),
                            CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::BYTE) };
            case IntegerNode::INT:
                return { llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Int->Value),
                            CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::INT) };
            case IntegerNode::LONG:
                return { llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), Int->Value),
                             CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::LONG) };
            default:
                ERROR("Unknown integer type")
        }
    }

    TypedValue LLVMCompiler::CompileFloat(const FloatingPointNode *Float)
    {
        switch (Float->Type)
        {
            case FloatingPointNode::DOUBLE:
                return { llvm::ConstantFP::get(llvm::Type::getDoubleTy(Context), Float->Value),
                            CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::DOUBLE) };
            case FloatingPointNode::FLOAT:
                return { llvm::ConstantFP::get(llvm::Type::getFloatTy(Context), Float->Value),
                            CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::FLOAT) };
            default:
                ERROR("Unknown float type")
        }
    }

    TypedValue LLVMCompiler::CompileBool(const BoolNode *Bool)
    {
        return { llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), Bool->Value),
                    CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::BOOL) };
    }

    TypedValue LLVMCompiler::CompileChar(const CharNode *Char)
    {
        return { llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), Char->Value),
                    CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::CHAR) };
    }

    TypedValue LLVMCompiler::CompileString(const StringNode *String)
    {
        return { Builder.CreateGlobalString(String->Value.ToString()),
                    CompilerArena.Create<PtrDataTypeNode>(
                        CompilerArena.Create<PrimitiveDataTypeNode>(PrimitiveDataType::CHAR)) };
    }

    TypedValue LLVMCompiler::CompileIdentifier(const IdentifierNode *Identifier)
    {
        // const std::string Value = Identifier->Value.ToString();
        //
        // if (auto Iter = SymbolTable.find(Value); Iter != SymbolTable.end())
        // {
        //     llvm::AllocaInst* Alloca = Iter->second;
        //     //return Builder.CreateLoad(Alloca->getAllocatedType(), Alloca, Value + "_val");
        // }
        //
        // ERROR("Cannot resolve symbol: '" + Value + "'")

        ERROR("Unsupported operation: identifier");
    }

    TypedValue LLVMCompiler::CompileRef(const RefNode *Ref)
    {
        // llvm::AllocaInst* LValue = GetLValue(Ref->Target);
        // if (!LValue)
        //     ERROR("Cannot apply operator '$' to l-value")

        ERROR("Unsupported operation: ref")
        //return LValue;
    }

    TypedValue LLVMCompiler::CompilePrefix(const PrefixOpNode *Prefix)
    {
        // llvm::AllocaInst* LValue = GetLValue(Prefix->Operand);
        // if (!LValue)
        //     ERROR("Cannot apply prefix operator to r-value")
        //
        // llvm::Value* Value = Builder.CreateLoad(LValue->getAllocatedType(), LValue);
        // switch (Prefix->Type)
        // {
        //     case Operator::INC:
        //         Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
        //         break;
        //     case Operator::DEC:
        //         Value = Builder.CreateSub(Value, llvm::ConstantInt::get(Value->getType(), 1));
        //         break;
        //     default:
        //         ERROR("Unknown prefix operator")
        // }
        //
        // Builder.CreateStore(Value, LValue);

        ERROR("Unsupported operation: prefix")
        //return Value;
    }

    TypedValue LLVMCompiler::CompileSuffix(const SuffixOpNode *Suffix)
    {
        // llvm::AllocaInst* LValue = GetLValue(Suffix->Operand);
        // if (!LValue)
        //     ERROR("Cannot apply suffix operator to r-value")
        //
        // llvm::Value* Value = Builder.CreateLoad(LValue->getAllocatedType(), LValue);
        // llvm::Value* Temp = Value;
        // switch (Suffix->Type)
        // {
        //     case Operator::INC:
        //         Value = Builder.CreateAdd(Value, llvm::ConstantInt::get(Value->getType(), 1));
        //         break;
        //     case Operator::DEC:
        //         Value = Builder.CreateSub(Value, llvm::ConstantInt::get(Value->getType(), 1));
        //         break;
        //     default:
        //         ERROR("Unknown suffix operator")
        // }
        //
        // Builder.CreateStore(Value, LValue);

        ERROR("Unsupported operation: suffix")
        //return Temp;
    }

    TypedValue LLVMCompiler::CompileUnary(const UnaryOpNode *Unary)
    {
        // llvm::Value* Value = CompileNode(Unary->Operand).GetValue();
        // switch (Unary->Type)
        // {
        //     case Operator::ADD:         return Value;
        //     case Operator::SUB:         return Builder.CreateNeg(Value);
        //     case Operator::LOGICAL_NOT: return Builder.CreateNot(CastToBool(Value));
        //     case Operator::BIT_NOT:     return Builder.CreateNot(Value);
        //     default: ERROR("Unknown unary operator")
        // }
    }

    TypedValue LLVMCompiler::CompileComparison(const ComparisonNode *Comparison)
    {
        // llvm::Value* Left = CompileNode(Comparison->Left);
        // llvm::Value* Right = CompileNode(Comparison->Right);
        //
        // CastToJointType(Left, Right);
        //
        // llvm::Type* Type = Left->getType();
        //
        // if (Type->isIntegerTy())
        // {
        //     switch (Comparison->Type)
        //     {
        //         case Operator::EQ:  return Builder.CreateICmpEQ(Left, Right);
        //         case Operator::NEQ: return Builder.CreateICmpNE(Left, Right);
        //         case Operator::LT:  return Builder.CreateICmpSLT(Left, Right);
        //         case Operator::LTE: return Builder.CreateICmpSLE(Left, Right);
        //         case Operator::GT:  return Builder.CreateICmpSGT(Left, Right);
        //         case Operator::GTE: return Builder.CreateICmpSGE(Left, Right);
        //         default: ERROR("Unknown comparison operator")
        //     }
        // }
        //
        // return nullptr;

        ERROR("Unsupported operation: comparison")
    }

    TypedValue LLVMCompiler::CompileAssignment(const AssignmentNode *Assignment)
    {
        // llvm::AllocaInst* LValue = GetLValue(Assignment->Left);
        // if (!LValue)
        //     ERROR("Cannot apply assignment operator to r-value")
        //
        // llvm::Value* Right = CompileNode(Assignment->Right);
        //
        // if (Assignment->Type == Operator::ASSIGN)
        //     return Builder.CreateStore(Right, LValue);
        //
        // llvm::Type* Type = LValue->getAllocatedType();
        // llvm::Value* Left = Builder.CreateLoad(Type, LValue);
        //
        // bool IsFP = Type->isFloatingPointTy();
        //
        // switch (Assignment->Type)
        // {
        //     case Operator::ADD_ASSIGN:
        //         Left = Builder.CreateAdd(Left, Right);
        //         break;
        //     case Operator::SUB_ASSIGN:
        //         Left = Builder.CreateSub(Left, Right);
        //         break;
        //     case Operator::MUL_ASSIGN:
        //         Left = Builder.CreateMul(Left, Right);
        //         break;
        //     case Operator::DIV_ASSIGN:
        //         Left = IsFP ? Builder.CreateFDiv(Left, Right) :
        //             Builder.CreateSDiv(Left, Right);
        //         break;
        //     case Operator::MOD_ASSIGN:
        //         Left = Builder.CreateSRem(Left, Right);
        //         break;
        //     default:
        //         ERROR("Unknown assignment operator")
        // }
        //
        // return Builder.CreateStore(Left, LValue);

        ERROR("Unsupported operation: assignment")
    }

    TypedValue LLVMCompiler::CompileBinary(const BinaryOpNode *BinaryOp)
    {
        // llvm::Value* Left = CompileNode(BinaryOp->Left);
        // llvm::Value* Right = CompileNode(BinaryOp->Right);
        //
        // CastToJointType(Left, Right);
        // llvm::Type* Type = Left->getType();
        // bool IsFP = Type->isFloatingPointTy();
        //
        // switch (BinaryOp->Type)
        // {
        //     case Operator::ADD:     return Builder.CreateAdd(Left, Right);
        //     case Operator::SUB:     return Builder.CreateSub(Left, Right);
        //     case Operator::MUL:     return Builder.CreateMul(Left, Right);
        //     case Operator::DIV:     return IsFP ? Builder.CreateFDiv(Left, Right) :
        //                                           Builder.CreateSDiv(Left, Right);
        //     case Operator::MOD:     return Builder.CreateSRem(Left, Right);
        //     case Operator::BIT_AND: return Builder.CreateAnd(Left, Right);
        //     case Operator::BIT_OR:  return Builder.CreateOr(Left, Right);
        //     case Operator::BIT_XOR: return Builder.CreateXor(Left, Right);
        //     case Operator::LSHIFT:  return Builder.CreateShl(Left, Right);
        //     case Operator::RSHIFT:  return Builder.CreateAShr(Left, Right);
        //     default: ERROR("Unknown binary operator")
        // }

        ERROR("Unsupported operation: binary")
    }

    TypedValue LLVMCompiler::CompileCall(const CallNode *Call)
    {
        // if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        // {
        //     const std::string& FuncName = Identifier->Value.ToString();
        //     if (auto Iter = Functions.find(FuncName); Iter != Functions.end())
        //     {
        //         llvm::SmallVector<llvm::Value*, 16> Args;
        //         Args.reserve(Call->Arguments.size());
        //
        //         for (auto& Arg : Call->Arguments)
        //             Args.push_back(CompileNode(Arg));
        //
        //         return Builder.CreateCall(Iter->second, Args);
        //     }
        //
        //     if (FuncName == "OutInt" || FuncName == "OutStr")
        //     {
        //         llvm::Function* OutFunc = Module->getFunction(FuncName);
        //         return Builder.CreateCall(OutFunc, { CompileNode(Call->Arguments[0]) });
        //     }
        //
        //     ERROR("Function '" + FuncName + "' not found");
        // }
        //
        // ERROR("Called object is not a function")

        ERROR("Unsupported operation: call")
    }

    TypedValue LLVMCompiler::CompileVariable(const VariableNode *Var)
    {
        // llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        // llvm::Type* Type = CompileType(Var->Type);
        //
        // llvm::IRBuilder<> TmpBuilder(&Func->getEntryBlock(), Func->getEntryBlock().begin());
        // llvm::AllocaInst* Alloca = TmpBuilder.CreateAlloca(Type, nullptr, Var->Name.ToString());
        //
        // DeclareVariable(Var->Name.ToString(), Alloca);
        //
        // if (Var->Value)
        //     Builder.CreateStore(CompileNode(Var->Value), Alloca);
        //
        // return nullptr;

        ERROR("Unsupported operation: variable")
    }

    TypedValue LLVMCompiler::CompileFunction(const FunctionNode *Function)
    {
        // llvm::SmallVector<llvm::Type*, 8> Params;
        // Params.reserve(Function->Params.size());
        //
        // for (const auto Param : Function->Params)
        //     Params.push_back(CompileType(Param->Type));
        //
        // llvm::Type* RetType = CompileType(Function->ReturnType);
        // llvm::FunctionType* FuncType = llvm::FunctionType::get(
        //     RetType, Params, false);
        //
        // const std::string& FuncName = Function->Name.ToString();
        // llvm::Function* Func = llvm::Function::Create(
        //     FuncType, llvm::Function::ExternalLinkage, FuncName, Module.get());
        //
        // const auto& FuncParams = Function->Params;
        // for (size_t i = 0; i < FuncParams.size(); i++)
        // {
        //     auto Arg = Func->args().begin() + i;
        //     Arg->setName(FuncParams[i]->Name.ToString());
        // }
        //
        // CurrentFunction = Func;
        //
        // llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Context, "entry", Func);
        // Builder.SetInsertPoint(Entry);
        // CompileBlock(Cast<BlockNode>(Function->Body));
        //
        // llvm::BasicBlock* Bb = Builder.GetInsertBlock();
        //
        // if (!Bb->getTerminator())
        // {
        //     if (RetType->isVoidTy())
        //         Builder.CreateRetVoid();
        //     else
        //         ERROR("Function '" + FuncName + "' must return value");
        // }
        //
        // Functions[FuncName] = Func;
        //
        // return nullptr;

        ERROR("Unsupported operation: function");
    }

    TypedValue LLVMCompiler::CompileReturn(const ReturnNode *Return)
    {
        // if (Return->ReturnValue)
        // {
        //     llvm::Value* RetVal = CompileNode(Return->ReturnValue);
        //     Builder.CreateRet(RetVal);
        // }
        // else
        //     Builder.CreateRetVoid();
        // return nullptr;

        ERROR("Unsupported operation: return");
    }

    TypedValue LLVMCompiler::CompileIf(const IfNode *If)
    {
        // llvm::Value* Cond = CompileNode(If->Condition);
        // Cond = CastToBool(Cond);
        //
        // llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        //
        // auto ThenBB  = llvm::BasicBlock::Create(Context, "then", Func);
        // auto ElseBB  = llvm::BasicBlock::Create(Context, "else", Func);
        // auto MergeBB = llvm::BasicBlock::Create(Context, "ifcont");
        //
        // Builder.CreateCondBr(Cond, ThenBB, ElseBB);
        //
        // Builder.SetInsertPoint(ThenBB);
        // CompileNode(If->Branch);
        // if (!Builder.GetInsertBlock()->getTerminator())
        //     Builder.CreateBr(MergeBB);
        //
        // Builder.SetInsertPoint(ElseBB);
        // if (If->ElseBranch)
        //     CompileNode(If->ElseBranch);
        // if (!Builder.GetInsertBlock()->getTerminator())
        //     Builder.CreateBr(MergeBB);
        //
        // if (!MergeBB->getParent())
        //     MergeBB->insertInto(Func);
        //
        // Builder.SetInsertPoint(MergeBB);
        //
        // return nullptr;

        ERROR("Unsupported operation: if")
    }

    TypedValue LLVMCompiler::CompileWhile(const WhileNode *While)
    {
        // llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        //
        // llvm::BasicBlock* LoopHeader = llvm::BasicBlock::Create(Context, "loop.header", Func);
        // Builder.CreateBr(LoopHeader);
        // Builder.SetInsertPoint(LoopHeader);
        // llvm::Value* Cond = CompileNode(While->Condition);
        // Cond = CastToBool(Cond);
        //
        // llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "loop.body", Func);
        // llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
        // Builder.CreateCondBr(Cond, ThenBB, EndBB);
        //
        // LoopEndStack.push(EndBB);
        // LoopHeaderStack.push(LoopHeader);
        //
        // Builder.SetInsertPoint(ThenBB);
        // CompileNode(While->Branch);
        //
        // LoopEndStack.pop();
        // LoopHeaderStack.pop();
        //
        // if (!Builder.GetInsertBlock()->getTerminator())
        //     Builder.CreateBr(LoopHeader);
        //
        // if (!EndBB->getParent())
        //     EndBB->insertInto(Func);
        //
        // Builder.SetInsertPoint(EndBB);
        // return nullptr;

        ERROR("Unsupported operation: while")
    }

    TypedValue LLVMCompiler::CompileFor(const ForNode *For)
    {
        // llvm::Function* Func = Builder.GetInsertBlock()->getParent();
        //
        // EnterScope();
        //
        // llvm::BasicBlock* InitializationBB = llvm::BasicBlock::Create(Context, "for.initialization", Func);
        // Builder.CreateBr(InitializationBB);
        // Builder.SetInsertPoint(InitializationBB);
        // CompileNode(For->Initialization);
        //
        // llvm::BasicBlock* ForHeader = llvm::BasicBlock::Create(Context, "for.header", Func);
        // Builder.CreateBr(ForHeader);
        // Builder.SetInsertPoint(ForHeader);
        // llvm::Value* Cond = CompileNode(For->Condition);
        // Cond = CastToBool(Cond);
        //
        // llvm::BasicBlock* ThenBB = llvm::BasicBlock::Create(Context, "for.body", Func);
        // llvm::BasicBlock* LatchBB = llvm::BasicBlock::Create(Context, "for.latch", Func);
        // llvm::BasicBlock* EndBB = llvm::BasicBlock::Create(Context, "loop.end");
        // Builder.CreateCondBr(Cond, ThenBB, EndBB);
        //
        // LoopEndStack.push(EndBB);
        // LoopHeaderStack.push(ForHeader);
        //
        // Builder.SetInsertPoint(ThenBB);
        // CompileNode(For->Body);
        //
        // LoopEndStack.pop();
        // LoopHeaderStack.pop();
        //
        // Builder.CreateBr(LatchBB);
        //
        // Builder.SetInsertPoint(LatchBB);
        // CompileNode(For->Iteration);
        // Builder.CreateBr(ForHeader);
        //
        // if (!EndBB->getParent())
        //     EndBB->insertInto(Func);
        // Builder.SetInsertPoint(EndBB);
        //
        // ExitScope();
        // return nullptr;

        ERROR("Unsupported operation: for");
    }

    TypedValue LLVMCompiler::CompileBreak()
    {
        if (LoopEndStack.empty())
            ERROR("'break' used outside loop")

        Builder.CreateBr(LoopEndStack.top());
        return { };
    }

    TypedValue LLVMCompiler::CompileContinue()
    {
        if (LoopHeaderStack.empty())
            ERROR("'continue' used outside loop")

        Builder.CreateBr(LoopHeaderStack.top());
        return { };
    }

    llvm::Type* LLVMCompiler::ToLLVMType(PrimitiveDataType Type)
    {
        switch (Type)
        {
            case PrimitiveDataType::VOID:
                return llvm::Type::getVoidTy(Context);
            case PrimitiveDataType::BOOL:
            case PrimitiveDataType::CHAR:
            case PrimitiveDataType::BYTE:
                return llvm::Type::getInt1Ty(Context);
            case PrimitiveDataType::INT:
                return llvm::Type::getInt32Ty(Context);
            case PrimitiveDataType::LONG:
                return llvm::Type::getInt64Ty(Context);
            case PrimitiveDataType::FLOAT:
                return llvm::Type::getFloatTy(Context);
            case PrimitiveDataType::DOUBLE:
                return llvm::Type::getDoubleTy(Context);
            default:
                ERROR("Unknown data type");
        }
    }

    llvm::Value* LLVMCompiler::CastInteger(llvm::Value* Value, llvm::Type* Target, bool Signed)
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

    llvm::Value* LLVMCompiler::CastToBool(llvm::Value* Value)
    {
        llvm::Type* Type = Value->getType();

        if (Type == Builder.getInt1Ty())
            return Value;

        if (Type->isIntegerTy())
            return Builder.CreateICmpNE(Value, llvm::ConstantInt::get(Type, 0));
        if (Type->isFloatingPointTy())
            return Builder.CreateFCmpONE(Value, llvm::ConstantFP::get(Type, 0.0));
        if (Type->isPointerTy())
            return Builder.CreateICmpNE(Value, llvm::ConstantPointerNull::get(
                llvm::cast<llvm::PointerType>(Type)));

        ERROR("Cannot cast to bool")
    }

    int LLVMCompiler::GetTypeRank(llvm::Type* Type)
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

    llvm::Type* LLVMCompiler::CompileType(const DataTypeNodeBase* Type)
    {
        if (const auto PrimitiveType = Cast<const PrimitiveDataTypeNode>(Type))
            return ToLLVMType(PrimitiveType->PrimitiveType);
        if (const auto PtrType = Cast<const PtrDataTypeNode>(Type))
            return llvm::PointerType::get(CompileType(PtrType->BaseType)->getContext(), 0);

        ERROR("Unsupported type node: " + Type->GetName());
    }

    llvm::AllocaInst* LLVMCompiler::GetLValue(const ASTNode* Node)
    {
        if (const auto Identifier = Cast<const IdentifierNode>(Node))
            return GetVariable(Identifier->Value.ToString());

        return nullptr;
    }

    void LLVMCompiler::CreateDefaultFunction(const std::string &Name, llvm::Type *RetType,
        const llvm::SmallVector<llvm::Type*, 8> &Params)
    {
        llvm::FunctionType* FuncType = llvm::FunctionType::get(RetType, Params, false);
        llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, Name, Module.get());
    }

    TypedValue LLVMCompiler::ImplicitCast(const TypedValue &Value, DataTypeNodeBase *Target)
    {
        // auto SrcType = Cast<PrimitiveDataTypeNode>(Value.Type);
        // if (!SrcType)
        //     ERROR("Cannot cast non-primitive type");
        // auto DstType = Cast<PrimitiveDataTypeNode>(Target);
        // if (!DstType)
        //     ERROR("Cannot cast to non-primitive type");

        return { };
    }
}
//
// Created by bohdan on 19.12.25.
//

#include "Compiler.h"
#include <iostream>
#include <algorithm>

void Compiler::PrintVariables()
{
    for (const auto& [Name, Var] : GlobalVariables)
    {
        std::cout << "Name: " << Name;
        std::cout << " Type: " << MemTypeToString(Var.Type) << " InitialValue: " << Var.InitialValue <<
            " Address: " << Var.Address << std::endl;
    }
}

void Compiler::PrintSection(size_t Tabs) const
{
    size_t n = 0;
    for (auto Inst : *ActiveSection)
    {
        std::cout << n;
        for (size_t i = 0; i < Tabs; i++)
            std::cout << '\t';

        switch (Inst.Code)
        {
            case OpCode::MOV_REG_IMM:
                std::cout << "MOV " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    Inst.Parameters[1] << std::endl;
                break;
            case OpCode::MOV_REG_REG:
                std::cout << "MOV " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::MOV_MEM_REG:
                std::cout << "MOV " << MemTypeToString(static_cast<MemType>(Inst.Parameters[0])) << " " <<
                    "[RBP-" << Inst.Parameters[1] << "], " <<
                        RegisterToString(Inst.Parameters[2]) << std::endl;
                break;
            case OpCode::MOV_REG_MEM:
            {
                auto RefReg = Inst.Parameters[1];
                std::cout << "MOV " << RegisterToString(Inst.Parameters[0]) << ", " <<
                     "[" << RegisterToString(RefReg) << (static_cast<RegType>(RefReg) == RegType::RBP ? "-" : "+") <<
                         Inst.Parameters[2] << "]" << std::endl;
                break;
            }
            case OpCode::PUSH_REG:
                std::cout << "PUSH " << RegisterToString(Inst.Parameters[0]) << std::endl;
                break;
            case OpCode::POP_REG:
                std::cout << "POP " << RegisterToString(Inst.Parameters[0]) << std::endl;
                break;
            case OpCode::ADD_REG_REG:
                std::cout << "ADD " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::ADD_REG_IMM:
                std::cout << "ADD " << RegisterToString(Inst.Parameters[0]) << ", " << Inst.Parameters[1] << std::endl;
                break;
            case OpCode::SUB_REG_REG:
                std::cout << "SUB " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::SUB_REG_IMM:
                std::cout << "SUB " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    Inst.Parameters[1] << std::endl;
                break;
            case OpCode::MUL_REG_REG:
                std::cout << "MUL " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::DIV_REG_REG:
                std::cout << "DIV " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::NEG_REG:
                std::cout << "NEG " << RegisterToString(Inst.Parameters[0]) << std::endl;
                break;
            case OpCode::INC_REG:
                std::cout << "INC " << RegisterToString(Inst.Parameters[0]) << std::endl;
                break;
            case OpCode::DEC_REG:
                std::cout << "DEC " << RegisterToString(Inst.Parameters[0]) << std::endl;
                break;
            case OpCode::JMP:
                std::cout << "JMP " << Inst.Parameters[0] << std::endl;
                break;
            case OpCode::RET:
                std::cout << "RET\n";
                break;
            case OpCode::CALL:
                std::cout << "CALL " << Inst.Parameters[0] << std::endl;
                break;
            case OpCode::CMP_REG_REG:
                std::cout << "CMP " << RegisterToString(Inst.Parameters[0]) << ", " <<
                    RegisterToString(Inst.Parameters[1]) << std::endl;
                break;
            case OpCode::CMP_REG_IMM:
                std::cout << "CMP " << RegisterToString(Inst.Parameters[0]) << ", " << Inst.Parameters[1] << std::endl;
                break;
            case OpCode::JE:
                std::cout << "JE " << Inst.Parameters[0] << std::endl;
                break;
            case OpCode::NOP:
                std::cout << "NOP\n";
                break;
            default:
                std::cout << "UNKNOWN" << std::endl;
                break;
        }

        n++;
    }
}

void Compiler::PrintSections()
{
    std::cout << ".data\n";
    for (const auto& [Name, Var] : GlobalVariables)
    {
        std::cout << "\t";
        std::cout << Name << ": " << MemTypeToString(Var.Type) << " " << Var.InitialValue << std::endl;
    }
    std::cout << ".function\n";
    ActiveSection = &SectionFunction;
    PrintSection(1);
    std::cout << ".text\n";
    ActiveSection = &SectionText;
    PrintSection(1);
}

UInt64 Compiler::CompileNode(ASTNode *Node)
{
    if (!Node)
        return RegToU64(RegType::INVALID);

    if (auto Seq = Cast<SequenceNode>(Node))
        return CompileSequence(Seq);
    if (auto Block = Cast<BlockNode>(Node))
        return CompileBlock(Block);
    if (auto Int = Cast<IntNode>(Node))
        return CompileIntNode(Int);
    if (auto Float = Cast<FloatNode>(Node))
        return CompileFloatNode(Float);
    if (auto Bool = Cast<BoolNode>(Node))
        return CompileBoolNode(Bool);
    if (auto Char = Cast<CharNode>(Node))
        return CompileCharNode(Char);
    if (auto Identifier = Cast<IdentifierNode>(Node))
        return CompileIdentifier(Identifier);
    if (auto Unary = Cast<UnaryOpNode>(Node))
        return CompileUnaryOpNode(Unary);
    if (auto Binary = Cast<BinaryOpNode>(Node))
        return CompileBinaryOpNode(Binary);
    if (auto Call = Cast<CallNode>(Node))
        return CompileCallNode(Call);
    if (auto Subscript = Cast<SubscriptNode>(Node))
        return CompileSubscriptNode(Subscript);
    if (auto Var = Cast<VariableNode>(Node))
        return CompileVariableNode(Var);
    if (auto Function = Cast<FunctionNode>(Node))
        return CompileFunctionNode(Function);
    if (auto Return = Cast<ReturnNode>(Node))
        return CompileReturnNode(Return);
    if (auto If = Cast<IfNode>(Node))
        return CompileIfNode(If);
    if (auto While = Cast<WhileNode>(Node))
        return CompileWhileNode(While);
    if (auto For = Cast<ForNode>(Node))
        return CompileForNode(For);
    if (auto Break = Cast<BreakNode>(Node))
        return CompileBreakNode(Break);
    if (auto Continue = Cast<ContinueNode>(Node))
        return CompileContinueNode(Continue);

    return RegToU64(RegType::INVALID);
}

UInt64 Compiler::CompileSequence(SequenceNode *Node)
{
    UInt64 Last = RegToU64(RegType::INVALID);
    for (auto Statement : Node->Statements)
        Last = CompileNode(Statement);
    return Last;
}

UInt64 Compiler::CompileBlock(BlockNode *Node)
{
    bool OldInGlobalScope = HasFlag(GLOBAL_SCOPE);
    bool OldFunctionBody = HasFlag(FUNCTION_BODY);
    bool OldLoopBody = HasFlag(LOOP_BODY);

    ScopeFlagState<UInt8> BlockState(State, GLOBAL_SCOPE | FUNCTION_BODY, false);
    UInt64 OldLocalDataOffset = LocalDataOffset;

    EnterScope();

    size_t StackStart = ActiveSection->size();
    if (OldInGlobalScope)
    {
        Emit(OpCode::PUSH_REG, static_cast<UInt64>(RegType::RBP));
        Emit(OpCode::MOV_REG_REG, static_cast<UInt64>(RegType::RBP), static_cast<UInt64>(RegType::RSP));
    }
    ScopeState<size_t> AllocStackMemIndexState(AllocStackMemIndex, ActiveSection->size());
    Emit(OpCode::SUB_REG_IMM, static_cast<UInt64>(RegType::RSP), 0);

    UInt64 Last = RegToU64(RegType::INVALID);
    for (auto Statement : Node->Statements)
        Last = CompileNode(Statement);

    if (OldFunctionBody)
    {
        size_t FreeStackMemIndex = ActiveSection->size();
        while (!JumpToFreeStackMemIndexStack.empty())
        {
            size_t Index = JumpToFreeStackMemIndexStack.top();
            JumpToFreeStackMemIndexStack.pop();

            if (Index == FreeStackMemIndex - 1)
            {
                ActiveSection->pop_back();
                FreeStackMemIndex--;
            }
            else
                (*ActiveSection)[Index].Parameters[0] = FreeStackMemIndex;
        }
    }

    size_t BytesAlloc = (*ActiveSection)[StackStart + (OldInGlobalScope ? 2 : 0)].Parameters[1];

    if (OldInGlobalScope)
    {
        Emit(OpCode::MOV_REG_REG, static_cast<UInt64>(RegType::RSP), static_cast<UInt64>(RegType::RBP));
        Emit(OpCode::POP_REG, static_cast<UInt64>(RegType::RBP));
    }
    else
        Emit(OpCode::ADD_REG_IMM, static_cast<UInt64>(RegType::RSP), BytesAlloc);
    // else
    // {
    //     for (size_t i = 0; i < (OldInGlobalScope ? 3 : 1); i++)
    //         (*ActiveSection)[StackStart + i] = Instruction(OpCode::NOP);
    // }

    ExitScope();

    LocalDataOffset = OldLocalDataOffset;

    return Last;
}

UInt64 Compiler::CompileIntNode(IntNode *Node)
{
    UInt64 Reg = AllocReg();
    Emit(OpCode::MOV_REG_IMM, Reg, Node->Value);

    return Reg;
}

UInt64 Compiler::CompileFloatNode(FloatNode *Node)
{
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileCharNode(CharNode *Node)
{
    UInt64 Reg = AllocReg();
    Emit(OpCode::MOV_REG_IMM, Reg, Node->Value);

    return Reg;
}

UInt64 Compiler::CompileBoolNode(BoolNode *Node)
{
    UInt64 Reg = AllocReg();
    Emit(OpCode::MOV_REG_IMM, Reg, Node->Value);

    return Reg;
}

UInt64 Compiler::CompileIdentifier(IdentifierNode *Node)
{
    if (IsCompilingFunction() && CompiledFunction)
    {
        auto Iter = std::find_if(CompiledFunction->Params.begin(),
            CompiledFunction->Params.end(),
            [Node](const std::pair<std::string, UInt64>& Value) -> bool {
                return Value.first == Node->Value;
            });

        if (Iter != CompiledFunction->Params.end())
            return Iter->second;
    }

    if (auto Iter = GlobalVariables.find(Node->Value.ToString()); Iter != GlobalVariables.end())
    {
        UInt64 Reg = AllocReg();
        Emit(OpCode::MOV_REG_MEM, Reg,
            static_cast<UInt64>(RegType::DATA), Iter->second.Address);
        return Reg;
    }

    if (auto Iter = LocalVariables.find(Node->Value.ToString()); Iter != LocalVariables.end())
    {
        UInt64 Reg = AllocReg();
        Emit(OpCode::MOV_REG_MEM, Reg,
            static_cast<UInt64>(RegType::RBP), Iter->second.Offset);
        return Reg;
    }

    return RegToU64(RegType::INVALID);
}

UInt64 Compiler::CompileUnaryOpNode(UnaryOpNode *Node)
{
    UInt64 Reg = CompileNode(Node->Operand);

    OpCode Code;
    switch (Node->Type)
    {
        case Operator::UN_PLS:
            return Reg;
        case Operator::UN_MNS:
            Code = OpCode::NEG_REG;
            break;
        case Operator::INC:
            Code = OpCode::INC_REG;
            break;
        case Operator::DEC:
            Code = OpCode::DEC_REG;
            break;
        default:
            Code = OpCode::UNKNOWN;
            break;
    }

    Emit(Code, Reg);

    return Reg;
}

UInt64 Compiler::CompileBinaryOpNode(BinaryOpNode *Node)
{
    UInt64 LeftReg = CompileNode(Node->Left);
    UInt64 RightReg = CompileNode(Node->Right);

    OpCode Code;
    switch (Node->Type)
    {
        case Operator::ADD:
            Code = OpCode::ADD_REG_REG;
            break;
        case Operator::SUB:
            Code = OpCode::SUB_REG_REG;
            break;
        case Operator::MUL:
            Code = OpCode::MUL_REG_REG;
            break;
        case Operator::DIV:
            Code = OpCode::DIV_REG_REG;
            break;
        case Operator::MOD:
            Code = OpCode::MOD_REG_REG;
            break;
        default:
        {
            UInt64 Reg = CompileEquality(Node->Type, LeftReg, RightReg);
            if (Reg != RegToU64(RegType::INVALID))
            {
                FreeTempReg(RightReg);
                return Reg;
            }

            Code = OpCode::UNKNOWN;
            break;
        }
    }

    Emit(Code, LeftReg, RightReg);
    FreeReg(RightReg);

    return LeftReg;
}

UInt64 Compiler::CompileEquality(Operator::Type Type, UInt64 LeftReg, UInt64 RightReg)
{
    OpCode Code;
    switch (Type)
    {
        case Operator::EQ:
            Code = OpCode::JE;
            break;
        default:
            return RegToU64(RegType::INVALID);
    }

    Emit(OpCode::CMP_REG_REG, LeftReg, RightReg);
    Emit(Code, ActiveSection->size() + 3);
    Emit(OpCode::MOV_REG_IMM, LeftReg, 0);
    Emit(OpCode::JMP, ActiveSection->size() + 2);
    Emit(OpCode::MOV_REG_IMM, LeftReg, 1);

    return LeftReg;
}

UInt64 Compiler::CompileCallNode(CallNode *Node)
{
    if (auto Identifier = Cast<IdentifierNode>(Node->Callee))
    {
        if (auto Iter = Functions.find(Identifier->Value.ToString()); Iter != Functions.end())
        {
            const Function& Func = Iter->second;
            if (Func.Params.size() != Node->Arguments.size()) return RegToU64(RegType::NONE);

            CompiledParamIndex = 0;

            ScopeState<const Function*> CalleeState(Callee, &Func);
            ScopeFlagState<UInt8> CState(State, PARAMETERS);
            for (size_t i = 0; i < Node->Arguments.size(); i++, CompiledParamIndex++)
            {
                const auto& Param = Func.Params[i];
                CompileNode(Node->Arguments[i]);
            }

            Emit(OpCode::CALL, Func.StartIndex);

            for (auto [NewReg, Reg] : SpilledParamRegs)
                Emit(OpCode::MOV_REG_REG, Reg, NewReg);

            SpilledParamRegs.clear();

            return RegToU64(RegType::RAX);
        }
    }

    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileSubscriptNode(SubscriptNode *Node)
{
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileVariableNode(VariableNode *Node)
{
    std::string Name = Node->Name.ToString();
    const DataTypeInfo& TypeInfo = Node->Type->TypeInfo;

    MemType Type;

    switch (TypeInfo.BaseType)
    {
        case DataType::INT:
            Type = MemType::DWORD;
            break;
        case DataType::CHAR:
        case DataType::BOOL:
            Type = MemType::BYTE;
            break;
        default:
            return RegToU64(RegType::NONE);
    }

    if (HasFlag(GLOBAL_SCOPE))
    {
        if (GlobalVariables.contains(Name))
            return RegToU64(RegType::NONE);

        GlobalVariable Var{};

        Var.Type = Type;
        Var.Size = TypeInfo.Size;
        size_t Align = TypeInfo.Align;

        size_t Padding = (Align - (DataOffset % Align)) % Align;

        DataOffset += Padding;

        Var.InitialValue = 0;
        Var.Address = DataOffset;
        DataOffset += Var.Size;

        GlobalVariables[Name] = Var;

        return RegToU64(RegType::NONE);
    }

    if (LocalVariables.contains(Name))
        return RegToU64(RegType::NONE);

    UInt64 Reg = CompileNode(Node->Value);

    LocalVariable Var{};

    Var.Type = Type;
    Var.Size = TypeInfo.Size;
    Var.InitialValue = 0;

    UInt64& LocalVarDataOffset = (*ActiveSection)[AllocStackMemIndex].Parameters[1];

    UInt64 Align = TypeInfo.Align;
    UInt64 Padding = (Align - (LocalVarDataOffset % Align)) % Align;

    LocalVarDataOffset += Padding;
    LocalVarDataOffset += Var.Size;

    LocalDataOffset += LocalVarDataOffset;
    Var.Offset = LocalDataOffset;

    Emit(OpCode::MOV_MEM_REG, static_cast<UInt64>(Type),
        LocalDataOffset, Reg);

    DeclareVar(Name, Var);
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileFunctionNode(FunctionNode *Node)
{
    std::vector<std::pair<std::string, UInt64>> Parameters;
    Parameters.reserve(Node->Params.size());
    for (size_t i = 0; i < Node->Params.size(); i++)
    {
        auto Param = Node->Params[i];
        UInt64 Reg = (i < 4 ? RegToU64(ParamRegs[i]) : i + 4);
        MarkAsUsed(Reg);
        Parameters.emplace_back(Param->Name.ToString(), Reg);
    }

    Functions[Node->Name.ToString()] = { SectionFunction.size(), Parameters };
    ScopeState<const Function*> CompiledFunctionState(
        CompiledFunction, &Functions[Node->Name.ToString()]);
    ScopeState<std::vector<Instruction>*> ActiveSectionState(ActiveSection, &SectionFunction);
    ScopeFlagState<UInt8> FunctionBodyState(State, FUNCTION_BODY);

    CompileBlock(Cast<BlockNode>(Node->Body));
    Emit(OpCode::RET);

    for (const auto& [Name, Reg] : Parameters)
        FreeReg(Reg);

    while (!RegsInFunction.empty())
    {
        FreeTempReg(RegsInFunction.top());
        RegsInFunction.pop();
    }

    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileReturnNode(ReturnNode *Node)
{
    if (Node->ReturnValue)
    {
        UInt64 Reg = CompileNode(Node->ReturnValue);
        if (Reg != RegToU64(RegType::RAX))
            Emit(OpCode::MOV_REG_REG, RegToU64(RegType::RAX), Reg);
    }

    JumpToFreeStackMemIndexStack.push(ActiveSection->size());
    Emit(OpCode::JMP, 0);
    return RegToU64(RegType::RAX);
}

UInt64 Compiler::CompileIfNode(IfNode *Node)
{
    UInt64 CondReg;
    {
        ScopeFlagState<UInt8> ConditionState(State, CONDITION);
        CondReg = CompileNode(Node->Condition);
    }

    Emit(OpCode::CMP_REG_IMM, CondReg, 0);
    size_t JmpToElseIndex = ActiveSection->size();
    Emit(OpCode::JE, 0);
    CompileNode(Node->Branch);
    size_t JmpToEndElseIndex = ActiveSection->size();
    if (Node->ElseBranch)
        Emit(OpCode::JMP, 0);
    (*ActiveSection)[JmpToElseIndex].Parameters[0] = ActiveSection->size();

    if (Node->ElseBranch)
    {
        CompileNode(Node->ElseBranch);
        (*ActiveSection)[JmpToEndElseIndex].Parameters[0] = ActiveSection->size();
    }

    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileWhileNode(WhileNode *Node)
{
    UInt64 CondReg;
    {
        ScopeFlagState<UInt8> ConditionState(State, CONDITION);
        CondReg = CompileNode(Node->Condition);
    }

    size_t CmpIndex = ActiveSection->size();
    Emit(OpCode::CMP_REG_IMM, CondReg, 0);
    size_t JmpToEndLoopIndex = ActiveSection->size();
    Emit(OpCode::JE, 0);
    BreakStack.emplace();
    ContinueStack.emplace();
    CompileNode(Node->Branch);
    auto& ContinueStackTop = ContinueStack.top();
    while (!ContinueStackTop.empty())
    {
        (*ActiveSection)[ContinueStackTop.top()].Parameters[0] = ActiveSection->size();
        ContinueStackTop.pop();
    }
    ContinueStack.pop();

    Emit(OpCode::JMP, CmpIndex);
    (*ActiveSection)[JmpToEndLoopIndex].Parameters[0] = ActiveSection->size();

    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileForNode(ForNode *Node)
{
    CompileNode(Node->Initialization);
    UInt64 CondReg;
    {
        ScopeFlagState<UInt8> ConditionState(State, CONDITION);
        CondReg = CompileNode(Node->Condition);
    }

    size_t CmpIndex = ActiveSection->size();
    Emit(OpCode::CMP_REG_IMM, CondReg, 0);
    size_t JmpToEndLoopIndex = ActiveSection->size();
    Emit(OpCode::JE, 0);
    CompileNode(Node->Body);
    CompileNode(Node->Iteration);
    Emit(OpCode::JMP, CmpIndex);
    (*ActiveSection)[JmpToEndLoopIndex].Parameters[0] = ActiveSection->size();
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileBreakNode(BreakNode *Node)
{
    BreakStack.top().push(ActiveSection->size());
    Emit(OpCode::JMP, 0);
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::CompileContinueNode(ContinueNode *Node)
{
    ContinueStack.top().push(ActiveSection->size());
    Emit(OpCode::JMP, 0);
    return RegToU64(RegType::NONE);
}

UInt64 Compiler::AllocReg()
{
    if (Callee && HasFlag(PARAMETERS))
    {
        UInt64 Reg = Callee->Params[CompiledParamIndex].second;
        if (IsRegUsed(Reg))
            SpillParamReg(Reg);

        return Reg;
    }

    return AllocTempReg();
}

void Compiler::FreeReg(UInt64 Reg)
{
    if (Reg >= UsedRegs.size()) return;
    UsedRegs[Reg] = false;
}

UInt64 Compiler::AllocTempReg()
{
    if (UsedRegs.size() < 8)
        UsedRegs.resize(8);

    for (UInt64 i = 8; i < UsedRegs.size(); i++)
    {
        if (!UsedRegs[i])
        {
            if (IsCompilingFunction())
                RegsInFunction.push(i);

            UsedRegs[i] = true;
            return i;
        }
    }

    UsedRegs.push_back(true);
    RegsInFunction.push(UsedRegs.size() - 1);
    return UsedRegs.size() - 1;
}

void Compiler::FreeTempReg(UInt64 Reg)
{
    if (Reg < 8 || Reg >= UsedRegs.size()) return;
    UsedRegs[Reg] = false;
}

void Compiler::SpillParamReg(UInt64 Reg)
{
    UInt64 NewReg = AllocTempReg();
    Emit(OpCode::MOV_REG_REG, NewReg, Reg);
    SpilledParamRegs.emplace_back(NewReg, Reg);
}

void Compiler::MarkAsUsed(UInt64 Reg)
{
    if (Reg >= UsedRegs.size())
        UsedRegs.resize(Reg + 1);

    UsedRegs[Reg] = true;
}

void Compiler::Emit(OpCode Code, UInt64 Param1, UInt64 Param2, UInt64 Param3)
{
    ActiveSection->emplace_back(Code, Param1, Param2, Param3);
}

void Compiler::DeclareVar(const std::string &Name, LocalVariable Var)
{
    ScopeEntry Entry;
    Entry.Name = Name;

    if (auto Iter = LocalVariables.find(Name); Iter != LocalVariables.end())
    {
        Entry.HadPrevious = true;
        Entry.Previous = Iter->second;
    }
    else
    {
        Entry.HadPrevious = false;
    }

    LocalVariables[Name] = Var;
    ScopeStack.back().push_back(Entry);
}

void Compiler::ExitScope()
{
    for (const auto& Entry : ScopeStack.back())
    {
        if (Entry.HadPrevious)
            LocalVariables[Entry.Name] = Entry.Previous;
        else
            LocalVariables.erase(Entry.Name);
    }

    ScopeStack.pop_back();
}

std::string Compiler::RegisterToString(UInt64 Reg)
{
    if (Reg >= 8 && Reg < RegToU64(RegType::RIP))
        return "R" + std::to_string(Reg);

    switch (U64ToReg(Reg))
    {
        case RegType::RAX:     return "RAX";
        case RegType::RBX:     return "RBX";
        case RegType::RCX:     return "RCX";
        case RegType::RDX:     return "RDX";
        case RegType::RSI:     return "RSI";
        case RegType::RDI:     return "RDI";
        case RegType::RBP:     return "RBP";
        case RegType::RSP:     return "RSP";
        case RegType::RIP:     return "RIP";
        case RegType::DATA:    return "DATA";
        case RegType::NONE:    return "NONE";
        case RegType::INVALID: return "INVALID";
        default:               return "R?";
    }
}

std::string Compiler::MemTypeToString(MemType Mem)
{
    switch (Mem)
    {
        case MemType::BYTE:
            return "BYTE";
        case MemType::DWORD:
            return "DWORD";
        case MemType::QWORD:
            return "QWORD";
        default:
            return "UNKNOWN";
    }
}

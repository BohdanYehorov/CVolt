//
// Created by bohdan on 19.12.25.
//

#ifndef CVOLT_COMPILER_H
#define CVOLT_COMPILER_H

#include <bitset>

#include "Parser.h"
#include "CompilerTypes.h"
#include "DataType.h"
#include <stack>

class Compiler
{
private:
    enum CompilingStateFlag : UInt8
    {
        GLOBAL_SCOPE = 1 << 0,
        FUNCTION_BODY = 1 << 1,
        PARAMETERS = 1 << 2,
        CONDITION = 1 << 3,
        LOOP_BODY = 1 << 4
    };

private:
    static constexpr RegType ParamRegs[] = { RegType::RDI, RegType::RSI, RegType::RDX, RegType::RCX };

private:
    ASTNode* ASTTree;

    std::vector<Instruction> SectionText;
    std::vector<Instruction> SectionFunction;
    std::vector<Instruction>* ActiveSection = &SectionText;

    std::unordered_map<std::string, GlobalVariable> GlobalVariables;
    std::unordered_map<std::string, LocalVariable> LocalVariables;
    std::vector<std::vector<ScopeEntry>> ScopeStack;

    std::unordered_map<std::string, Function> Functions;
    std::stack<UInt64> RegsInFunction;

    std::vector<Instruction> InstructionsToEndBlock;

    std::vector<bool> UsedRegs;
    std::vector<std::pair<UInt64, UInt64>> SpilledParamRegs;

    std::stack<size_t> JumpToFreeStackMemIndexStack;
    std::stack<std::stack<size_t>> BreakStack;
    std::stack<std::stack<size_t>> ContinueStack;

    UInt8 State = GLOBAL_SCOPE;
    UInt64 DataOffset = 0;
    UInt64 LocalDataOffset = 0;
    size_t AllocStackMemIndex = 0;

    const Function* CompiledFunction = nullptr;
    const Function* Callee = nullptr;
    size_t CompiledParamIndex = 0;

public:
    Compiler(const Parser& Psr) : ASTTree(Psr.GetASTTree()), UsedRegs(8, false) {}

    void Compile() { CompileNode(ASTTree); }

    void PrintVariables();
    void PrintSection(size_t Tabs) const;
    void PrintSections();

private:
    UInt64 CompileNode(ASTNode *Node);

    UInt64 CompileSequence(SequenceNode *Node);
    UInt64 CompileBlock(BlockNode *Node);
    UInt64 CompileIntNode(IntNode *Node);
    UInt64 CompileFloatNode(FloatNode *Node);
    UInt64 CompileCharNode(CharNode *Node);
    UInt64 CompileBoolNode(BoolNode *Node);
    UInt64 CompileIdentifier(IdentifierNode *Node);
    UInt64 CompileUnaryOpNode(UnaryOpNode *Node);
    UInt64 CompileBinaryOpNode(BinaryOpNode *Node);
    UInt64 CompileEquality(Operator::Type Type, UInt64 LeftReg, UInt64 RightReg);
    UInt64 CompileCallNode(CallNode *Node);
    UInt64 CompileSubscriptNode(SubscriptNode *Node);
    UInt64 CompileVariableNode(VariableNode *Node);
    UInt64 CompileFunctionNode(FunctionNode *Node);
    UInt64 CompileReturnNode(ReturnNode *Node);
    UInt64 CompileIfNode(IfNode *Node);
    UInt64 CompileWhileNode(WhileNode *Node);
    UInt64 CompileForNode(ForNode *Node);
    UInt64 CompileBreakNode(BreakNode *Node);
    UInt64 CompileContinueNode(ContinueNode *Node);

    UInt64 AllocReg();
    void FreeReg(UInt64 Reg);

    UInt64 AllocTempReg();
    void FreeTempReg(UInt64 Reg);

    void SpillParamReg(UInt64 Reg);
    void MarkAsUsed(UInt64 Reg);

    void Emit(OpCode Code, UInt64 Param1 = 0, UInt64 Param2 = 0, UInt64 Param3 = 0);
    void DeclareVar(const std::string& Name, LocalVariable Var);
    void EnterScope() { ScopeStack.emplace_back(); }
    void ExitScope();

    static std::string RegisterToString(UInt64 Reg);
    static std::string MemTypeToString(MemType Mem);

    [[nodiscard]] bool HasFlag(CompilingStateFlag Flag) const { return State & Flag; }
    void ClearFlag(CompilingStateFlag Flag) { State &= ~Flag; }

    [[nodiscard]] static constexpr RegType U64ToReg(UInt64 Num) noexcept { return static_cast<RegType>(Num); }
    [[nodiscard]] static constexpr UInt64 RegToU64(RegType Reg) noexcept { return static_cast<UInt64>(Reg); }

    [[nodiscard]] static constexpr bool IsTempReg(UInt64 Reg) noexcept { return Reg > RegToU64(RegType::RSP) &&
        Reg < RegToU64(RegType::RIP); }
    [[nodiscard]] bool IsRegUsed(UInt64 Reg) const { return Reg < UsedRegs.size() && UsedRegs[Reg]; }

    [[nodiscard]] bool IsCompilingFunction() const { return ActiveSection == &SectionFunction; }
};

#endif //CVOLT_COMPILER_H
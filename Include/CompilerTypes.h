//
// Created by bohdan on 21.12.25.
//

#ifndef CVOLT_INSTRUCTION_H
#define CVOLT_INSTRUCTION_H

#include "TypeDefs.h"
#include <string>
#include <limits>
#include <vector>

constexpr UInt64 EndReg = std::numeric_limits<UInt64>::max();

enum class OpCode : UInt8
{
    MOV_REG_REG,
    MOV_REG_IMM,
    MOV_MEM_REG,
    MOV_REG_MEM,

    SUB_REG_IMM,
    ADD_REG_IMM,

    SUB_REG_REG,
    ADD_REG_REG,
    MUL_REG_REG,
    DIV_REG_REG,
    MOD_REG_REG,

    INC_REG,
    DEC_REG,
    NEG_REG,

    PUSH_REG,
    POP_REG,

    JMP,
    JE,
    JNE,
    JZ,
    JNZ,
    CALL,
    RET,

    CMP_REG_REG,
    CMP_REG_IMM,

    NOP,
    HALT,

    UNKNOWN
};

enum class RegType : UInt64
{
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RBP,
    RSP,

    RIP = EndReg - 3,
    DATA = EndReg - 2,
    NONE = EndReg - 1,
    INVALID = EndReg
};

enum class MemType : UInt8
{
    BYTE,
    DWORD,
    QWORD
};

struct Instruction
{
    OpCode Code;
    UInt64 Parameters[3];

    Instruction() = default;
    Instruction(OpCode Code, UInt64 Param1 = 0, UInt64 Param2 = 0, UInt64 Param3 = 0)
        : Code(Code), Parameters{ Param1, Param2, Param3 } {}
};

struct GlobalVariable
{
    MemType Type;
    UInt64 Size;
    UInt64 InitialValue;
    UInt64 Address;
};

struct LocalVariable
{
    MemType Type;
    UInt64 Size;
    UInt64 InitialValue;
    UInt64 Offset;
};

struct ScopeEntry
{
    std::string Name;
    bool HadPrevious;
    LocalVariable Previous;
};

struct Function
{
    size_t StartIndex;
    std::vector<std::pair<std::string, UInt64>> Params;
};

template <typename T>
class ScopeState
{
private:
    T& Current;
    T Prev;

public:
    ScopeState(T& Current, const T& New)
        : Current(Current), Prev(Current)
    {
        Current = New;
    }

    ~ScopeState()
    {
        Current = Prev;
    }
};

template <typename T>
class ScopeFlagState
{
private:
    T& Current;
    T Prev;

public:
    ScopeFlagState(T& Current, const T NewFlag, bool Checked = true)
        : Current(Current), Prev(Current)
    {
        Current = static_cast<T>(Checked ? Current | NewFlag : Current & ~NewFlag);
    }

    ~ScopeFlagState()
    {
        Current = Prev;
    }
};

#endif //CVOLT_INSTRUCTION_H
#include "global.hpp"
#include "organization.hpp"

/*************************
*  ALU
*************************/

int32_t ALU::signal(ALU_OP op, int32_t operand1, int32_t operand2)
{
  switch (op)
  {
    case ADD:
      return operand1 + operand2;
    case SUB:
      return operand1 - operand2;
    case MUL:
      return operand1 * operand2;
    case DIV:
      return operand1 / operand2;
    case LT:
      return operand1 < operand2;
    case EQ:
      return operand1 == operand2;
  }
}

/*************************
*  Memory and register
*************************/

int32_t Register::read()
{
  return value;
}

void Register::write(int32_t x)
{
  value = x;
}

template <uint32_t MEMSIZE>
int32_t Memory<MEMSIZE>::load(uint32_t index)
{
  return slots[index];
}

template <uint32_t MEMSIZE>
void Memory<MEMSIZE>::store(uint32_t index, int32_t param)
{
  slots[index] = param;
}

/*************************
*  Instruction memory / PC
*************************/

InstructionMemory::InstructionMemory(const uint32_t* new_instructions)
{
  instructions = new_instructions;
}

void InstructionMemory::load(const uint32_t index)
{
  instruction = instructions[index];
}

const uint32_t InstructionMemory::read()
{
  return instruction;
}

OPCODE InstructionMemory::opCode()
{
  byte* bitfield = reinterpret_cast<byte*>(&instruction);
  return (OPCODE)data_from_bitfield<uint32_t,0,8>(bitfield);
}

REGISTER InstructionMemory::regOut()
{
  byte* bitfield = reinterpret_cast<byte*>(&instruction);
  return (REGISTER)data_from_bitfield<uint32_t,0+8,8+8>(bitfield);
}

REGISTER InstructionMemory::regIn1()
{
  byte* bitfield = reinterpret_cast<byte*>(&instruction);
  return (REGISTER)data_from_bitfield<uint32_t,0+16,8+16>(bitfield);
}

REGISTER InstructionMemory::regIn2()
{
  byte* bitfield = reinterpret_cast<byte*>(&instruction);
  return (REGISTER)data_from_bitfield<uint32_t,0+24,8+24>(bitfield);
}

int32_t InstructionMemory::immediate()
{
  byte* bitfield = reinterpret_cast<byte*>(&instruction);
  return data_from_bitfield<int32_t,0+24,8+24>(bitfield);
}

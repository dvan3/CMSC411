#ifndef ORG
#define ORG

// OPCODE, TARGET REGISTER INDEX, REGISTER INDEX 1, REGISTER INDEX 2 OR IMMEDIATE

#include <iostream>
#include "global.hpp"
#include "codes.hpp"

class ALU
{
  public:
    int32_t signal(ALU_OP op, int32_t operand1, int32_t operand2);
};

/*************************
*  Memory and register
*************************/

class Register
{
  public:
    int32_t read();
    void write(int32_t x);
  private:
    int32_t value;
};

template <uint32_t MEMSIZE>
class Memory
{
  public:
    int32_t load(uint32_t index);
    void store(uint32_t index, int32_t param);
  private:
    uint32_t slots[MEMSIZE];
};

/*************************
*  Instruction memory / PC
*************************/

class InstructionMemory
{
  public:
    InstructionMemory(const uint32_t* new_instructions);
    void load(uint32_t index);
    const uint32_t read();
    OPCODE opCode();
    REGISTER regOut();
    REGISTER regIn1();
    REGISTER regIn2();
    int32_t immediate();

  private:
    const uint32_t* instructions;
    uint32_t instruction;
    Register* registers;
};


/*************************
*  Pipeline caching
*************************/

struct PipelineCache
{
  // Not all pipeline caches need all of these
  // registers, but it's easier to glom together.

  PipelineCache() :
    stall(false),
    valid(false),
    opCode((OPCODE)0),
    target((REGISTER)0),
    reg1(0),
    reg2(0),
    immediate(0),
    result(0),
    mem(0),
    forward(false)
    {}

  // organization
  bool stall;
  bool valid;

  // fetch
  uint32_t pc;

  // register
  int32_t reg1;
  int32_t reg2;
  int32_t immediate;
  OPCODE opCode;
  REGISTER target;

  // execute
  int32_t result;

  // memory
  int32_t mem;

  // forwarding
  bool forward;
};

#endif // organization include guard

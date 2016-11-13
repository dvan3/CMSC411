#ifndef CODES_HPP
#define CODES_HPP

// This file contains all of the enumerations
// for opcodes, registers, etc.  If this changes,
// it breaks old compiles, so instructions will
// need to be recompiled.  (So VERSION THEM.)
// am, Apr 29 2012

/*************************
*  opcodes
*************************/
enum OPCODE {
  // organizational
  NOOP = 0,
  // registers
  SET_R,    SET_I,
  // arithmetic
  ADD_R,    ADD_I,
  SUB_R,    SUB_I,
  MUL_R,    MUL_I,
  DIV_R,    DIV_I,
  // memory
  STORE_R,  STORE_I,
  LOAD_R,   LOAD_I,
  // branching
  BEQ_R,    BLT_R,  // conditional
  JMP_R,    JMP_I, // absolute
  // robot operation
  DRIVE_R,  DRIVE_I,
  TURN_R,   TURN_I,
  FIRE_R,   FIRE_I,
  SCAN
};

enum REGISTER {
  PC,
  OP1,
  OP2,
  RESULT,
  SCANRANGE,
  FIRERANGE,
  DRIVESPEED,
  TURNSPEED,
  R0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  N_REGISTERS
};

/*************************
*  ALU
*************************/

enum ALU_OP {
  ADD,
  SUB,
  MUL,
  DIV,
  LT,
  EQ
};

#endif // #IFDEF CODES_HPP

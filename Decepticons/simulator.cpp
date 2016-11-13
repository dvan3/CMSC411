// libs
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
// our files
#include "organization.hpp"

// Set DEBUG to do a no-op if you want to conform to project spec.
#define DEBUG(statement) (println(statement));

/********************
* Convenience fn's
********************/

template <typename T>
void println(T line)
{
  // Intentional no-op right now.
  //std::cout << line << std::endl;
}

/********************
* Main loop
********************/

int main(int argc, const char** argv)
{
  // We should have one arg -- a filename,
  // which contains our opcodes.
  if (argc != 2 /* 1 arg + executable name */)
  {
    println("Usage:  ./simulator [path to instructions]");
    return 0;
  }
  const char* filename = *(argv+1); 

  // Read everything into a buffer.
  const int BUFSIZE = (1024*1024)/sizeof(uint32_t); // assuming <= 1 MB of instructions, our AI isn't THAT prolific.
  uint32_t instruction_buffer[BUFSIZE];
  {
    FILE *f;
    f = fopen(filename, "rb");
    if (f)
        fread(instruction_buffer, BUFSIZE, 1, f);
    else
    {
      println("Couldn't open file.");
      return 1;
    }
    DEBUG("Successfully read instructions.");
    fclose(f);
  }

  DEBUG("Initializating hardware...");
  
  // We now have everything we need to construct our simulation.
  // Create our organization and set our instruction memory, along
  // with any other defaults.
  InstructionMemory instructionMem = InstructionMemory(instruction_buffer);

  // Initialize data registers
  // Double-buffered (read from prev, write to next, swap at end of cycle)
  Register* prevRegisterBank = new Register[N_REGISTERS];
  Register* nextRegisterBank = new Register[N_REGISTERS];
  prevRegisterBank[PC].write(0);

  // Initialize pipeline caches
  // Double-buffered (read from prev, write to next, swap at end of cycle)
  PipelineCache* prevPLState = new PipelineCache[4];
  PipelineCache* nextPLState = new PipelineCache[4];
  enum Pipeline {
    FETCH, REGISTER, EXECUTION, MEMORY
  };

  // Initialize memory
  Memory<1024> dataMem = Memory<1024>();

  // Initialize ALUs
  // For math operations
  ALU mainALU = ALU();
  // For incrementing the PC
  ALU pcALU = ALU();
  // Fetch phase needs to add 2 to the PC
  // to adjust for the delay that the
  // execution phase has in receiving it
  ALU fetchALU = ALU();

  DEBUG("Hardware initialized.");
  DEBUG("Preparing for first tick.");
  prevPLState[FETCH].valid = true;
  nextPLState[FETCH].valid = true;

  // Clock cycle
  uint32_t cycle = 0;
  std::string line;
  while (getline(std::cin, line), ++cycle) // we can safely stop at 2^32-1 cycles, ok?
  {
    char buffer[128];
    sprintf(buffer, "\n === BEGINNING OF CYCLE [ %i ] ", cycle);
    DEBUG(buffer);

    /****

      What happens each clock:

      1)  Simulate each segment of the pipeline.

          This is done serially, but as far as we're concerned,
          they're simultaneous.

          We simulate in reverse order, as each segment depends on the
          pipeline registers (and possibly forwarding) of the previous
          segment, but not the subsequent segment.

      2)  Detect hazards (and destroy obsolete work, in the case of branching)

          Each segment can look at the instructions of the previous segments,
          and has the responsibility to detect structural hazards.

          Segments can have their 'stall bit' set.  If so, instead of operating,
          they spend the cycle clearing that stall bit.  (In terms of control,
          we're locking out writes to the segment caches, but that level of fidelity
          isn't simulated.)

      3)  Send commands to the robot hardware (run a special robot operation)

          Robot operations must happen ASAP, so they output as soon as they can.
          They run with _whatever the register is at the BEGINNING of the cycle_.
          This is simulated by just saving to a variable for later, as the registers
          are updated before we get to the instruction decode.

      Structural hazards:
      
        - more than 2 register reads / cycle
        - more than 1 register write / cycle
        - more than 1 memory access/cycle
        - simultaneous robot operations (not a concern of our pipeline)

    ****/

    /***********
    * Aliases
    ***********/
    
    PipelineCache* prev;  // Reading from the previous stage's cache (written last cycle, read this cycle)
    PipelineCache* next;  // Writing to the next stage's cache (written this cycle, read next cycle)
    PipelineCache* me;    // For unsetting the stall bit.  (It'd be nice if C++ supported closures...)

    /*****************************
    * Memory segment ( memory load/store, & write to registers)
    *****************************/

    DEBUG("Resolving memory segment...");
    prev = &prevPLState[MEMORY];
    me   = &nextPLState[MEMORY];
    // Assume we can't forward, then if
    // we write to a register, say that
    // we can, so the register segment
    // knows to read from us, instead
    // of from the register.
    me->forward = false;
    // We can only forward if
    // our target is their source.
    me->target = prev->target;
    if (!prev->stall && prev->valid)
    {
      switch(prev->opCode)
      {
        case SET_R:
          DEBUG("  SET_R");
          nextRegisterBank[ prev->target ].write( prev->reg1 );
          me->forward = true;
          me->result  = prev->reg1;
          break;
        case SET_I:
          DEBUG("  SET_I");
          nextRegisterBank[ prev->target ].write( prev->immediate );
          me->forward = true;
          me->result  = prev->immediate;
          break;
        case SCAN:
          DEBUG("  SCAN");
          nextRegisterBank[ prev->target ].write( prev->result );
          me->forward = true;
          me->result  = prev->result;
        case ADD_R:
        case ADD_I:
        case SUB_R:
        case SUB_I:
        case MUL_R:
        case MUL_I:
        case DIV_R:
        case DIV_I:
          DEBUG("  MATH");
          nextRegisterBank[ prev->target ].write( prev->result );
          me->forward = true;
          me->result  = prev->result;
          break;
        case LOAD_I:
          DEBUG("  LOAD_I");
          nextRegisterBank[ prev->target ].write( dataMem.load( (uint32_t) prev->immediate ) );
          me->forward = true;
          me->result = prev->immediate;
          break;
        case LOAD_R:
          DEBUG("  LOAD_R");
          nextRegisterBank[ prev->target ].write( dataMem.load( (uint32_t) prev->reg1 ) );
          me->forward = true;
          me->result = prev->reg1;
          break;
        case STORE_I:
          DEBUG("  STORE_I");
          dataMem.store( prev->reg1, prev->immediate );
          break;
        case STORE_R:
          DEBUG("  STORE_R");
          dataMem.store( prev->reg1, prev->reg2 );
          break;
        default:
          DEBUG("  Instruction does not interact with memory or write to a register.");
      };

    } else DEBUG("  Memory segment has been signalled to stall.");
    me->stall = false;


    /*****************************
    * Execute segment
    *****************************/

    DEBUG("Resolving execution segment...")
    prev = &prevPLState[EXECUTION];
    next = &nextPLState[MEMORY];
    me   = &nextPLState[EXECUTION];
    if (!prev->stall && prev->valid)
    {
      // How this segment happens:
      //
      // Arithmetic portion:
      // 1)  Read in ALU inputs (from register or immediate)
      // 2)  Set ALU control
      // 3)  Write ALU output to pipeline cache
      //
      // Branching portion:
      // 1)  Read in second ALU input
      // 2)  Set ALU control
      // 3)  Write ALU output to program counter

      char buffer[128];
      sprintf(buffer, "  Executing opcode %i.", prev->opCode );
      DEBUG( buffer );

      // All of these default initializations exist just to shut the compiler up.
      // If we don't use the ALU output, the output is trash, and we don't care.
      ALU_OP operation = ADD;
      int32_t op1 = 0;
      int32_t op2 = 0;

      /*****************************
      * Robot commands (non-ALU)
      *****************************/
      switch (prev->opCode)
      {
        // from immediate
        case FIRE_I:
          std::cout << "F " << prev->immediate << std::endl;
          break;
        case DRIVE_I:
          std::cout << "M " << prev->immediate << std::endl;
          break;
        case TURN_I:
          std::cout << "R " << prev->immediate << std::endl;
          break;

        // from register
        case FIRE_R:
          std::cout << "F " << prev->reg1 << std::endl;
          break;
        case DRIVE_R:
          std::cout << "M " << prev->reg1 << std::endl;
          break;
        case TURN_R:
          std::cout << "R " << prev->reg1 << std::endl;
          break;

        case SCAN:
          std::cout << "S " << std::endl;
          getline(std::cin,line);
          std::istringstream(line) >> next->result;
          break;
        // noop
        default:
          std::cout << "N" << std::endl;
          break;
      };

      /*****************************
      * Arithmetic
      *****************************/

      // Set ALU inputs
      switch (prev->opCode)
      {
        case ADD_R:
        case SUB_R:
        case MUL_R:
        case DIV_R:
          DEBUG("  R-type ALU instruction.")
          op1 = prev->reg1;
          op2 = prev->reg2;
          break;
        default:
          DEBUG("  Not R-type ALU instruction -- either I-type, or result not used.")
          op1 = prev->reg1;
          op2 = prev->immediate;
          break;
      }

      // Set ALU operation
      switch (prev->opCode)
      {
        case ADD_R:
        case ADD_I:
          DEBUG("  ADD instruction.")
          operation = ADD;
          break;
        case SUB_R:
        case SUB_I:
          DEBUG("  SUB instruction.")
          operation = SUB;
          break;
        case MUL_R:
        case MUL_I:
          DEBUG("  MUL instruction.")
          operation = MUL;
          break;
        case DIV_I:
        case DIV_R:
          DEBUG("  DIV instruction.")
          operation = DIV;
          break;
      }

      sprintf(buffer, "  Math ALU inputs %i, %i = %i ", op1, op2, mainALU.signal(operation, op1, op2) );
      DEBUG(buffer);
      if (prev->opCode != SCAN)
        next->result = mainALU.signal(operation, op1, op2);
      
      /*****************************
      * Branching
      *****************************/

      // Determine ALU operation.
      // For conditionals, we must use the ALU to determine whether we branch,
      //   as using one ALU output as another ALU's input would require 2 cycles.
      // For jumps and regular operation, we (may) use the ALU to increment the program counter.
      uint32_t pc_destination;
      switch (prev->opCode)
      {
        case BLT_R:
          DEBUG("  BLT (set PC to immediate if one register > other) instruction.");
          op1 = prev->reg1;
          op2 = prev->reg2;
          operation = LT;
          break;

        case BEQ_R:
          DEBUG("  BEQ (set PC to immediate if two registers are equal) instruction.");
          op1 = prev->reg1;
          op2 = prev->reg2;
          operation = EQ;
          break;

        case JMP_I:
          DEBUG("  JMP (set PC to immediate) instruction.");
          pc_destination = prev->immediate;
          operation = ADD; // don't care
          break;

        case JMP_R:
          DEBUG("  JMP (set PC to register) instruction.");
          pc_destination = prev->reg1;
          operation = ADD; // don't care
          break;

        default:
          DEBUG("  Opcode isn't a branch.  Incrementing PC by 1.");
          op1 = 1;
          op2 = prev->pc;
          operation = ADD;
          break;
      }

      // Branch instructions have to invalidate the pipeline.
      // We force a stall for this pipeline, and then set
      // phases to invalid.
      switch (prev->opCode)
      {
        case BLT_R:
        case BEQ_R:
        case JMP_I:
        case JMP_R:
          DEBUG("  Branching -- invalidating previous pipeline stages.");
          // The next instruction we would execute is unreliable, so
          // we throw it out.  We mark the next 2 instructions (in the
          // register segment and in the fetch segment) as invalid,
          // so that we do not execute them.
          // Next cycle, the PC will be set to the correct branch,
          // so we can restart and flush validity back in.
          prevPLState[REGISTER].stall = true;
          prevPLState[FETCH].stall = true;
          nextPLState[EXECUTION].valid = false;
          nextPLState[REGISTER].valid = false;
          nextPLState[FETCH].valid = false;

          break;
      }

      // Run the ALU and update the PC.
      // (Some instructions use ALU to determine whether we branch,
      // others use it to determine the PC.)
      uint32_t result = pcALU.signal(operation, op1, op2);
      // If we're branching, we haven't used the math ALU.
      // We use the math ALU as a backup, to increment the PC,
      // in case a conditional branch fails and we still need
      // to do a PC++.
      uint32_t incremented_pc = mainALU.signal(ADD, prev->pc, 1);
      switch (prev->opCode)
      {
        // ALU as conditional
        case BLT_R:
        case BEQ_R:
          if (result)
          {
            sprintf(buffer, "  PC updating from %i to %i.", prev->pc, prev->target);  // target is treated as an immediate here (kludgy...)
            DEBUG(buffer);
            nextRegisterBank[PC].write(
              prev->target
              );
          } else {
            sprintf(buffer, "  PC updating from %i to %i.", prev->pc, incremented_pc);
            DEBUG(buffer);
            nextRegisterBank[PC].write(
              incremented_pc
              );
          }
          break;

        case JMP_R:
          sprintf(buffer, "  PC updating from %i to %i.", prev->pc, prev->reg1); // JMP_R jumps to value in reg1
          DEBUG(buffer);
          nextRegisterBank[PC].write(
            prev->reg1
            );
          break;

        case JMP_I:
          sprintf(buffer, "  PC updating from %i to %i.", prev->pc, prev->immediate);
          DEBUG(buffer);
          nextRegisterBank[PC].write(
            prev->immediate 
            );
          break;

        // ALU as target
        default:
          sprintf(buffer, "  PC updating from %i to %i.", prev->pc, result);
          DEBUG(buffer);
          nextRegisterBank[PC].write(
            result
            );
          break;
      }


      /*****************************
      * Pass through values to pipeline cache
      *****************************/

      next->opCode      = prev->opCode;
      next->target      = prev->target;
      next->immediate   = prev->immediate;
      next->reg1        = prev->reg1;
      next->valid       = true;

    } else {
      DEBUG("  Execution segment has been signalled to stall.");

      // no operation this cycle
      std::cout << "N" << std::endl;

      // If we detect that the execution pipeline didn't run
      // (due to a pipeline flush on a branch, or when we start)
      // we have a backup bootstrap ALU to increment our PC.
      DEBUG("  PC incremented by backup execution phase.");
      nextRegisterBank[PC].write(
          pcALU.signal( ADD, 1, prevRegisterBank[PC].read() )
          );
    }
    me->stall = false;

    /*****************************
    * Register read segment
    *****************************/

    DEBUG("Resolving register read segment...")
    prev = &prevPLState[REGISTER];
    next = &nextPLState[EXECUTION];
    me   = &nextPLState[REGISTER];
    if (!prev->stall && prev->valid)
    {
      // Decode the instruction
      next->opCode      = instructionMem.opCode();
      next->target      = instructionMem.regOut();
      next->immediate   = instructionMem.immediate();

      // Only perform a read from the register when necessary:
      switch (instructionMem.opCode())
      {
        // All instructions that read 2 registers:
        case ADD_R:
        case SUB_R:
        case MUL_R:
        case DIV_R:
        case STORE_R:
        case LOAD_R:
        case BEQ_R:
        case BLT_R:
          DEBUG("  Reading from instruction's 2nd register index.");
          if (nextPLState[MEMORY].forward && nextPLState[MEMORY].target == next->target)
          {
            DEBUG("  Forwarding from memory writeback segment!");
            next->reg2    = nextPLState[MEMORY].result;
          }
          else
            next->reg2    = prevRegisterBank[instructionMem.regIn2()].read();
        // All instructions that read 1 register:
        case SET_R:
        case ADD_I:
        case SUB_I:
        case MUL_I:
        case JMP_R:
        case DRIVE_R:
        case TURN_R:
        case FIRE_R:
        case DIV_I:
          DEBUG("  Reading from instruction's 1st register index.");
          if (nextPLState[MEMORY].forward && nextPLState[MEMORY].target == next->target)
          {
            DEBUG("  Forwarding from memory writeback segment!");
            next->reg2    = nextPLState[MEMORY].result;
          } else
            next->reg1    = prevRegisterBank[instructionMem.regIn1()].read();
          break;
        default:
          DEBUG("  Operation doesn't require a register read.");
      }

      // forward the previous caches
      next->pc    = prev->pc;
      next->valid = true;

      // If the execution segment is holding an operation that will be written
      // to a register that we are trying to read, then we need to repeat
      // our read, so that we can get the result forwarded from the memory
      // segment.
      // We can repeat our read by stalling the instruction fetch stage.
      // The pipeline cache doesn't update, so we reread the same instruction
      // as before -- this time, detecting the forward from memory.
      // We also don't want to feed BS to the execution stage, so we invalidate it.
      // TODO:  Push these out to separate functions; this is ugly code.
      switch (instructionMem.opCode())
      {
        // All instructions that read 2 registers:
        case ADD_R:
        case SUB_R:
        case MUL_R:
        case DIV_R:
        case STORE_R:
        case LOAD_R:
        case BEQ_R:
        case BLT_R:
          switch(prevPLState[EXECUTION].opCode)
          {
            case SET_R:
            case SET_I:
            case ADD_R:
            case ADD_I:
            case SUB_R:
            case SUB_I:
            case MUL_R:
            case MUL_I:
            case DIV_R:
            case DIV_I:
            case LOAD_I:
            case LOAD_R:
            case SCAN:
              if (prevPLState[EXECUTION].target == instructionMem.regIn1() || prevPLState[EXECUTION].target == instructionMem.regIn2())
              {
                DEBUG("  Data hazard detected; stalling new instruction to repeat register read.");
                prevPLState[FETCH].stall = true;
                next->opCode = NOOP;  // We need to flush the opcode, or else we'll redetect a false hazard.
                next->stall = true;
                // Simulation hack... unsimulating the execution phase's PC increment.
                nextRegisterBank[PC].write(  nextRegisterBank[PC].read() - 1 );
                //nextPLState[REGISTER].pc--;
              }
          };
          break;
        // All instructions that read 1 register:
        case SET_R:
        case ADD_I:
        case SUB_I:
        case MUL_I:
        case JMP_R:
        case DRIVE_R:
        case TURN_R:
        case FIRE_R:
        case DIV_I:
          switch(prevPLState[EXECUTION].opCode)
          {
            case SET_R:
            case SET_I:
            case ADD_R:
            case ADD_I:
            case SUB_R:
            case SUB_I:
            case MUL_R:
            case MUL_I:
            case DIV_R:
            case DIV_I:
            case LOAD_I:
            case LOAD_R:
              if (prevPLState[EXECUTION].target == instructionMem.regIn1())
              {
                DEBUG("  Data hazard detected; stalling new instruction to repeat register read.");
                prevPLState[FETCH].stall = true;
                next->opCode = NOOP;  // We need to flush the opcode, or else we'll redetect a false hazard.
                next->stall = true;
                // Simulation hack... unsimulating the execution phase's PC increment.
                nextRegisterBank[PC].write(  nextRegisterBank[PC].read() - 1 );
                //nextPLState[REGISTER].pc--;
              }
          };
          break;
      }

      
    } else DEBUG("  Register read segment has been signalled to stall.");
    me->stall = false;

    /*****************************
    * Instruction fetch segment
    *****************************/

    DEBUG("Resolving instruction fetch segment...");
    prev = &prevPLState[FETCH];
    next = &nextPLState[REGISTER];
    me   = &nextPLState[FETCH];
    if (!prev->stall && prev->valid) 
    {
      char buffer[128];
      sprintf(buffer, "  Current PC:  %i", prevRegisterBank[PC].read() );
      DEBUG(buffer);

      // Load from current address
      instructionMem.load( prevRegisterBank[PC].read() );

      // We fake out the execution phase -- any PC it reads is from 2 cycles ago,
      // so we need to tell it that we're 2 in the future.
      // It may be the case that we've obtained our PC from the bootstrap ALU
      next->pc          = fetchALU.signal( ADD, 2, prevRegisterBank[PC].read() );
      next->valid       = true; 

      sprintf(buffer, "  Instruction that was read: 0x%X", instructionMem.read() );
      DEBUG(buffer)

    } else DEBUG("  Instruction fetch segment has been signalled to stall.");
    
    me->stall = false;


    /*****************************
    * Post-cycle
    *****************************/

    // If the pipeline was invalidated, we signal that the
    // instruction fetch phase can restart.
    nextPLState[FETCH].valid = true;

    // State-tracking
    // Copy next buffers to prev
    memcpy( (void*)prevPLState, (void*)nextPLState, sizeof(PipelineCache)*4);
    memcpy( (void*)prevRegisterBank, (void*)nextRegisterBank, sizeof(Register)*N_REGISTERS);
    
  };

  // Mem cleanup
  delete[] prevPLState;
  delete[] nextPLState;
  delete[] prevRegisterBank;
  delete[] nextRegisterBank;

  // Realistically, we don't reach this point
  // as our simulation process is killed.
  return 0;
}


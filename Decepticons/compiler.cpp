//Filename: complier.cpp
//CMSC 411
//Group Project: Decepticons

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

using namespace std;

int main(int argc, char * argv[]){

  //checking command arguments
  if(argc != 2){
    cout << "Syntax error: usage ./compile <filename>" << endl;
  }
  
  //map for opCodes
  map<string,unsigned int> opCode;
  opCode["NOOP"] = 0;
  opCode["SET_R"] = 1;
  opCode["SET_I"] = 2;
  opCode["ADD_R"] = 3;
  opCode["ADD_I"] = 4;
  opCode["SUB_R"] = 5;
  opCode["SUB_I"] = 6;
  opCode["MUL_R"] = 7;
  opCode["MUL_I"] = 8;
  opCode["DIV_R"] = 9;
  opCode["DIV_I"] = 10;
  opCode["STORE_R"] = 11;
  opCode["STORE_I"] = 12;
  opCode["LOAD_R"] = 13;
  opCode["LOAD_I"] = 14;
  opCode["BEQ_R"] = 15;
  opCode["BLT_R"] = 16;
  opCode["JMP_R"] = 17;
  opCode["JMP_I"] = 18;
  opCode["DRIVE_R"] = 19;
  opCode["DRIVE_I"] = 20;
  opCode["TURN_R"] = 21;
  opCode["TURN_I"] = 22;
  opCode["FIRE_R"] = 23;
  opCode["FIRE_I"] = 24;
  opCode["SCAN"] = 25;

  //map for registers
  map<string, unsigned int> registerMap;
  registerMap["PC"] = 0;
  registerMap["OP1"] = 1;
  registerMap["OP2"] = 2;
  registerMap["RESULT"] = 3;
  registerMap["SCANRANGE"] = 4;
  registerMap["FIRERANGE"] = 5;
  registerMap["DRIVESPEED"] = 6;
  registerMap["TURNSPEED"] = 7;
  registerMap["R0"] = 8;
  registerMap["R1"] = 9;
  registerMap["R2"] = 10;
  registerMap["R3"] = 11;
  registerMap["R4"] = 12;
  registerMap["R5"] = 13;
  registerMap["R6"] = 14;
  registerMap["R7"] = 15;
  registerMap["N_REGISTERS"] = 16;

  //declaration of variables
  string instruction, value, operation, reg1, reg2, target;
  char * memblock;
  int temp;

  ifstream aiFile;

  //removing bin every time program is executed
  remove("program.bin");

  //open AI file to read
  aiFile.open(argv[1]);

  //open the output Binary file
  ofstream compiledFile ("program.bin", ios::out | ios::app | ios::binary);

  //check if file successfully opened
  if(aiFile.is_open()){
    cerr << "AI file loaded" << endl;
    
    //parse AI file line by line
    int linenum = 0;
    while(getline(aiFile,instruction)){
      linenum++;
      
      //basically splitting the stream
      std::stringstream line(instruction);
      
      //parsing input
      line >> operation >> target >> reg1 >> reg2;
      
      //DEBUGGING
      /*
      cout << "Instruction: " <<  instruction << endl;
      cout << "Operation : " <<  operation << endl;
      cout << "Target: " << target << endl;
      cout << "Register 1 / Immediate: " << reg1 << endl;
      cout << "Register 2 / Immediate: " << reg2 << endl;
      cout << "opcode[operation] = " << opCode[operation] << endl;
      */      

      //cut off extra characters if a target register
      if(target[0] == '$'){
	target = target.substr(2,target.length() -3);

	//debugging
	//cout << "Trimmed target : " << target << endl;
      }
      
      //cut off extra characters if a register
      if(reg1[0] == '$'){
	reg1 = reg1.substr(2,reg1.length() - 3);

	//debugging
	//cout << "Trimmed reg1 : " << reg1 << endl;
      }
      
      //cut off extra characters if a register
      if(reg2[0] == '$'){
	reg2 = reg2.substr(2,reg2.length() - 3);

	//debugging
	//cout << "Trimmed reg2 : " << reg2 << endl;
      }
      
      // create an array of 4 bytes (32 bits)
      memblock = new char [4];
      
      //switch on opCode index returned from the map
      switch(opCode[operation]){

      case 0: //NOOP
	// NOOP FF FF FF
	memblock[0] = memblock[1] = memblock[2] = memblock[3] = (char)0;
	cout << "NOOP" << endl;
	break;
	
      /*FORMAT
       *INPUT: OPCODE TARGET REG1 IMMEDIATE
       *OUTPUT: TARGET REGISTER1 REGISTER2
       */
      case 4: //ADD_I
      case 6: //SUB_I
      case 8: //MUL_I
      case 10: //DIV_I
        cerr << "Math w/ immediate" << endl;
	memblock[0] = (char)opCode[operation];
	
	// if target is a register
	if( (registerMap.count(target) > 0) ){
	  memblock[1] = (char)registerMap[target];
	}else{
	  cerr << "ERROR: INVALID TARGET REGISTER, LINE " << linenum << endl;
	  exit(-1);
	}

	//if register is a register
	if( (registerMap.count(reg1) > 0) ){
	  memblock[2] = (char)registerMap[reg1];
	}else{
	  cerr << "ERROR: INVALID REGISTER 1, LINE " << linenum << endl;
	  exit(-1);
	}
	
        memblock[3] = (char)atoi(reg2.c_str());
	
	break;

      /*FORMAT
       *INPUT: OPCODE TARGET REG1 REG2
       *OUTPUT: TARGET REGISTER1 REGISTER2
       */
      case 3: //ADD_R
      case 5: //SUB_R
      case 7: //MUL_R
      case 9: //DIV_R
      case 15: //BEQ_R
      case 16: //BLT_R
	cerr << "Math with registers, BLT, BEQ" << endl;
	memblock[0] = (char)opCode[operation];
	
	// if target is a register
	if( (registerMap.count(target) > 0) ){
	  memblock[1] = (char)registerMap[target];
	}else{
          memblock[1] = atoi(target.c_str());
	  //cerr << "ERROR: INVALID TARGET REGISTER, LINE " << linenum << endl;
	  //exit(-1);
	}

	//if register is a register
	if( (registerMap.count(reg1) > 0) ){
	  memblock[2] = (char)registerMap[reg1];
	}else{
	  cerr << "ERROR: INVALID REGISTER 1, LINE " << linenum << endl;
	  exit(-1);
	}
	
	//if register is register
	if( (registerMap.count(reg2) > 0) ){
	  memblock[3] = (char)registerMap[reg2];
	}else{
	  temp = atoi(reg2.c_str());
	  memblock[3] = (char) temp;
	}
	
	break;

      /*FORMAT
       *INPUT: OPCODE TARGET IMMEDIATE
       *OUTPUT: OPCODE TARGET 0 IMMEDIATE
       */
      case 2: //SET_I
      case 12: //STORE_I
      case 14: //LOAD_I
        cerr << "Set store load, immediate" << endl;
	memblock[0] = (char)opCode[operation];
	
	// if target is a register
	if( (registerMap.count(target) > 0) ){
	  memblock[1] = (char)registerMap[target];
	}else{
	  cerr << "ERROR: INVALID TARGET REGISTER, LINE " << linenum << endl;
	  exit(-1);
	}

	//setting register byte to 0
	memblock[2] = (char)0;
	
	//setting immediate
	temp = atoi(reg1.c_str());
	memblock[3] = (char) temp;	
	
	break;

      /*FORMAT
       *INPUT: OPCODE TARGET REG1
       *OUTPUT: OPCODE TARGET REGISTER1 0 
       */
      case 1: //SET_R
      case 11: //STORE_R
      case 13: //LOAD_R
      case 19: //DRIVE_R
      case 21: //TURN_R
	cerr << "Set store load drive turn, register" << endl;
	memblock[0] = (char)opCode[operation];

        // if target is a register
        if( (registerMap.count(target) > 0) ){
          memblock[1] = (char)registerMap[target];
        }else{
          cerr << "ERROR: INVALID TARGET REGISTER, LINE " << linenum << endl;
          exit(-1);
        }

        if( (registerMap.count(reg1) > 0) ){
          memblock[2] = (char)registerMap[reg1];
        }else{
          cerr << "ERROR: INVALID REGISTER 1, LINE " << linenum << endl;
          exit(-1);
        }

        memblock[3] = (char)0;

	break;
	
      /*FORMAT
       *INPUT: OPCODE IMMEDIATE
       *OUTPUT: OPCODE 0 0 IMMEDIATE
       */
      case 18: //JMP_I
      case 20: //DRIVE_I
      case 22: //TURN_I
      case 24: //FIRE_I, ISN'T THIS USELESS???
        cerr << "Jump drive turn fire, immediate" << endl;

	memblock[0] = (char)opCode[operation];
	memblock[1] = (char)0;
        memblock[2] = (char)0;

        temp = atoi(target.c_str());
        memblock[3] = (char) temp;

	break;
	
      /*FORMAT
       *INPUT: OPCODE TARGET
       *OUTPUT: OPCODE TARGET 0 0
       */
      case 17: //JMP_R
      case 23: //FIRE_R
      case 25: //SCAN
        cerr << "Jump fire scan, register" << endl;
	memblock[0] = (char)opCode[operation];

        // if target is a register
        if( (registerMap.count(target) > 0) ){
          // this is a hack, but it works.  FIRE_R
          // and JMP_R should really be reading from REG1.
          memblock[1] = (char)registerMap[target];
          memblock[2] = (char)registerMap[target];
        }else{
          cerr << "ERROR: INVALID TARGET REGISTER, LINE " << linenum << endl;
          exit(-1);
        }

	memblock[3] = (char)0;

	break;
	
      default:
	cerr << "DEFAULT" << endl;
	break;	   
   
      }//end switch
      
      //write to binary file
      compiledFile.write(memblock, 4 * sizeof(char));

    }//end while
    
  }else{
    
    cerr << "Failed to open ai.txt" << endl;
  }

  //freeing memory
  delete [] memblock; 
    
  //closing files
  compiledFile.close();
  aiFile.close();
  return 1;
}


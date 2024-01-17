
//  CITS2002 Project 1 2021
//  Name(s):             student-name1   (, student-name2)
//  Student number(s):   student-number1 (, student-number2)

//  compile with:  cc -std=c11 -Wall -Werror -o runcool runcool.c

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  THE STACK-BASED MACHINE HAS 2^16 (= 65,536) WORDS OF MAIN MEMORY
#define N_MAIN_MEMORY_WORDS (1<<16)

//  EACH WORD OF MEMORY CAN STORE A 16-bit UNSIGNED ADDRESS (0 to 65535)
#define AWORD               uint16_t
//  OR STORE A 16-bit SIGNED INTEGER (-32,768 to 32,767)
#define IWORD               int16_t

//  THE ARRAY OF 65,536 WORDS OF MAIN MEMORY
AWORD                       main_memory[N_MAIN_MEMORY_WORDS];

//  THE SMALL-BUT-FAST CACHE HAS 32 WORDS OF MEMORY
#define N_CACHE_WORDS       32


//  see:  https://teaching.csse.uwa.edu.au/units/CITS2002/projects/coolinstructions.php
enum INSTRUCTION {
    I_HALT       = 0,
    I_NOP,
    I_ADD,
    I_SUB,
    I_MULT,
    I_DIV,
    I_CALL,
    I_RETURN,
    I_JMP,
    I_JEQ,
    I_PRINTI,
    I_PRINTS,
    I_PUSHC,
    I_PUSHA,
    I_PUSHR,
    I_POPA,
    I_POPR
};

//  USE VALUES OF enum INSTRUCTION TO INDEX THE INSTRUCTION_name[] ARRAY
const char *INSTRUCTION_name[] = {
    "halt",
    "nop",
    "add",
    "sub",
    "mult",
    "div",
    "call",
    "return",
    "jmp",
    "jeq",
    "printi",
    "prints",
    "pushc",
    "pusha",
    "pushr",
    "popa",
    "popr"
};

//  ----  IT IS SAFE TO MODIFY ANYTHING BELOW THIS LINE  --------------

AWORD        cache_memory[N_CACHE_WORDS][2];

//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;

void report_statistics(void)
{
    printf("@number-of-main-memory-reads\t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t%i\n",  n_cache_memory_misses);
}

//  -------------------------------------------------------------------

//  EVEN THOUGH main_memory[] IS AN ARRAY OF WORDS, IT SHOULD NOT BE ACCESSED DIRECTLY.
//  INSTEAD, USE THESE FUNCTIONS read_memory() and write_memory()
//
//  THIS WILL MAKE THINGS EASIER WHEN WHEN EXTENDING THE CODE TO
//  SUPPORT CACHE MEMORY

AWORD read_memory(int address)
{
	//check if address stored in cache which means we have the word, if we do return it to the CPU

	IWORD value;

        if (address != 0){
	bool hit = false;
        for (int i = 0; i < N_CACHE_WORDS; i++){
		//printf("%i %i\n", i, cache_memory[i][1]);
		if (cache_memory[i][1] == address) {
			//printf("Its a cache hit %i - %i\n", address, cache_memory[i][0]);
			n_cache_memory_hits++; 
			hit = true; 
			value = cache_memory[i][0];
			break;
		}
	}
	
	if (hit == false){
		//if we dont, lets grab it from the main memory and write it to cache (at the appropriate location)
		n_cache_memory_misses++;
		value  = main_memory[address];
	        AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
		AWORD cache_location = difference_setter % 32;
		cache_memory[cache_location][0] = value;
		cache_memory[cache_location][1] = address;
		//printf("%i %i\n", difference_setter, cache_location);

		n_main_memory_reads++;
	}
	return value;
	}
        else {
	n_main_memory_reads++;
	return main_memory[address];
	}
        
}

void write_memory(AWORD address, AWORD value)
{
	//printf("%i-hello\n", value);
	AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
	AWORD cache_location = difference_setter % 32;
	cache_memory[cache_location][0] = value;
	cache_memory[cache_location][1] = address;
	//printf("Writing to cache as usual - %i, %i\n", value, address);

	n_main_memory_writes++;
	main_memory[address] = value;
	//printf("%i, %i\n",address, main_memory[address]);
}


//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                     // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer

//  REMOVE THE FOLLOWING LINE ONCE YOU ACTUALLY NEED TO USE FP
    //FP = FP+0;

    while(true) {

	    IWORD value1, value2, return_value, offset;
            AWORD first_instruct_addy, return_addy, int_addy;
//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

      //printf("%s\n", INSTRUCTION_name[instruction]);

        if(instruction == I_HALT) {
		break;}
	switch(instruction){
		case I_NOP:
			break;

		case I_ADD:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_memory(SP, value1 + value2);
			break;

		case I_SUB:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_memory(SP,value2 - value1);
			break;

		case I_MULT:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_memory(SP, value1*value2);
			break;

		case I_DIV:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_memory(SP,value2/value1);
			break;

		case I_CALL:
			first_instruct_addy = read_memory(PC);
			--SP;
			write_memory(SP,PC+1);
			//printf("%i,%i\n",SP, PC);
			--SP;
			write_memory(SP, FP);
			//printf("SP is %i, FP is %i", SP, FP);
			FP = SP;
			PC = first_instruct_addy;
			break;

		case I_RETURN:
			offset = read_memory(PC);
			return_addy = FP +1;
			PC = read_memory(return_addy);
			//printf("%i,%i\n", PC,return_addy);
			return_value = read_memory(SP);
			SP = FP + offset;
			write_memory(SP, return_value);
			FP = read_memory(FP);
			break;

		case I_JMP:
			PC  = read_memory(PC);
			break;

		case I_JEQ:
			value1 = read_memory(SP);
			SP++;
			if (value1 == 0){PC = read_memory(PC);}
			else {PC++;}
			break;

		case I_PRINTI:
			value1 = read_memory(SP);
			SP++;
			printf("%i",value1);
			break;

		case I_PRINTS:
			int_addy = PC; //save where we are
			PC = read_memory(PC);

			while (true){
			        value1 = read_memory(PC);
                                char c1 = value1 & 0xff;
		           	char c2 = (value1 & 0xff00)>>8;
				if (c1 != '\0') {printf("%c", c1);}
				else {break;}

				if (c2 != '\0') {printf("%c", c2);}
				else {break;}
				PC++;}
			PC = int_addy;
			PC++;
			break;

		case I_PUSHC:
			value1 = read_memory(PC);
			PC++;
			SP--;
			write_memory(SP, value1);
			break;
		case I_PUSHA:
			int_addy = read_memory(PC);
			value1 = read_memory(int_addy);
			SP--;
			write_memory(SP, value1);
			PC++;
			break;
			
		case I_PUSHR:
			offset = read_memory(PC);
			int_addy = FP + offset;
			value1 = read_memory(int_addy);
			SP--;
			write_memory(SP,value1);
			PC++;
			break;

		case I_POPA:
			int_addy = read_memory(PC);
			value1 = read_memory(SP);
			SP++;
			write_memory(int_addy,value1);
			PC++;
			break;

		case I_POPR:
			offset = read_memory(PC);
			int_addy = FP + offset;
			value1 = read_memory(SP);
			SP++;
			write_memory(int_addy, value1);
			PC++;
			break;
	}

//  SUPPORT OTHER INSTRUCTIONS HERE
//      ....
    }

//  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    return read_memory(SP);
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[])
{
    memset(main_memory, 0, sizeof main_memory);   //  clear all memory

    FILE* file;
  
    file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    int numbytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    int elements = numbytes/2;
    //IWORD buffer[elements];
    
    fread(main_memory, sizeof(main_memory), elements, file);
    fclose(file);

   // for (AWORD i=0;i <elements; i++){
	   // int l = sizeof(i);
	    //write_memory(i, buffer[i]);
	    //printf("%i, size of %i\n", main_memory[i],l);
	    
    //}
 

//  READ CONTENTS OF coolexe FILE
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{
//  CHECK THE NUMBER OF ARGUMENTS
    if(argc != 2) {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();
    printf("%i\n", result);
    return result;          // or  exit(result);
}

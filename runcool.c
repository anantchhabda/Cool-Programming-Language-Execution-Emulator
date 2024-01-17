// THE PROGRAM IS DESIGNED TO EMULATE THE EXECTUTION OF "COOL" PROGRAMS 
// BY INTERPRETING CONTENTS OF A COOLEXE FILE.
// WRITE BACK CACHE POLICY IS IMPLEMENTED WITH DIRECT MAPPING CACHE. 
//
// PROGRAM TRIES TO REPLICATE THE IDEA OF A SMALL BUT FAST CACHE MEMORY
// WORKING ALONGSIDE MAIN MEMORY(RAM). IMPLEMENTED BY USING ARRAYS. 

//  CITS2002 Project 1 2021
//  Name(s):             Anant Chhabda
//  Student number(s):   21712878

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

//CREATE A 2D ARRAY FOR CACHE MEMORY TO STORE A WORD, AN ADDRESS AND A DIRTY TAG RESPECTIVELY
AWORD        cache_memory[N_CACHE_WORDS][3]; 

//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;

void report_statistics(void)
{
    printf("@number-of-main-memory-reads-(fast-jeq) \t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes-(fast-jeq)\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits            \t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses          \t%i\n",  n_cache_memory_misses);
}

//  -------------------------------------------------------------------

//  EVEN THOUGH main_memory[] IS AN ARRAY OF WORDS, IT SHOULD NOT BE ACCESSED DIRECTLY.
//  INSTEAD, USE THESE FUNCTIONS read_memory() and write_memory()
//
//  THIS WILL MAKE THINGS EASIER WHEN WHEN EXTENDING THE CODE TO
//  SUPPORT CACHE MEMORY


// FUNCTION TO WRITE TO MAIN MEMORY
void write_memory(AWORD address, AWORD value)
{
	n_main_memory_writes++;
	main_memory[address] = value;

}

//  ------------------------------------------------------------------

// FUNCTION TO READ FROM MAIN MEMORY/CACHE WHEN POSSIBLE
AWORD read_memory(int address)
{
	IWORD value;
  
	// FIGURE OUT WHICH CACHE LOCATION REQUIRED FOR THE PARTICULAR ADDRESS
	AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
	AWORD cache_location = difference_setter % 32;

	// CHECK IF THE CACHE LOCATION HAS THE REQUIRED ADDRESS
	if (cache_memory[cache_location][1] == address) {
		n_cache_memory_hits++;
		value = cache_memory[cache_location][0];
		return value; // return the stored value
	}

	// IF CACHE MEMORY DOES NOT HAVE IT,GRAB IT FROM THE MAIN MEMORY AND WRITE IT
	// TO THE CACHE (FOR FUTURE READS)
	else{

		n_cache_memory_misses++;
		value  = main_memory[address];
		n_main_memory_reads++;

		// BEFORE WRITING IT TO THE CACHE, CHECK IF CACHE LOCATION IS DIRTY OR NOT
		if (cache_memory[cache_location][2] !=1){
			
			// TAG IS NOT DIRTY SO WRITE IT DIRECTLY TO CACHE
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 0; // mark as not dirty since grabbed from main memory
		}
		else {
			// TAG IS DIRTY, SO WRITE THE CACHE LOCATION DATA TO MAIN MEMORY
			// FIRST
			IWORD our_dirty_value = cache_memory[cache_location][0];
			AWORD main_memory_address = cache_memory[cache_location][1];

			write_memory(main_memory_address, our_dirty_value);
		   
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 0; // mark as not dirty since grabbed from the main memory

		}

	return value;
	}
        
}

//  ------------------------------------------------------------------

// SPECIFIC FUNCTION WRITING TO CACHE MEMORY FIRST
void write_cache_memory(AWORD address, AWORD value)
{
	// FIGURE OUT WHICH CACHE LOCATION TO WRITE IT TO
	AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
	AWORD cache_location = difference_setter % 32;

	// BEFORE WRITING, CHECK IF WE ARE UPDATING THE SAME ADDRESS. 
	// IF WE ARE, NO NEED TO WRITE IT TO THE MAIN MEMORY JUST YET

	//IF ADDRESS IS THE SAME, DOUBLE CHECK REQUESTED VALUE BEING WRITTEN IS DIFFERENT 
        if(cache_memory[cache_location][1] == address && cache_memory[cache_location][0] != value) 
	{
		cache_memory[cache_location][0] = value;
		cache_memory[cache_location][2] = 1;
	}

	//IF ADDRESS IS THE SAME AND REQUESTED VALUE BEING WRITTEN IS THE SAME, DO NOTHING
	else if (cache_memory[cache_location][1] == address && cache_memory[cache_location][0] == value)
	{}

	// THE ADDRESS IS NOT THE SAME, SOME OTHER MEMORY ADDRESS REQUIRES THE 
	// CACHE LOCATION. CHECK IF THE CACHE LOCATION TAG IS DIRTY OR NOT
	else{
		//IT IS NOT DIRTY, WRITE THE REQUEST DIRECTLY INTO CACHE
		if (cache_memory[cache_location][2] != 1)
		{
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 1; 
		}

		// IT IS DIRTY, WRITE THE EXISTING DATA INTO MAIN MEMORY FIRST
		// BEFORE PROCESSING THE NEW WRITE REQUEST
		else
		{

			IWORD our_dirty_value  = cache_memory[cache_location][0];
			AWORD main_memory_address = cache_memory[cache_location][1];

			write_memory(main_memory_address, our_dirty_value);

			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 1;
		}	    
	}

}

//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                     // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // initialised to bottom of the stack

    while(true) {

	    IWORD value1, value2, return_value, offset;
            AWORD first_instruct_address, return_address, our_address;

//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

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
			write_cache_memory(SP, value1 + value2);
			break;

		case I_SUB:

			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_cache_memory(SP,value2 - value1);
			break;

		case I_MULT:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_cache_memory(SP, value1*value2);
			break;

		case I_DIV:
			value1 = read_memory(SP);
			SP++;
			value2 = read_memory(SP);
			SP++;
			SP--;
			write_cache_memory(SP,value2/value1);
			break;

		case I_CALL:
			first_instruct_address = read_memory(PC);
			--SP;
			write_cache_memory(SP,PC+1);
			--SP;
			write_cache_memory(SP, FP);
			FP = SP;
			PC = first_instruct_address;
			break;

		case I_RETURN:
			offset = read_memory(PC);
			return_address = FP +1;
			PC = read_memory(return_address);
			return_value = read_memory(SP);
			SP = FP + offset;
			write_cache_memory(SP, return_value);
			FP = read_memory(FP);
			break;

		case I_JMP:
			PC  = read_memory(PC);
			break;

		case I_JEQ:
			value1 = read_memory(SP);
			SP++;
			if(value1 == 0)
			{
				PC = read_memory(PC);
			}
			else {PC++;}
			break;

		case I_PRINTI:
			value1 = read_memory(SP);
			SP++;
			printf("%i",value1);
			break;

		case I_PRINTS:
			our_address = PC; 
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
			PC = our_address;
			PC++;
			break;

		case I_PUSHC:
			value1 = read_memory(PC);
			PC++;
			SP--;
			write_cache_memory(SP, value1);
			break;

		case I_PUSHA:
			our_address = read_memory(PC);
			value1 = read_memory(our_address);
			SP--;
			write_cache_memory(SP, value1);
			PC++;
			break;
			
		case I_PUSHR:
			offset = read_memory(PC);
			our_address = FP + offset;
			value1 = read_memory(our_address);
			SP--;
			write_cache_memory(SP,value1);
			PC++;
			break;

		case I_POPA:
			our_address = read_memory(PC);
			value1 = read_memory(SP);
			SP++;
			write_cache_memory(our_address,value1);
			PC++;
			break;

		case I_POPR:
			offset = read_memory(PC);
			our_address = FP + offset;
			value1 = read_memory(SP);
			SP++;
			write_cache_memory(our_address, value1);
			PC++;
			break;
	}

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
    int numbytes = ftell(file); // total num of bytes in the file
    fseek(file, 0, SEEK_SET); 

    int word  = numbytes/2; // each word is 2 bytes

    fread(main_memory, sizeof(main_memory), word, file);
    fclose(file);
 
}

//  -------------------------------------------------------------------

//INITALISE CACHE_MEMORY
void initialise_cache(void)
{
    for(int i =0; i <N_CACHE_WORDS; i++){
	cache_memory[i][1] = (N_MAIN_MEMORY_WORDS-1) - i;
	cache_memory[i][2] = 1;} //all locations initialised as dirty
}
 
//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{
//  CHECK THE NUMBER OF ARGUMENTS
    if(argc != 2) {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

// INITIALISE THE CACHE MEMORY
    initialise_cache();
    

//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();
    printf("%i\n", result);
    return result;          // or  exit(result);
}

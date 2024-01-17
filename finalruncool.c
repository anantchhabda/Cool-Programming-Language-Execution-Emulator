

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

AWORD        cache_memory[N_CACHE_WORDS][3]; //inside array 3 spots - one for word, one for address and one for dirty tag.


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

AWORD read_memory(int address) //nothing changes for read_memory for write back cache 
{
	//check if address stored in cache which means we have the word, if we do return it to the CPU

	IWORD value;

	//bool hit = false;
        for (int i = 0; i < N_CACHE_WORDS; i++){
		//printf("%i %i\n", i, cache_memory[i][1]);
		if (cache_memory[i][1] == address) {
			//printf("Its a cache hit %i - %i\n", address, cache_memory[i][0]);
			n_cache_memory_hits++; 
		//	hit = true; 
			value = cache_memory[i][0];
			return value;
		}
	}
	
	//if (hit == false){
		//if we dont, lets grab it from the main memory and write it to cache (at the appropriate location)

		n_cache_memory_misses++;
		value  = main_memory[address];
		n_main_memory_reads++;
	        AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
		AWORD cache_location = difference_setter % 32;
                
		//we have grabbed it from main memory, and about to write it to the appopriate location in the cache
		//but first we need to check if that place in cache is dirty or not. If its dirty, then we write it to main memory, if not its straightforward.
		if (cache_memory[cache_location][2] !=1){
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 0; //not dirty since it came from main memory
		}
		else {
			IWORD our_dirty_value = cache_memory[cache_location][0];
			AWORD main_memory_address = cache_memory[cache_location][1];
			
			main_memory[main_memory_address] = our_dirty_value;
		        n_main_memory_writes++;

			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 0; //not dirty since it came from main memory
		}
		//printf("%i %i\n", difference_setter, cache_location);

	//	}
	return value;	
        
}

void write_memory(AWORD address, AWORD value)

//implementation will be as follows. Every time we have a write call from the CPU, we will write it to cache, just like we always do. But this time we wont write it 
// to the main memory. When writing to the cache, we need to check that the place we are writing, has that data of that place consistent with the memory. It would be consisitent
// if it was a result of a read operation from the main memory. But it would be inconsistent if it was a result of a write operation previoously 
// quite simply, I will use the notion of a dirty tag. 1 meaning tag is dirty, 0 meaning its not dirty.
{
	//printf("%i-hello\n", value);
	
	AWORD difference_setter = (N_MAIN_MEMORY_WORDS-1) - address;
	AWORD cache_location = difference_setter % 32;

	//if tag is not 1 therefore its 0, so not dirty. Therefore we can just replace it as usual

        //update: so write writes, I have realised that we are getting a lot more writes, the problem lies in the simple
        //fact that I am writing the same "variable/memory address" every time its updated. Compared to writing it only when some other memory address requires that place,
        //so we will be implementing this now 

        if(cache_memory[cache_location][1] == address) //updating the same memory location, no need to write it to main_memory just yet
	{
		cache_memory[cache_location][0] = value;
		cache_memory[cache_location][2] = 1; //two possibilities, either its in cache because it was written before (which means its dirty) and we are re-writing it. Or its in cache because it was read (which means its not dirty) but my writing to it, we make it dirty.
	}

	else{
		if (cache_memory[cache_location][2] != 1){
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 1;
		}
	
	//if the tag is 1 therefore its dirty. We need to write the data to the main memory and then update with new data
		else{

			IWORD our_dirty_value  = cache_memory[cache_location][0];
			AWORD main_memory_address = cache_memory[cache_location][1];
			main_memory[main_memory_address] = our_dirty_value;
			n_main_memory_writes++;
			cache_memory[cache_location][0] = value;
			cache_memory[cache_location][1] = address;
			cache_memory[cache_location][2] = 1;
		}	    
	}

	//printf("Writing to cache as usual - %i, %i\n", value, address);

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
			//AWORD temp_var = read_memory(PC);
			if (value1 == 0){//PC = temp_var;
				PC = read_memory(PC);}

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

    for(int i =0; i <N_CACHE_WORDS; i++){
	cache_memory[i][1] = (N_MAIN_MEMORY_WORDS-1) - i;
	cache_memory[i][2] = 1;}

//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();
    printf("%i\n", result);
    return result;          // or  exit(result);
}

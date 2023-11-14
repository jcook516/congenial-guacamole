#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "machine.h"
#include "code.h"

struct machine_t machine;

/*
 * Allocate more space to keep track of values on the simulated stack.
 */
void grow_stack(uint64_t new_sp) {
    // Grow the stack upwards
    if (new_sp < machine.stack_top) {
        // Round down to a multiple of word size
        if (new_sp % WORD_SIZE_BYTES != 0) {
            new_sp -= new_sp % WORD_SIZE_BYTES;
        }

        // Allocate space and copy over old values 
        void *new_stack = malloc(machine.stack_bot - new_sp + 1);
        memset(new_stack, 0, machine.stack_top - new_sp);
        if (machine.stack != NULL) {
            memcpy(new_stack + (machine.stack_top - new_sp), machine.stack, machine.stack_bot - machine.stack_top + 1);
            free(machine.stack);
        }

        // Update machine
        machine.stack = new_stack;
        machine.stack_top = new_sp;
    }
    // Grow the stack downwards
    else if (new_sp > machine.stack_bot) {
        // Round up to a multiple of word size
        if (new_sp % WORD_SIZE_BYTES != 0) {
            new_sp += WORD_SIZE_BYTES - (new_sp % WORD_SIZE_BYTES);
        }
        else {
            new_sp += WORD_SIZE_BYTES;
        }

        // Allocate space and copy over old values 
        void *new_stack = malloc(new_sp - machine.stack_top);
        memset(new_stack + (machine.stack_bot - machine.stack_top), 0, new_sp - machine.stack_bot);
        if (machine.stack != NULL) {
            memcpy(new_stack, machine.stack, machine.stack_bot - machine.stack_top);
            free(machine.stack);
        }

        // Update machine
        machine.stack = new_stack;
        machine.stack_bot = new_sp - 1;
    }
}

/*
 * Initialize the machine
 */
void init_machine(uint64_t sp, uint64_t pc, char *code_filepath) {
    // Populate general purpose registers
    for (int i = 0; i <= 30; i++) {
        machine.registers[i] = REGISTER_NULL;
    }
    
    // Populate special purpose registers
    machine.sp = sp;
    machine.pc = pc;
    
    // Load code
    machine.code = parse_file(code_filepath, &(machine.code_top), &(machine.code_bot));
    /*int i = 0;
    while (machine.code[i].operation.code != OPERATION_NULL) {
        print_instruction(machine.code[i]);
        i++;
    }*/

    // Prepare stack
    machine.stack_top = sp;
    machine.stack_bot = sp + WORD_SIZE_BYTES - 1;
    machine.stack = malloc(WORD_SIZE_BYTES);
    memset(machine.stack, 0, WORD_SIZE_BYTES);

    // Clear all condition codes
    machine.conditions = 0;
}

void print_memory() {
    // Print condition codes
    printf("Condition codes:");
    if (machine.conditions & CONDITION_ZERO) {
        printf(" Z");
    }
    if (machine.conditions & CONDITION_NEGATIVE) {
        printf(" N");
    }
    if (machine.conditions & CONDITION_POSITIVE) {
        printf(" P");
    }
    printf("\n");

    // Print the value of all used registers
    printf("Registers:\n");
    for (int i = 0; i <= 30; i++) {
        if (machine.registers[i] != REGISTER_NULL) {
            printf("\tw/x%d = 0x%lx\n", i, machine.registers[i]);
        }
    }
    printf("\tsp = 0x%lX\n", machine.sp);
    printf("\tpc = 0x%lX\n", machine.pc);

    // If necessary, grow the stack before printing it
    if (machine.sp < machine.stack_top || machine.sp > machine.stack_bot) {
        grow_stack(machine.sp);
    }

    // Print the value of all words on the stack
    printf("Stack:\n");
    unsigned char *stack = machine.stack;
    for (int i = 0; i < (machine.stack_bot - machine.stack_top); i += 8) {
        printf("\t");

        if (machine.sp == i + machine.stack_top) {
            printf("%10s ", "sp->");
        }
        else {
            printf("           ");
        }

        printf("+-------------------------+\n");
        printf("\t0x%08lX | ", i + machine.stack_top);
        for (int j = 0; j < 8; j++) {
            printf("%02X ", stack[i+j]);
        }
        printf("|\n");
    }
    printf("\t           +-------------------------+\n");
}

/*
 * Get the next instruction to execute
 */
struct instruction_t fetch() {
    int index = (machine.pc - machine.code_top) / 4;
    return machine.code[index];
}

/*
 * Get the value associated with a constant or register operand.
 */
uint64_t get_value(struct operand_t operand) {
    switch (operand.type) {
        case OPERAND_constant:
            return operand.constant;
        
        case OPERAND_address:
            return operand.constant;

        case OPERAND_register:
            switch (operand.reg_type) {
                case REGISTER_w:
                    // Return only lower 32-bits for w registers - from ChatGPT
                    return machine.registers[operand.reg_num] & 0xFFFFFFFF;

                case REGISTER_x:
                    return machine.registers[operand.reg_num];

                case REGISTER_pc:
                    return machine.pc;
                
                case REGISTER_sp:
                    return machine.sp;
            }
    }
    return 0;
}

/*
 * Put a value in a register specified by an operand.
 */
void put_value(struct operand_t operand, uint64_t value) {
    assert(operand.type == OPERAND_register);
    switch (operand.reg_type) {
        case REGISTER_w:
            machine.registers[operand.reg_num] = value & 0xFFFFFFFF;
            break;

        case REGISTER_x:
            machine.registers[operand.reg_num] = value;
            break;

        case REGISTER_pc:
            machine.pc = value;
            break;

        case REGISTER_sp:
            machine.sp = value;
            break;
    }
}

/*
 * Get the memory address associated with a memory operand.
 */
uint64_t get_memory_address(struct operand_t operand) {
    assert(operand.type == OPERAND_memory);

    uint64_t val = 0;
    operand.type = OPERAND_register;
    val = get_value(operand) + operand.constant;
    operand.type = OPERAND_memory;

    return val;
}


void execute_ldr_str(struct operand_t first, struct operand_t second, unsigned int operation){
    
    if (first.reg_type == REGISTER_w){
        uint32_t* addy = machine.stack + (machine.stack_top - get_memory_address(second));
    
        switch(operation){
            case OPERATION_ldr:
                put_value(first,*addy);
                break;

            case OPERATION_str:
                *addy = get_value(first);
                break;
        }
    }

    else{        
        uint64_t* addy = machine.stack + (machine.stack_top - get_memory_address(second));

        switch(operation){
            case OPERATION_ldr:
                put_value(first,*addy);
                break;

            case OPERATION_str:
                *addy = get_value(first);
                break;
        }
    }

}

/*
 * Execute an instruction
 */

void execute(struct instruction_t instruction) {
    struct operand_t first = instruction.operands[0];
    struct operand_t second = instruction.operands[1];
    struct operand_t third = instruction.operands[2];
    
    switch(instruction.operation) {
    // TODO
        case OPERATION_nop:
            // Nothing to do
            break;

        case OPERATION_add:
            put_value(first,get_value(second) + get_value(third));
            break;

        case OPERATION_sub:
            put_value(first,get_value(second) - get_value(third));
            break;

        case OPERATION_lsl:
            put_value(first,get_value(second) << get_value(third));
            break;
        
        case OPERATION_lsr:
            put_value(first,get_value(second) >> get_value(third));
            break;
        
        case OPERATION_and:
            put_value(first,get_value(second) & get_value(third));
            break;
        
        case OPERATION_orr:
            put_value(first,get_value(second) | get_value(third));
            break;

        case OPERATION_ldr:
            execute_ldr_str(first,second,instruction.operation);
            break;
        
        case OPERATION_str:
            execute_ldr_str(first,second,instruction.operation);
            break;
        
        case OPERATION_mov:
            put_value(first,get_value(second));
            break;

        case OPERATION_b:
            machine.pc = get_value(first);
            break;

        case OPERATION_bl:
            machine.pc = get_value(first);

            break;


        default:
            printf("!!Instruction not implemented!!\n");
    }
}

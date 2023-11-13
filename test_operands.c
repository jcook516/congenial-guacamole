#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "code.h"
#include "machine.h"

bool ok = true;

#define XTEST(expr, errmsg)                                                    \
  if (!expr) {                                                                 \
    printf("Failure: %s on line %d of %s\n", errmsg, __LINE__, __FILE__);      \
    ok = false;                                                                \
  }

int main() {
    // Initial machine state
    machine.stack_top = 0xFFD0;
    machine.stack_bot = 0xFFF7;
    machine.stack = malloc(machine.stack_bot - machine.stack_top + 1);
    memset(machine.stack, 0, machine.stack_bot - machine.stack_top + 1);
    memset(machine.registers, 0, sizeof(machine.registers));
    machine.sp = machine.stack_top;
    machine.pc = 0xDEADC0DE;
    machine.code = NULL;
    machine.code_top = 0;
    machine.code_bot = 0;

    // Initialize test operands
    struct operand_t w13 = {OPERAND_register, REGISTER_w, 13, 0};
    struct operand_t pc = {OPERAND_register, REGISTER_pc, 0, 0};
    struct operand_t constant = {OPERAND_constant, 0, 0, 21};
    struct operand_t stack = {OPERAND_memory, REGISTER_sp, 0, 20};
    struct operand_t deref = {OPERAND_memory, REGISTER_x, 9, 0};
    struct operand_t addr = {OPERAND_address, 0, 0, 0x12AB};

    // Test get_value
    machine.registers[13] = 0x1234567887654321;
    uint64_t w13_value = get_value(w13);
    XTEST(((w13_value & 0xFFFFFFFF) == 0x87654321), "get_value returned incorrect value for w13");
    XTEST(((w13_value >> 32) == 0x0), "get_value should only return lower 32-bits of w13");
    XTEST((machine.registers[13] == 0x1234567887654321), "get_value should not change value in w13");

    uint64_t pc_value = get_value(pc); 
    XTEST((pc_value == 0xDEADC0DE), "get_value returned incorrect value for pc");
    XTEST((machine.pc == 0xDEADC0DE), "get_value should not change value in pc");

    uint64_t constant_value = get_value(constant); 
    XTEST((constant_value == 21), "get_value returned incorrect value for #21"); 

    uint64_t addr_value = get_value(addr); 
    XTEST((addr_value == 0x12AB), "get_value returned incorrect value for 12AB"); 

    // Test put_value
    machine.registers[13] = 0x2244668811335577;
    put_value(w13, 0x12345678);
    XTEST(((machine.registers[13] & 0xFFFFFFFF) == 0x12345678), "put_value put incorrect value in lower 32-bits of w13");
    XTEST(((machine.registers[13] >> 32) == 0x0), "put_value should put zeros in upper 32-bits of w13");

    machine.pc = 0xDEADC0DE;
    put_value(pc, 0xFEEDC0DE);
    XTEST((machine.pc == 0xFEEDC0DE), "put_value put incorrect value in pc");

    // Test get_memory_address
    uint64_t stack_address = get_memory_address(stack);
    XTEST((stack_address == 0xFFE4), "get_memory_address returned incorrect value for [sp, #20]"); 
    XTEST((machine.sp == 0xFFD0), "get_memory_address should not change value in sp"); 

    machine.registers[9] = 0xFFE8;
    uint64_t deref_address = get_memory_address(deref);
    XTEST((deref_address == 0xFFE8), "get_memory_address returned incorrect value for [x9]"); 
    XTEST((machine.registers[9] == 0xFFE8), "get_memory_address should not change value in x9"); 

    if (ok) {
        printf("All tests passed\n");
    }
}

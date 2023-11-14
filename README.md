# Project 03: Assembly simulator

## Overview
In this project, you'll finish implementing a simulator that visualizes the execution of ARM assembly code. Your simulator will support the following instructions:
* arithmetic (`add`, `sub`) and bitwise (`lsl`, `lsr`, `and`, `orr`)
* load/store/move (`ldr`, `str`, `mov`)
* branching (`b`, `b.ne`, `b.eq`, `b.lt`, `b.gt`, `b.le`, `b.ge`, `bl`, `ret`)

### Standards
This project assesses the following standards:
* 1.2: Use variables, operators, conditionals, loops, and functions appropriately in C code
* 3.1: Use arrays, pointers, dynamic memory allocation, and structs appropriately in C code
* 3.3: Diagnose and correct pointer and dynamic memory errors and leaks in C code
* 5.2: Understand the semantics of ARM assembly code

### Important tips
* Read the entirety of the project description before you write any code.
* Work on the project over multiple days. Spreading out the work will give you time to think about how to solve problems, allow you to ask questions, and result in better outcomes.
* Ensure you follow good program design, coding, testing, and debugging practices (details below).

## Getting started 
Clone your repository on your RPi. 

Your repository contains several source files:
* `code.h` defines structs and constants for representing assembly instructions and operands
* `code.c` contains functions for parsing output from objdump and displaying parsed assembly instructions/operands
* `machine.h` defines a struct for representing a simulated ARM system
* `machine.c` contains a global variable (`machine`) representing a simulated ARM system, functions for initializing and printing the system state (i.e., stack and registers), and functions for fetching and executing assembly instructions
* `simulator.c` contains the `main` function which calls functions in `machine.c` to initialize the simulated ARM system, fetch and execute assembly instructions, and print the system state
* `test_operands.c` contains testcases for the [operand helper functions](#operand-struct-and-helper-functions) you will write

Your repository also includes an `examples` directory with C code, assembly code, and the expected simulator output for several example programs.

Compile your code using `make`.

### Running the simulator
To simulate the execution of assembly instructions and display the system state, run the command
```bash
./simulator CODE_FILEPATH PC SP
```
replacing `CODE_FILEPATH` with the path to a file containing output from objdump, `PC` with the initial value of the program counter, and `SP` with the initial value of the stack pointer.

For example, to run the simulator with the code in `examples/initvars.txt` and store the result in a file called `sum.out`, run the command
```bash
./simulator examples/initvars.txt 0x71c 0xFFF0 | tee initvars.out
```

You can compare your simulator's output with the expected output using `diff`. For example:
```bash
diff initvars.out examples/initvars.log
```
If the output matches, then `diff` will not produce any output; otherwise, it will show where the output differs.

## Operand struct and helper functions
Your first task is to complete three operand helper functions in `machine.c`: `get_value`, `put_value`, and `get_memory_address`.  Each of these functions takes a `struct operand_t` and performs a task related to the operand.

### struct operand_t
A `struct operand_t` (defined in `code.h`) has four fields:
* `type` stores the operand's type. The types are defined by the `OPERAND_*` constants.
* `reg_type` stores the type of register contained in an operand whose `type` is `OPERAND_register` or `OPERAND_memory`. The register types are defined by the `REGISTER_*` constants.
* `reg_num` stores the number of a general purpose register (`0` through `30`) when the `reg_type` is `REGISTER_w` or `REGISTER_x`.
* `constant` stores a value whose meaning depends on the value in the `type` field.
    | Operand's `type`   | Meaning of `constant`     |
    |--------------------|---------------------------|
    | `OPERAND_constant` | Decimal number            |
    | `OPERAND_offset`   | Memory offset             |
    | `OPERAND_address`  | Address of an instruction |
    | `OPERAND_register` | _Not used_                |

Here are some example operands and the values stored in the corresponding `struct operand_t`:
* `w13`
    * `type` is `OPERAND_register`
    * `reg_type` is `REGISTER_w`
    * `reg_number` is `13`
    * `constant` is not used
* `pc`
    * `type` is `OPERAND_register`
    * `reg_type` is `REGISTER_pc`
    * `reg_number` is not used
    * `constant` is not used
* `#0x15`
    * `type` is `OPERAND_constant`
    * `reg_type` is not used
    * `reg_number` is not used
    * `constant` is `21`
* `[sp, #20]`
    * `type` is `OPERAND_memory`
    * `reg_type` is `REGISTER_sp`
    * `reg_number` is not used
    * `constant` is `20`
* `[x9]`
    * `type` is `OPERAND_memory`
    * `reg_type` is `REGISTER_x`
    * `reg_number` is `9`
    * `constant` is `0` 

### get_value
The `get_value` function should return the value associated with a constant, address, or register operand. For a register operand, the function must get the register value from the appropriate field in the global variable `struct system_t machine` declared in `machine.c`. For operands corresponding to `w` registers (i.e., `type` is `OPERAND_register` and `reg_type` is `REGISTER_w`), `get_value` must return only the lower 32-bits of the 64-bit registered value stored in the `machine` struct (i.e., the upper 32-bits of the returned value must be zeros).

We recommend you use a `switch` statement on the `reg_type` field, instead of a chain of `if`-`else if` statements, to handle the different types of registers. Read [Section 2.9.1](https://diveintosystems.org/book/C2-C_depth/advanced_switch.html) in _Dive Into Systems_ to learn about `switch` statements. 

### put_value
The `put_value` function should store the provided value in the register specified by the operand. In other words, the value should be stored in the appropriate field in the `machine` struct. For operands corresponding to `w` registers (i.e., `type` is `OPERAND_register` and `reg_type` is `REGISTER_w`), `put_value` must store only the lower 32-bits of the value in the `machine` struct (i.e., the upper 32-bits of the stored value must be zeros).

Again, we recommend you use a `switch` statement on the `reg_type` field, instead of a chain of `if`-`else if` statements, to handle the different types of registers.

### get_memory_address
The `get_memory_address` function should get the memory address associated with a memory operand. This requires getting the value from the register specified by the `reg_type` and `reg_num` fields of the operand and adding the offset stored in the `constant` field of the operand.

Notice that your `get_value` and `get_memory_address` functions must both get a value from a register. The code from `get_value` should **NOT** be duplicated  in `get_memory_address`.

### Testing 
You should run the `test_operands` program to test your operand helper functions (`get_value`, `put_value` and `get_memory_address`). You may want to (but are not required to) add additional test cases. 

## Execute instructions
Your second task is to write code that simulates the execution of the [instructions supported by your simulator](#overview). Your code should go in the `execute` function (in `machine.c`) and in helper functions you create. When complete, your simulator should support all instructions represented by `OPERATION_*` constants in `code.h`.

We recommend you implement and test instructions in the order listed below.

### Arithmetic and bitwise instructions
All arithmetic and bitwise instructions supported by your simulator take three operands:
* The register in which to store the result
* The register in which the first value is stored
* The register in which the second value is stored, or a constant to use for the second value

(See [Section 9.3](https://diveintosystems.org/book/C9-ARM64/arithmetic.html) of _Dive Into Systems_ for details on arithmetic and bitwise instructions.)

### Load/store/move instructions
A move instruction takes two operands:
* The register in which to store the value
* A constant value or a register in which a value is currently stored

Load and store instructions take two operands:
* The destination or source register
* The source or destination memory address

(See [Section 9.2](https://diveintosystems.org/book/C9-ARM64/common.html) of _Dive Into Systems_ for details on load, store, and move instructions.)

The `stack` field in the `machine` struct represents the simulated stack segment of the program's memory. You can assume all memory addresses specified in load/store instructions will be located within the stack segment. The `stack_top` field in the `machine` struct stores the simulated memory address corresponding to the first byte of data in the `stack` field; the `stack_bot` field in the `machine` struct stores the simulated memory address corresponding to the last byte of data in the `stack` field.

The `stack` field in the `machine` struct is a pointer which stores the _real memory address_ where the first byte of the simulated stack is stored. In other words, the _real memory address_ in the `stack` field corresponds to the _simulated memory address _in the `stack_top` field. Given a simulated memory address, you will need to compute a corresponding real memory address based on the value in `stack_top` and the value in `stack`.

To read/write a 32-bit value starting at the computed real memory address, you will need to store the computed memory address in a variable with type `uint32_t *`. To read/write a 64-bit value starting at the computed real memory address, you will need to store the computed memory address in a variable with type `uint64_t *`. The `reg_type` field of the first operand for the load/store instruction determines whether you need to read/write a 32-bit or a 64-bit value.

### Branching instructions
Branch (`b`), branch and link (`bl`), and conditional branch (`b.ne`, `b.eq`, `b.lt`, `b.gt`, `b.le`, `b.ge`) instructions have only one operand: the address of the line of code to execute next.

A compare instruction (`cmp`) takes two operands:
* The register containing the first value to compare
* A constant value or register in which a value is currently stored to compare against the first value.
A compare instruction should set the appropriate condition flag (`CONDITION_ZERO`, `CONDITION_NEGATIVE`, or `CONDITION_POSITIVE`) in the `conditions` field in the `machine` struct. Conditional branch instructions (`b.ne`, `b.eq`, `b.lt`, `b.gt`, `b.le`, `b.ge`) should check the conditions field in the `machine` struct to determine whether to branch.

Return instructions (`ret`) do not have any operands.

(See [Section 9.4.1](https://diveintosystems.org/book/C9-ARM64/preliminaries.html) and [Section 9.5.0](https://diveintosystems.org/book/C9-ARM64/functions.html#_functions_in_assembly) of _Dive Into Systems_ for details on branch instructions.)

### Testing
You should test your execution of instructions by [running your simulator](#running-the-simulator) on the assembly code contained in the `examples` directory. Make sure you **specify the appropriate initial value for the program counter**. You can **always use `0xFFF0` for the initial value of the stack pointer**. The expected output is stored in the `.log` files in the `examples` directory.

## Program design

You **must follow good program design and coding practices**, including:
* Using constants: Use the constants defined in `code.h` and `machine.h`. **Do not** put constant values directly in your code.
* Using multiple functions: **Do not** put all of your code for executing instructions in the `execute` function. 
* Avoiding duplicated code: For example, **do not** put the same code for accessing register values in your `get_value` and `get_memory_address` functions. 
* Include comments: Each function (except for `main`), must be preceded by a short comment that describes what the function does.
* Making multiple commits to your git repository: **Do not** wait until your entire program is working before you commit it to your git repository. You should commit your code each time you write and debug a piece of functionality.

## Submission instructions
You should **commit and push** your updated files to your git repository. However, as noted above, do not wait until your entire program is working before you commit it to your git repository; you should commit your code each time you write and debug a piece of functionality. 

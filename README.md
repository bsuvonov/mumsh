# Mum Shell

## Overview

The **Mumsh Shell** is a basic command-line shell built using C, designed to provide essential features found in standard Unix shells such as `bash` and `sh`. The shell supports features like:

1. **Command Execution**: Supports simple commands like `ls`, `cat`, and `echo`.
2. **Input/Output Redirection**: Supports redirecting input and output using symbols such as `>` and `<`.
3. **Pipelining**: Allows chaining multiple commands with `|`, connecting the output of one command to the input of another.
4. **Built-in Commands**: Includes support for basic shell commands like:
   - `cd`: Change directory
   - `pwd`: Print current working directory
5. **Signal Handling**: Gracefully handles interrupts such as `Ctrl-C` without terminating the shell unexpectedly.
6. **Handling Quotes in Commands**: The shell supports single (`'`) and double (`"`) quotes.

---

## Installation and Build

### Prerequisites:
Ensure that you have the necessary development tools installed:

```bash
sudo apt update
sudo apt install clang make gcc
```

### Build Instructions:
1. Clone the repository
```bash
git clone git@github.com:Bunyod-Suvonov/mumsh.git
```
2. Navigate to the project directory and build the shell. You can use the Makefile:
```bash
cd mumsh
make
```
This will generate two executables: mumsh and mumsh_memory_check.

3. Run the shell:
```bash
./mumsh
```

## Testing

Testing is important to ensure that the shell behaves correctly under various conditions. You can test the shell manually by running a variety of commands with different combinations of redirection, piping, and built-in commands. Automated testing scripts can also be used by giving input through STDIN and analyzing the result printed to STDOUT of the program. 

The program is tested by school's online judge and got full scores for all testcases.


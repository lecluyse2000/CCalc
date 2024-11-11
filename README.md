# Final Project - EECS348  

This is a fork of the repository for my final project for EECS 348 at KU. The goal of this fork is to implement my own AST, improve my existing code, and add more functionality to the program.

## Project Objective  

> This project delves into the world of digital logic. You will develop a C++ program acting as a    
> simplified Boolean logic simulator. The aim of this project is to develop a program that    
> simulates the behavior of logic circuits, including operations such as AND, OR, NOT, NAND, and    
> XOR. The program should be able to handle complex logic circuits with multiple gates and   
> input/output signals. The project will provide students with comprehensive hands-on experience   
> in software engineering, emphasizing the development process from project planning to fully   
> realized product. While aiming for the functionality of a full-fledged circuit simulator, we'll focus   
> on evaluating Boolean expressions for an introductory learning experience.   
> 
> This project provides an opportunity to explore the fascinating concepts of logic gates, truth   
> tables, and expression evaluation. You'll gain valuable skills in parsing, data structures,   
> algorithm design, and software engineering principles.   

## Quick Links   

- [Installation](#installation)
- [Quickstart Guide](#start-guide)
  * [Flags](#flags)
- [Build from Source](#building-from-source)
  * [Dependencies](#required-dependencies)
  * [Linux](#linux)
  * [macOS](#macos)
  * [Windows](#windows)
- [Members](#members)

## Installation   

This program requires [building from source](#building-from-source). You can download the latest version from the [release page](https://github.com/YAKU-Student/EECS-348---Group-Project/releases).    

## Start Guide   

Below are examples of general program usage. For a more in-depth look, please look at the [User Manual](https://github.com/YAKU-Student/EECS-348---Group-Project/blob/main/doc/06-Users-Manual.pdf).    

```console
user@archlinux:~$ boolean_simulator 'T & F'
Result: False!

user@archlinux:~$ boolean_simulator -c 'T & F'
Expected 1 argument, received 2. Use the --help flag to see all flags, or pass in an expression.
Make sure to wrap the expression in quotes.

user@archlinux:~$ boolean_simulator -c
Please enter your boolean expression, or enter help to see all available commands: T & F
Result: False!

Please enter your boolean expression, or enter help to see all available commands: T @ F
Result: True!

Please enter your boolean expression, or enter help to see all available commands: (T | F) $ (F & T)
Result: True!

Please enter your boolean expression, or enter help to see all available commands: history
Expression: T & F
Result: False!
Expression: T @ F
Result: True!
Expression: (T | F) $ (F & T)
Result: True!

Please enter your boolean expression, or enter help to see all available commands: quit
Exiting...
```

### Flags

- Without any flags, the program will expect a boolean expression as the input. For example: `boolean_simulator 'T & F'`
- With the `-c` or `--continuous` flag, the program will run in continuous mode. The user will be prompted for boolean expressions to evaluate until exiting the program by typing `exit`, `quit`, or `q`. Passing in any other arguments along with `-c` will result in an error and the program will not run.
- The flag `-f` or `--file` runs the program in file mode. Launching the program in this mode will take a list of expressions from `expressions.txt` and place the results in `results.txt`. The `expressions.txt` file must be placed in the current working directory.
- With the `-v` or `--version` flag. The program simply displays the version information of the program.    
- The `--help` flag prints a screen explaining all the flags and general program usage.

## Building from source

### Required dependencies

- A valid c++ compiler, we recommend [clang++](https://clang.llvm.org/), but you can use [g++](https://gcc.gnu.org/)   
  - [LLVM](https://www.llvm.org/)
    * [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
    * [clang++](https://clang.llvm.org/)
- [CMake](https://cmake.org/)

### Linux

Dependencies    

```bash
sudo apt-get install cmake
sudo apt install clang
```

Build    

```bash
cmake -B build && cmake --build build
```

Run    

```bash
build/boolean_simulator
```

### macOS

Dependencies

```bash
brew install cmake
brew install clang
```

Build

```bash
cmake -B build && cmake --build build
```

Run

```bash
build/boolean_simulator
```

### Windows

Use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install) and install via [Linux](#linux)    

If you don't want to use wsl CMake, Ninja, and clang++ is a good option.  

Or, you can use your own choice of installation for llvm/gcc and cmake.

```powershell
cmake -B build && cmake --build build
```

OR

```powershell
cmake -G Ninja -B build && cmake --build build
```

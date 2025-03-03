# CCalc - Caden's Calculator

CCalc is a CLI tool capable of solving both boolean and arithmetic expressions.

## Quick Links   

- [Installation](#installation)
- [Quickstart Guide](#start-guide)
  * [Operators](#operators)
  * [Flags](#flags)
  * [Configuration](#configuration)
- [Build from Source](#building-from-source)
  * [Linux](#debian)
  * [macOS](#macos)
  * [Windows](#windows)
- [Members](#members)

## Installation   

This program requires [building from source](#building-from-source). You can download the latest version from the [release page](https://github.com/YAKU-Student/EECS-348---Group-Project/releases).    

## Start Guide   

Below are examples of general program usage.

```console
user@archlinux:~$ ccalc 'T & F'
Result: False

user@archlinux:~$ ccalc -c 'T & F'
Expected 1 argument, received 2. Use the --help flag to see all flags, or pass in an expression.
Make sure to wrap the expression in quotes.

user@archlinux:~$ ccalc -c
Please enter your expression, or enter help to see all available commands: T & F
Result: False
Please enter your expression, or enter help to see all available commands: T @ F
Result: True
Please enter your expression, or enter help to see all available commands: (T | F) $ (F & T)
Result: True
Please enter your expression, or enter help to see all available commands: 2 + 3 - 3 * 3
Result: -4
Please enter your expression, or enter help to see all available commands: 3^2 / 3
Result: 3
Please enter your expression, or enter help to see all available commands: history
Expression: T & F
Result: False
Expression: T @ F
Result: True
Expression: (T | F) $ (F & T)
Result: True
Expression: 2 + 3 - 3 * 3
Result: -4
Expression: 3^2 / 3
Result: 3

Please enter your expression, or enter help to see all available commands: quit
Exiting...
```

### Operators

#### Boolean

* & - AND
* | - OR
* $ - XOR
* @ - NAND
* ! - NOT

#### Arithmetic

* \+ - Addition
* \- - Subtraction
* \* - Multiplication
* / - Division
* ^ - Exponent
* ! - Factorial

### Flags

- Without any flags, the program will expect a boolean or arithmetic expression as the input. For example: `ccalc 'T & F'` or `ccalc '2 + 2'`
- With the `-c` or `--continuous` flag, the program will run in continuous mode. The user will be prompted for expressions to evaluate until exiting the program by typing `exit`, `quit`, or `q`. Passing in any other arguments along with `-c` will result in an error and the program will not run.
- The flag `-f` or `--file` runs the program in file mode. Launching the program in this mode will take a list of expressions from `expressions.txt` and place the results in `results.txt`. The `expressions.txt` file must be placed in the current working directory.
- With the `-v` or `--version` flag. The program simply displays the version information of the program.    
- The `--help` flag prints a screen explaining all the flags and general program usage.

### Configuration

- All configuration is done in `~/.config/ccalc/settings.ini`
- The `precision=` field is set in bits, and it modifies the precision of internal computations (default = 320).
- The `display_digits=` field is set in digits, and it modifies the precision when printing the result (default = 20).

## Building from source

### Required dependencies

- A valid c++ compiler, I recommend [g++](https://gcc.gnu.org/)   
  - [LLVM](https://www.llvm.org/)
    * [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
    * [clang++](https://clang.llvm.org/)
- [CMake](https://cmake.org/)
- [GNU GMP](https://gmplib.org/)
- [GNU MPFR](https://www.mpfr.org/)

### Debian

```bash
sudo apt-get install cmake
sudo apt install build-essential libgmp-dev libmpfr-dev
```

### Fedora 

```bash
sudo dnf group install "Development Tools"
sudo dnf install cmake gmp-devel mpfr-devel 
```

### Arch

```bash
sudo pacman -S base-devel cmake gmp mpfr
```

Build    

```bash
cmake -B build && sudo cmake --build build --target install
```

Run    

```bash
ccalc
```

### macOS

Dependencies

```bash
xcode-select --install
brew install cmake gmp mpfr
```

Build

```bash
cmake -B build && sudo cmake --build build --target install
```

Run

```bash
ccalc
```

### Windows

Use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install) and install via [Linux](#Debian)    
Windows is not officially supported, but I have made an effort for it to work     
If wsl is not an option, CMake, Clang, and Ninja might work    
You will also have to figure out how to install and link GMP and MPFR     
Visual Studio/VScode might make this a lot easier, I am not sure though as I don't use Windows

# CCalc - Caden's Calculator

CCalc is a powerful CLI tool capable of solving both boolean and arithmetic expressions.

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

## Installation   

This program requires [building from source](#building-from-source). You can download the latest version from the [release page](https://github.com/lecluyse2000/CCalc/releases).    

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

&   AND   
|   OR   
$   XOR   
@   NAND   
!   NOT   

#### Arithmetic

\+   Addition   
\-   Subtraction   
\*   Multiplication   
/   Division   
^   Exponent   
!   Factorial   
Why no sqrt? Because sqrt(x) = x^(1/2)   
`pi` and `e` are supported variables   
`sin`, `cos`, and `tan` are supported in radians   

### Flags

- Without any flags, the program will expect a boolean or arithmetic expression as the input. For example: `ccalc 'T & F'` or `ccalc '2 + 2'`
- With the `-c` or `--continuous` flag, the program will run in continuous mode. The user will be prompted for expressions to evaluate until exiting the program by typing `exit`, `quit`, or `q`. Passing in any other arguments along with `-c` will result in an error and the program will not run.
- The flag `-f` or `--file` runs the program in file mode. You will be prompted for an input file, and the input file must be placed in the current working directory. The input file must contain an expression on each line. The program will then prompt you for an output file name and put the results in that file.
- With the `-v` or `--version` flag. The program simply displays the version information of the program.    
- The `--help` flag prints a screen explaining all the flags and general program usage.

### Continuous Mode

In continuous mode, there are four commands available:

1. `history` prints the program history to the screen.
2. `save` prompts you for a filename, then outputs the program history to that file.
3. `clear` clears the history.
4. `exit`, `quit`, or `q` exits the program.

### Configuration

- All configuration is done in `~/.config/ccalc/settings.ini`
- The `precision=` field is set in bits, and it modifies the precision of internal computations (default = 320).
- The `display_digits=` field is set in digits, and it modifies the precision when printing the result (default = 15).
- The `max_history=` field is set using a positive integer, and it modifies how many entries you can store in the program history (default = 50).
- The `angle=` field sets whether the program uses radians are degrees. Enter 0 for radians, 1 for degrees (default = 0).

```ini
[Settings]
precision=320
display_digits=15
max_history=50
angle=1
```

## Building from source

### Required dependencies

- A valid c++ compiler, I recommend [g++](https://gcc.gnu.org/)   
  - [LLVM](https://www.llvm.org/)
    * [clang-format](https://clang.llvm.org/docs/ClangFormat.html)
    * [clang++](https://clang.llvm.org/)
- [CMake](https://cmake.org/)
- [GNU GMP](https://gmplib.org/)
- [GNU MPFR](https://www.mpfr.org/)

#### Debian

```bash
sudo apt-get install cmake
sudo apt install build-essential libgmp-dev libmpfr-dev
```

#### Fedora 

```bash
sudo dnf group install "Development Tools"
sudo dnf install cmake gmp-devel mpfr-devel 
```

#### Arch

```bash
sudo pacman -S base-devel cmake gmp mpfr
```

#### MacOS

```bash
xcode-select --install
brew install cmake gmp mpfr
```

### Build    

```bash
cmake -B build && sudo cmake --build build --target install
```

By default, CCalc is installed in `/usr/local/bin` on Mac and `/usr/bin` on Linux. To change the installation directory, pass in the `-DCMAKE_INSTALL_PREFIX` flag to cmake. For example:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=~/Documents/ccalc && sudo cmake --build build --target install
```

### Run    

```bash
ccalc -c
```

### Windows

Use [wsl](https://learn.microsoft.com/en-us/windows/wsl/install) and install via [Linux](#Debian)    
Windows is not officially supported, but I have made an effort for it to work. If wsl is not an option, CMake, Clang, and Ninja might work.
You will also have to figure out how to install GMP and MPFR. Visual Studio/VScode might make this a lot easier, I am not sure though as I don't use Windows.

## TODO

* Implement trig function support (degrees and rads)
* ~~Include e and pi as predefined variables~~
* Add an 'ans' variable that stores the previous answer
* Add the ability to store values in a variable
* Give the user the ability to print in scientific notation
* Add modulus support
* Introduce the ability for the program to generate truth tables
* Develop equation solving capabilities (probably won't happen)
* Add support for higher level calculus concepts like integrals (also probably won't happen)
* Add more options to settings to make the program as customizable as possible

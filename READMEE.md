# Custom Language Compiler - CSE332 Compiler Design Project

A complete compiler implementation featuring a **DFA-based lexical analyzer** and **LL(1) parser** for a custom programming language. This project demonstrates the fundamental principles of compiler design including tokenization, syntax analysis, and grammar validation.

## 🎯 Project Overview

This compiler processes a custom programming language with C-like syntax and performs:
- **Lexical Analysis**: Token recognition using a 36-input Deterministic Finite Automaton (DFA)
- **Syntax Analysis**: Grammar validation using an LL(1) parsing table
- **Error Detection**: Comprehensive syntax error reporting

### Custom Language Features

The language supports:
- **Data Types**: `int`, `dec` (decimal)
- **Functions**: Custom functions ending with `Fn` (e.g., `computeValueFn`)
- **Control Flow**: `while` loops with labeled loop constructs
- **Variables**: Pattern `_[a-zA-Z]+[0-9][a-zA-Z]` (e.g., `_val1a`, `_temp2x`)
- **Statements**: `printf`, `return`, `break`
- **Statement Terminator**: `..` (double dot)
- **Loop Labels**: `loop_<name><2digits>:` (e.g., `loop_main01:`)

## 📂 Repository Structure

```
Compiler-Design-Project/
├── all.c                    # Main compiler source code (1510 lines)
├── example1.c               # Test: Function with loop
├── example2.c               # Test: Main function with loop
├── example3.c               # Test: Simple main function
├── compile_and_run.bat      # Automated build script (Windows)
├── README.md                # This file
└── FIX_SUMMARY.md           # Technical documentation of bug fixes
```

## 🚀 Quick Start

### Prerequisites

You need a C compiler. Choose one:
- **MinGW-w64** (GCC for Windows): [Download](https://www.mingw-w64.org/)
- **Visual Studio** (MSVC): [Download](https://visualstudio.microsoft.com/)
- **Online Compiler**: [OnlineGDB](https://www.onlinegdb.com/online_c_compiler) or [Programiz](https://www.programiz.com/c-programming/online-compiler/)

### Compilation & Execution

#### Method 1: Automated Script (Windows)
```bash
# Simply double-click or run:
compile_and_run.bat
```

#### Method 2: Manual Compilation (GCC)
```bash
gcc -o compiler.exe all.c
./compiler.exe
```

#### Method 3: Manual Compilation (MSVC)
```bash
cl all.c /Fe:compiler.exe
compiler.exe
```

#### Method 4: Online Compiler
1. Go to [OnlineGDB](https://www.onlinegdb.com/online_c_compiler)
2. Upload `all.c`
3. Click "Run"
4. Paste example code when prompted
5. Type `END` and press Enter

## 📝 Usage Instructions

1. **Run the compiler** - The program will display:
   - DFA transition matrix
   - Grammar productions
   - LL(1) parsing table

2. **Enter your code** - Type or paste code from example files

3. **End input** - Type `END` on a new line

4. **View results**:
   - Lexical analysis (tokenization)
   - Syntax analysis (parsing)
   - Final verdict: **ACCEPTED ✓** or **REJECTED ✗**

### Example Session

```
$ ./compiler.exe
[DFA and Grammar tables displayed...]

Enter code (type END to finish):
#include <stdio.h>

int main(){dec _input3k = 10.. dec _result4m = 2.. printf(_rt4c)..return 0..}
END

Lexer DFA Output:
=================
Token: I (#include<stdio.h>)
Token: T (int)
Token: M (main)
...

Parser LL(1) Output:
====================
...
Result: ACCEPTED ✓
```

## 📚 Example Programs

### Example 1: Function with Loop
**File**: `example1.c`

```c
#include <stdio.h>

dec computeValueFn(dec _val1a) { dec _temp2x = _val1a + 5.. return _temp2x.. }

int main() {
  dec _input3k = 10.. dec _result4m = computeValueFn(_input3k)..

                                      loop_main01
      : while (dec _loopin0x < 3..) {
    printf(_result4m)..break..
  }

  return 0..
}
```

**Features Demonstrated**:
- Custom function definition (`computeValueFn`)
- Function parameters and return values
- Variable declarations
- Loop label (`loop_main01:`)
- While loop with condition
- Printf statement
- Break statement

---

### Example 2: Main with Loop
**File**: `example2.c`

```c
#include <stdio.h>

int main() {
  dec _input3k = 10.. dec _result4m = 2..

      loop_main01 : while (int _m7x < 3..) {
    printf(_rt4c)..break..
  }

  return 0..
}
```

**Features Demonstrated**:
- Main function
- Multiple variable declarations
- Loop label with while loop
- Printf and break statements

---

### Example 3: Simple Main
**File**: `example3.c`

```c
#include <stdio.h>

int main()
{
    dec _input3k = 10.. 
    dec _result4m = 2.. 
    printf(_rt4c)..
    return 0..
}
```

**Features Demonstrated**:
- Clean multi-line syntax
- Basic variable declarations
- Printf statement
- Return statement

All three examples are **ACCEPTED ✓** by the compiler.

## 🔧 Technical Details

### Token Types

| Token | Symbol | Description | Example |
|-------|--------|-------------|---------|
| **I** | Include | `#include<stdio.h>` | `#include<stdio.h>` |
| **T** | Type | Data types | `int`, `dec` |
| **F** | Function | Function names ending with `Fn` | `computeValueFn` |
| **V** | Variable | Pattern: `_[a-zA-Z]+[0-9][a-zA-Z]` | `_val1a`, `_temp2x` |
| **N** | Number | Numeric literals | `10`, `5`, `3` |
| **P** | Printf | Print statement | `printf` |
| **W** | While | While loop | `while` |
| **K** | Break | Break statement | `break` |
| **R** | Return | Return statement | `return` |
| **M** | Main | Main function | `main` |
| **L** | Loop | Loop label | `loop_main01:` |
| **B** | Bracket | Parentheses, braces | `(`, `)`, `{`, `}` |
| **O** | Operator | Operators | `=`, `<`, `+`, `,`, `:` |
| **S** | Statement | Statement terminator | `..` |

### DFA Specifications

- **States**: 84 (D0 to D82 + DEAD state)
- **Input Symbols**: 36 distinct characters/patterns
- **Accepting States**: 15 states for different token types
- **Transition Table**: 84 × 36 matrix

### Grammar Productions

The compiler uses an LL(1) grammar with productions for:
- Program structure
- Function definitions
- Variable declarations
- Control flow statements
- Expressions and operators

### Architecture

```
┌─────────────────┐
│  Source Code    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Lexical        │
│  Analyzer       │ ◄── 36-Input DFA
│  (Tokenizer)    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Syntax         │
│  Analyzer       │ ◄── LL(1) Parser
│  (Parser)       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  ACCEPTED ✓     │
│  or             │
│  REJECTED ✗     │
└─────────────────┘
```

## 🐛 Bug Fixes & Improvements

This compiler has been thoroughly debugged and tested. Major fixes include:

1. ✅ **NUM_INPUTS Mismatch** - Fixed DFA input count (8 → 36)
2. ✅ **NUM_STATES Correction** - Updated state count (65 → 84)
3. ✅ **Duplicate DFA Systems** - Removed conflicting DFA code
4. ✅ **Function Call Errors** - Fixed `get_input_index()` → `get_input()`
5. ✅ **DFA Table Reference** - Corrected table name usage
6. ✅ **Printf Grammar** - Fixed production rule for printf statements

See `FIX_SUMMARY.md` for complete technical details.

## 📖 Language Syntax Reference

### Variable Naming Rules
- Must start with underscore `_`
- Followed by one or more letters (a-z, A-Z)
- Followed by exactly one digit (0-9)
- Followed by one or more letters (a-z, A-Z)
- Examples: `_val1a`, `_temp2x`, `_input3k`, `_result4m`

### Function Naming Rules
- Must end with `Fn`
- Only alphabetic characters allowed
- Examples: `computeValueFn`, `calculateFn`, `processFn`

### Loop Label Rules
- Pattern: `loop_<name><2digits>:`
- Must start with `loop_`
- Followed by letters/underscores
- Ends with exactly 2 digits and a colon
- Examples: `loop_main01:`, `loop_process99:`

### Statement Terminator
- All statements must end with `..` (double dot)
- Examples: `dec _x1a = 5..`, `return 0..`, `break..`

### Complete Program Template

```c
#include <stdio.h>

dec functionNameFn(dec _param1a) {
    dec _local2b = _param1a + 5..
    return _local2b..
}

int main() {
    dec _var1x = 10..
    dec _result2y = functionNameFn(_var1x)..
    
    loop_main01 : while (dec _counter3z < 5..) {
        printf(_result2y)..
        break..
    }
    
    return 0..
}
```

## 🎓 Educational Value

This project demonstrates:
- **Finite Automata Theory**: DFA construction and implementation
- **Formal Languages**: Grammar design and LL(1) parsing
- **Compiler Construction**: Multi-phase compilation process
- **Data Structures**: State transition tables, parsing tables
- **Algorithm Design**: Token recognition, syntax validation

## 🤝 Contributing

This is an educational project for CSE332 Compiler Design Lab. Feel free to:
- Report bugs or issues
- Suggest improvements
- Fork and experiment with the grammar
- Add new language features

## 📄 License

This project is created for educational purposes as part of CSE332 Compiler Design coursework.

## 🙏 Acknowledgments

- CSE332 Compiler Design Lab Course
- Compiler Design principles and theory
- DFA and LL(1) parsing techniques

---

For technical details about bug fixes, see [FIX_SUMMARY.md](FIX_SUMMARY.md)

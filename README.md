# CVolt
CVolt - compiler in C++ with LLVM integration, designed for embedded systems and for JIT compilation and calling volt functions in C++.

## Features

- LLVM IR generation
- JIT compilation
- Function overloading
- Arrays
- Pointers
- References
- Const qualifiers
- Type checking
- Compile-time evaluation
- Arena allocator

## Build project
```bash
cd CVolt
mkdir build
cd build
cmake .. -DLLVM_DIR="/path/to/llvm/lib/cmake/llvm"
ninja
```

## Build to static library
```bash
cmake .. -DCOMPILE_TO_LIB=ON
```

## Project structure
```
Volt
|- ADT (Custom containers)
|  |- Array
|  |- FixedMap
|  |- String
|  |- ArrayAllocator
|  |- ArrayIterator
|- Core (Core of project)
|  |- AST: contains AST nodes
|  |- BuiltinsFunctions: contains BuiltinFunctionTable to add user defined functions
|  |- CompilationContext: used for centralized memory management
|  |- Errors: lex, parse, type errors
|  |- Functions: contains callee for function tables in TypeChecker and LLVMCompiler
|  |- Hash: hash for DataType, QualType, FunctionSignature
|  |- Memory: contains Arena
|  |- Object: base class for all polymorphic classes
|  |- Types: DataType class and TypeConversion
|  |- Lexer
|  |- Parser
|  |- TypeChecker
|  |- TypeDefs
|  |- Enums
|- Compiler (IR gen)
|  |- Types: contains CompilerTypes
|  |- Functions: contains FunctionSignature
|  |- LLVMCompiler
|- Debug (for debug output)
|- Runtime (JITEngine for executing code in runtime)
|- Tests (ParserFuzzer)
```

## Example of integration with C++

```c++
// Init JITEngine
Volt::JITEngine::Init();

Volt::CompilationContext CContext(Code /*Source Code*/, "test.volt" /*File Name*/);

Volt::BuiltinFunctionTable FuncTable(CContext);
FuncTable.AddFunction("Out", "OutInt", &OutInt);
FuncTable.AddFunction("Out", "OutFloat", &OutFloat); // Builtin function overload

// Lexing
Volt::Lexer MyLexer(CContext);
MyLexer.Lex();

if (MyLexer.PrintErrors())
    return -1;

// Parsing
Volt::Parser MyParser(CContext);
MyParser.Parse();

if (MyParser.PrintErrors())
    return -1;

// Type Checking
Volt::TypeChecker MyTypeChecker(CContext, FuncTable);
MyTypeChecker.Check();

if (MyTypeChecker.PrintErrors())
    return -1;

// Compiling
Volt::LLVMCompiler MyCompiler(CContext, FuncTable);
MyCompiler.Compile();

// Calling function
Volt::JITEngine Engine(CContext, FuncTable);
int Res = Engine.CallFunction<int>("Main");
```

## UnitTests
To run the automated test suite, ensure the correct path to your compiled binary is set in `tests_config.json`:
```json
{
  "path_to_binary": "path/to/binary/CVolt"
}
```
then run this script:
```bash
python unit_tests.py
```

## Base syntax

### Variables
```
let:<type> <VariableName> = <value>
```

### Functions
```
fun:<type> <FunctionName>(<type> <ParameterName>)
{
	Body
}
```

### if/else
```
if (Condition)
{
	If body
}
else
{
	Else body
}
```

### while loop
```
while (Condition)
{
	Body
}
```

### for loop
```
for (Initialization; Condition; Iteration)
{
	Body
}
```

### Comments
```c++
// Line comment
/*
	Block comment
*/
```

## Examples

### Print "Hello, World!"
```c++
fun:int Main()
{
	Out("Hello, World!");
	return 0;
}
```

### If/Else
```c++
fun:int Main()
{
	let:int Num = 5;
	if (Num < 10)
		Out("Less than ten");
	else
		Out("Greater or equal ten");

	return 0;
}
```

### While
```c++
fun:int Main()
{
	let:int Num = 0;
	while (Num < 10)
	{
		Out(Num);
		Num++;
	}

	return 0;
}
```

### For
```c++
fun:int Main()
{
	let:int[5] Arr = [1, 2, 3, 4, 5];
	for (let:int i = 0; i < 5; i++)
		Out(Arr[i]);

	return 0;
}
```

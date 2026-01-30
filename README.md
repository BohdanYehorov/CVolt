# CVolt - compiler in C++ with LLVM integration
This project was created as an educational project to understand how a lexer, parser, and compiler work.

## Build project
```bash
cd CVolt
mkdir build
cd build
cmake .. -DLLVM_DIR="/path/to/llvm/lib/cmake/llvm"
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
```
// Line comment
/*
	Block comment
*/
```

## Examples

### Print "Hello, World!"
```
fun:int Main()
{
	Out("Hello, World!");
	return 0;
}
```

### If/else
```
fun:int Main()
{
	let:int Num = 5;
	if (Num < 10)
		Out("Less then ten");
	else
		Out("Greater or equal ten");

	return 0;
}
```

### While
```
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
```
fun:int Main()
{
	let:int[5] Arr = [1, 2, 3, 4, 5];
	for (let:int i = 0; i < 5; i++)
		Out(Arr[i]);

	return 0;
}
```

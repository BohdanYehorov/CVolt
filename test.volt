fun:int Main()
{
	let:int* ptr = int*(MemAlloc(32));
	
	*(ptr + 1) = 10;
	Out(ptr[1]);

	MemFree(ptr);
	return 0;
}
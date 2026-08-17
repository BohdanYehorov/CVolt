fun:i32 Main()
{
    let:i32* ptr = MemAlloc(16u64) to i32*;
    MemFree(ptr);
    return 0;
}
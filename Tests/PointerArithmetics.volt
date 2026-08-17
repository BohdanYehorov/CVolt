fun:i32 Main()
{
    let:i32* Ptr = MemAlloc(sizeof(i32) * 2u64) to i32*;
    *Ptr = 2;
    *(Ptr + 1) = 10;
    OutLine(Ptr[0]);
    OutLine(Ptr[1]);
    MemFree(Ptr);

    return 0;
}

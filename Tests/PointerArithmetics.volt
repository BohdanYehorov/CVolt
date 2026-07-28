fun:i32 Main()
{
    let:i32* Ptr = i32*(MemAlloc(8u64));
    *Ptr = 2;
    *(Ptr + 1) = 10;
    OutLine(Ptr[0]);
    OutLine(Ptr[1]);
    MemFree(Ptr);

    return 0;
}

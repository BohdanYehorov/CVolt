fun:i32 Main()
{
    let:i32* Ptr = i32*(MemAlloc(8));
    *Ptr = 2;
    *(Ptr + 1) = 10;
    Out(Ptr[0]);
    Out(Ptr[1]);
    MemFree(Ptr);

    return 0;
}

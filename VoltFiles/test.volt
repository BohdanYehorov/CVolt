fun:int Main()
{
    let:int* Ptr = int*(MemAlloc(4));
    let:int Num = 6;
    MemCpy(Ptr, $Num, 4);
    Out(*Ptr);
    MemFree(Ptr);

    return 0;
}
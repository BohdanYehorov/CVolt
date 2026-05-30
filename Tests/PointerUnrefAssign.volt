fun:int Main()
{
    let:int Num = 5;
    let:int* Ptr = $Num;
    *Ptr = 8;
    Out(Num);
    return 0;
}
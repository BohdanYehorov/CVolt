fun:i32 Main()
{
    let:i32 Num = 5;
    let:i32* Ptr = $Num;
    *Ptr = 8;
    OutLine(Num);
    return 0;
}
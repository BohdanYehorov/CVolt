fun:i32 Main()
{
    let:i32 Num = 5;
    let:i32$ Ref = Num;
    Ref = 10;
    OutLine(Ref);
    OutLine(Num);
    Num = 15;
    OutLine(Num);
    OutLine(Ref);
    return 0;
}
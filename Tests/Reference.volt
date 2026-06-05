fun:i32 Main()
{
    let:i32 Num = 5;
    let:i32$ Ref = Num;
    Ref = 10;
    Out(Ref);
    Out(Num);
    Num = 15;
    Out(Num);
    Out(Ref);
    return 0;
}
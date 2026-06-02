fun:int Main()
{
    let:int Num = 5;
    let:int$ Ref = Num;
    Ref = 10;
    Out(Ref);
    Out(Num);
    Num = 15;
    Out(Num);
    Out(Ref);
    return 0;
}
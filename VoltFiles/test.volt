fun:void Hello(int$ Num)
{
    Num = 6;
}

fun:int Main()
{
    let:int Num;
    Hello(Num);
    Out(Num);
    return 0;
}
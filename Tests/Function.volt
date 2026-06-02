fun:void OutHello()
{
    Out("Hello!");
}

fun:int Add(int a, int b)
{
    return a + b;
}

fun:void SetValue(int$ Ref, int Value)
{
    Ref = Value;
}

fun:int Main()
{
    OutHello();
    Out(Add(2, 5));

    let:int Num = 5;
    SetValue(Num, 10);
    Out(Num);

    return 0;
}
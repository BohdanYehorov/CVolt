fun:void OutHello()
{
    OutLine("Hello!");
}

fun:i32 Add(i32 a, i32 b)
{
    return a + b;
}

fun:void SetValue(i32$ Ref, i32 Value)
{
    Ref = Value;
}

fun:i32 Main()
{
    OutHello();
    OutLine(Add(2, 5));

    let:i32 Num = 5;
    SetValue(Num, 10);
    OutLine(Num);

    return 0;
}
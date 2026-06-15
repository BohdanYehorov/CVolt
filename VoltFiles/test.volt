fun:void SetValue(i32$ Ref, i32 Value)
{
    Ref = Value;
}

fun:i32 Main()
{
    let:i32 Num = 5;
    SetValue(Num, 10);
    Out(Num);

    return 0;
}
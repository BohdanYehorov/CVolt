fun:void A(i32 Num)
{
    if (Num <= 0)
        return;

    Out("A\n");
    B(Num - 1);
}

fun:void B(i32 Num)
{
    if (Num <= 0)
        return;

    Out("B\n");
    A(Num - 1);
}

fun:i32 Main()
{
    A(5);
    return 0;
}
fun:i32 Main()
{
    let:i32 i = 0;
    while (i < 10000)
    {
        Out(i);
        i += 2;
    }

    return 0;
}
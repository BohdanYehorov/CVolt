fun:i32 Main()
{
    let:i32 i = 0;

    while (true)
    {
        i++;
        if (i % 10 == 0)
            continue;

        if (i > 1000)
            break;

        Out(i);
    }

    return 0;
}
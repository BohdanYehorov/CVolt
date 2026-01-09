int Main()
{
    for (int i = 0; i < 100; i++)
    {
        if (i > 10 && i < 50)
            continue;

        OutInt(i);
        OutStr("\n");
    }
    return 0;
}
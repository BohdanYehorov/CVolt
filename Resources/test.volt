int main()
{
    int a = 0;
    while (a < 100)
    {
        a++;
        if (a == 50)
            continue;

        Out(a);
    }
    return 0;
}
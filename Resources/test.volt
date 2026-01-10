int Factorial(int N)
{
    if (N == 1 || N == 0)
        return 1;

    return N * Factorial(N - 1);
}

int Main()
{
    OutStr("Hello, World");
    OutStr("Hello");
    return 0;
}
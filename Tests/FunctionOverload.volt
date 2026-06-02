fun:void f(int a)
{
    Out(1);
}

fun:void f(float a)
{
    Out(2);
}

fun:int Main()
{
    f(2b);
    f(5);
    f(5l);
    f(25.f);
    f(5.0);
    return 0;
}
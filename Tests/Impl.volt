class A
{
    fun:void PrintA()
    {
        OutLine("A");
    }
}

class B
{
    impl a: A;
    fun:void PrintB()
    {
        OutLine("B");
    }
}

class C
{
    impl b: B;
}

fun:i32 Main()
{
    let:C c;
    c.PrintA();
    c.PrintB();
    return 0;
}
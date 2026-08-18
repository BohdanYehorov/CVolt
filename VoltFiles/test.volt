class A
{
    num: i32;
    fun:void Hello()
    {
        this.num = 51;
        OutLine(this.num);
    }
}

class B
{
    num: i32;
    impl a: A;
}

class C
{
    impl b: B;
}

fun:i32 Main()
{
    let:B b;
    b.Hello();
    let:C c;
    c.Hello();
    return 0;
}
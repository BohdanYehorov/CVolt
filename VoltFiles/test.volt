class MyClass
{
    let:i32 a;

    MyClass()
    {
        this.a = 5;
        Out("MyClass()\n");
    }
}

fun:i32 Main()
{
    let:MyClass myClass;

    OutLine(myClass.a);
    return 0;
}
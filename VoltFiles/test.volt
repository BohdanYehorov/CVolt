class MyClass
{
    let:i64 a;
    let:i32 b;
};

fun:i32 Main()
{
    let:MyClass myClass;
    OutLine(sizeof myClass);
    OutLine(alignof type MyClass);
    return 0;
}
fun:void f(i32 a)
{
    Out(1);
}

fun:void f(f32 a)
{
    Out(2);
}

fun:i32 Main()
{
    f(2i8);
    f(5);
    f(5i64);
    f(25.f32);
    f(5.0);
    return 0;
}
fun:void f(void* ptr)
{}

fun:i32 Main()
{
    let:i32* p = null;
    f(p);
    return 0;
}
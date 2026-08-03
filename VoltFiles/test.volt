class MyClass
{
    let:i32 a = 5;
    fun:void Hello()
    {
        if (true)
        {
            let:i32 this = 5;
        }

        this.a = 10;
    }
}

fun:i32 Main()
{
    return 0;
}
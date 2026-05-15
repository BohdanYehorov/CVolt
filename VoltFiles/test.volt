fun:void Assign(int$ Target, int Value)
{
    Target = Value;
}

fun:int Main()
{
    let:int num = 5;
    Assign(num, 8);
    Out(num);
    return 0;
}
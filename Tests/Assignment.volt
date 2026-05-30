fun:int Main()
{
    let:int a = 5;
    a = 8;
    Out(a);
    a += 10;
    Out(a);
    a -= 2;
    Out(a);
    a *= 10;
    Out(a);
    a /= 5;
    Out(a);
    a |= 2;
    Out(a);
    a &= 1 << 1;
    Out(a);
    return 0;
}
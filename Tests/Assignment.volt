fun:i32 Main()
{
    let:i32 a = 5;
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
    a ^= 1;
    Out(a);
    a >>= 1;
    Out(a);
    a <<= 2;
    Out(a);
    a %= 3;
    Out(a);

    let:f64 b = 5.0;
    b = 8.0;
    Out(b);
    b -= 2.2;
    Out(b);
    b *= 5.2;
    Out(b);
    b /= b / 2;
    Out(b);

    return 0;
}
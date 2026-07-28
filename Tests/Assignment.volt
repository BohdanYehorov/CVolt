fun:i32 Main()
{
    let:i32 a = 5;
    a = 8;
    OutLine(a);
    a += 10;
    OutLine(a);
    a -= 2;
    OutLine(a);
    a *= 10;
    OutLine(a);
    a /= 5;
    OutLine(a);
    a |= 2;
    OutLine(a);
    a &= 1 << 1;
    OutLine(a);
    a ^= 1;
    OutLine(a);
    a >>= 1;
    OutLine(a);
    a <<= 2;
    OutLine(a);
    a %= 3;
    OutLine(a);

    let:f64 b = 5.0;
    b = 8.0;
    OutLine(b);
    b -= 2.2;
    OutLine(b);
    b *= 5.2;
    OutLine(b);
    b /= b / 2;
    OutLine(b);

    return 0;
}
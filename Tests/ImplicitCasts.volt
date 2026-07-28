fun:i32 Main()
{
    let:i32 a = 45i8;
    let:i8  b = 85;
    let:f32 c = 5.4;
    let:f32 d = a;
    let:f64 e = 58.4f32;
    let:i32 f = a + e;

    OutLine(a);
    OutLine(b);
    OutLine(c);
    OutLine(d);
    OutLine(e);
    OutLine(f);

    return 0;
}
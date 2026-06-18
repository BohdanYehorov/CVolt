class Vec2
{
    let:i32 x;
    let:i32 y;
};

fun:i32 Main()
{
    let:Vec2 a;
    a.x = 7;
    a.y = 8;
    Out(a.x);
    Out(a.y);
    return 0;
}
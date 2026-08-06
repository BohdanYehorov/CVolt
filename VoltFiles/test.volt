class Vec2
{
    let:i32 x;
    let:i32 y;
};

fun:void Print(Vec2 Vec)
{
    OutLine(Vec.x);
    OutLine(Vec.y);
}

fun:i32 Main()
{
    let:Vec2 v;
    v.x = 10;
    v.y = 11;

    Print(v);
    return 0;
}
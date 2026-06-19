class Vec2
{
    let:i32 x;
    let:i32 y;
};

fun:void Hello(Vec2$ Vec)
{
    Vec.x = 10;
}

fun:i32 Main()
{
    let:Vec2 Vec;
    Vec.y = 5;
    Hello(Vec);
    Out(Vec.x);
    return 0;
}
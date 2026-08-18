class Vec2
{
    x: f32;
    y: f32;

    Vec2(f32 x, f32 y)
    {
        this.x = x;
        this.y = y;
    }
}

fun:Vec2 GetVec()
{
    return Vec2(5.f32, 15.f32);
}

fun:i32 Main()
{
    let:Vec2 Vec = GetVec();
    OutLine(Vec.x);
    OutLine(Vec.y);
    return 0;
}
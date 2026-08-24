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

class MyClass
{
    fun:Vec2 GetVec()
    {
        return Vec2(45.f32, 158.f32);
    }
}

fun:Vec2 GetVec()
{
    return Vec2(5.f32, 15.f32);
}

fun:i32 Main()
{
    let:MyClass c;
    let:Vec2 Vec = c.GetVec();
    OutLine(Vec.x);
    OutLine(Vec.y);
    return 0;
}
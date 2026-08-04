class Vec2
{
    let:f32 x;
    let:f32 y;

    Vec2(i32 x, i32 y)
    {
        this.x = x;
        this.y = y;
    }

    fun:f32 LengthSquared()
    {
        return this.x * this.x + this.y * this.y;
    }

    fun:f32 Length()
    {
        return Sqrt(this.LengthSquared());
    }

    fun:f32 Dot(const Vec2$ Other)
    {
        return this.x * Other.x + this.y * Other.y;
    }

    fun:f32 Cross(const Vec2$ Other)
    {
        return this.x * Other.y - this.y * Other.x;
    }

    fun:f32 CosBetween(const Vec2$ Other)
    {
        return this.Dot(Other) / (this.Length() * Other.Length());
    }
}

fun:i32 Main()
{
    let:Vec2 Vec(3, 4);

    let:Vec2 Vec1(5, 10);

    OutLine(Vec.LengthSquared());
    OutLine(Vec.Length());
    OutLine(Vec.Dot(Vec1));
    OutLine(Vec.Cross(Vec1));
    OutLine(Vec.CosBetween(Vec1));

    return 0;
}
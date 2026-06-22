class Vec2
{
    let:f32 x;
    let:f32 y;

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
    let:Vec2 Vec;
    Vec.x = 3;
    Vec.y = 4;

    let:Vec2 Vec1;
    Vec1.x = 5;
    Vec1.y = 10;

    Out(Vec.LengthSquared());
    Out(Vec.Length());
    Out(Vec.Dot(Vec1));
    Out(Vec.Cross(Vec1));
    Out(Vec.CosBetween(Vec1));

    return 0;
}
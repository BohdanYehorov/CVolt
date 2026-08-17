class Vec2
{
    x: i32;
    y: i32;

    Vec2(i32 x, i32 y)
    {
        this.x = x;
        this.y = y;

        Out("Vec2\n");
    }
}

fun:i32 Main()
{
    let:Vec2 Vec = Vec2(4, 5);
    return 0;
}
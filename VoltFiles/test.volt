class Vec2
{
    let:i32 x;
    let:i32 y;

    Vec2()
    {
        this.x = 0;
        this.y = 0;
    }

    fun:void Hello() {}
};

fun:i32 Main()
{
    let:Vec2 v;
    Out(v.x);
    Out(v.y);
    return 0;
}
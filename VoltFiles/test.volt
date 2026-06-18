class Vec2
{
    let:i32 x;
    let:i32 y;
};

fun:i32 Main()
{
    let:Vec2* Ptr = Vec2*(MemAlloc(8));
    Ptr.x = 5;
    Ptr.y = 10;

    Out(Ptr.x);
    Out(Ptr.y);

    MemFree(Ptr);
    return 0;
}
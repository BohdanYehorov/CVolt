class A
{
    A(i32 Num) {}
}

fun:i32 Main()
{
    let:A a = A(4);
    let:A a1;
    a1 = A(8);
    let:A[2] arr;
    arr[0] = A(5);
    arr[1] = A(10);
    return 0;
}
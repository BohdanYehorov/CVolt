fun:i32 Main()
{
    let:i32[5] Arr = [1, 2, 3, 4, 5];
    Arr[2] = 10;
    Arr[3] = 8;

    OutLine(Arr[0]);
    OutLine(Arr[1]);
    OutLine(Arr[2]);
    OutLine(Arr[3]);
    OutLine(Arr[4]);
    return 0;
}
fun:int Main()
{
    let:int[5] Arr = [1, 2, 3, 4, 5];
    Arr[2] = 10;
    Arr[3] = 8;

    Out(Arr[0]);
    Out(Arr[1]);
    Out(Arr[2]);
    Out(Arr[3]);
    Out(Arr[4]);
    return 0;
}
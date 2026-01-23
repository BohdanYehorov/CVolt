fun:void BubbleSort(int* Arr, int Len)
{
    for (let:int i = 0; i < Len - 1; i++)
    {
        for (let:int j = 0; j < Len - i - 1; j++)
        {
            if (Arr[j] > Arr[j + 1])
            {
                let:int Temp = Arr[j];
                Arr[j] = Arr[j + 1];
                Arr[j + 1] = Temp;
            }
        }
    }
}

fun:int Main()
{
    let:int* Arr = [6, 2, 7, 5, 4, 9, 8];
    BubbleSort(Arr, 7);

    for (let:int i = 0; i < 7; i++)
        Out(Arr[i]);

    return 0;
}
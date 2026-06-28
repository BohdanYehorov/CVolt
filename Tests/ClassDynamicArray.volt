class IntArray
{
    let:i32* Data;
    let:i32 Size;

    fun:void Construct()
    {
        this.Data = null;
        this.Size = 0;
    }

    fun:void Add(i32 Num)
    {
        let:i32* NewData = i32*(MemAlloc((this.Size + 1) * 4));
        if (this.Data)
        {
            MemCpy(NewData, this.Data, this.Size * 4);
            MemFree(this.Data);
        }

        NewData[this.Size] = Num;
        this.Data = NewData;
        this.Size++;
    }

    fun:void PushBack(i32 Value)
    {
        this.Add(Value);
    }

    fun:i32 GetEl(i32 Index)
    {
        return this.Data[Index];
    }

    fun:void SetEl(i32 Index, i32 Value)
    {
        this.Data[Index] = Value;
    }

    fun:void Free()
    {
        if (this.Data)
            MemFree(this.Data);
    }
}

fun:i32 Main()
{
    let:IntArray Arr = IntArray();

    for (let:i32 i = 0; i < 10; i++)
        Arr.PushBack(i);

    for (let:i32 i = 0; i < 10; i++)
        Out(Arr.GetEl(i));

    for (let:i32 i = 0; i < 10; i++)
        Arr.SetEl(i, i + 10);

    for (let:i32 i = 0; i < 10; i++)
        Out(Arr.GetEl(i));

    Arr.Free();

    return 0;
}
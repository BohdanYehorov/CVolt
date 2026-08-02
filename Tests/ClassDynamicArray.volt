class IntArray
{
    let:i32* Data;
    let:u64 Size;

    IntArray()
    {
        this.Data = null;
        this.Size = 0u64;
    }

    fun:void Add(i32 Num)
    {
        let:i32* NewData = i32*(MemAlloc((this.Size + 1u64) * 4u64));
        if (this.Data)
        {
            MemCpy(NewData, this.Data, this.Size * 4u64);
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

    fun:i32 GetEl(u64 Index)
    {
        return this.Data[Index];
    }

    fun:void SetEl(u64 Index, i32 Value)
    {
        this.Data[Index] = Value;
    }

    fun:void Free()
    {
        if (this.Data)
            MemFree(this.Data);
    }
};

fun:i32 Main()
{
    let:IntArray Arr();

    for (let:i32 i = 0; i < 10; i++)
        Arr.PushBack(i);

    for (let:u64 i = 0u64; i < 10u64; i++)
        OutLine(Arr.GetEl(i));

    for (let:u64 i = 0u64; i < 10u64; i++)
        Arr.SetEl(i, i32(i + 10u64));

    for (let:u64 i = 0u64; i < 10u64; i++)
        OutLine(Arr.GetEl(i));

    Arr.Free();

    return 0;
}
class Array
{
    let:i8* Data;
    let:i32 Size;
    let:i32 Cap;
    let:i32 ElementSize;

    fun:void Construct(i32 ElementSize)
    {
        this.Data = null;
        this.Size = 0;
        this.Cap = 0;
        this.ElementSize = ElementSize;
    }

    fun:i32 CalculateCap(i32 Cap)
    {
        if (Cap == 0)
            return 4;
        return Cap * 2;
    }

    fun:void Reserve(i32 Cap)
    {
        if (this.Cap >= Cap)
            return;

        let:i8* NewData = i8*(MemAlloc(Cap * this.ElementSize));

        if (this.Size > 0)
        {
            MemCpy(NewData, this.Data, this.Cap * this.ElementSize);
            MemFree(this.Data);
        }

        this.Data = NewData;
        this.Cap = Cap;
    }

    fun:void Resize(i32 Size)
    {
        if (this.Size < Size)
            this.Reserve(this.CalculateCap(Size));

        this.Size = Size;
    }

    fun:void Add(void* El)
    {
        this.Resize(this.Size + 1);
        MemCpy(this.Data + (this.Size - 1) * this.ElementSize, El, this.ElementSize);
    }

    fun:void* GetEl(i32 Index)
    {
        return this.Data + Index * this.ElementSize;
    }

    fun:void Free()
    {
        if (this.Data)
            MemFree(this.Data);
    }
}

fun:i32 Main()
{
    let:Array Arr = Array(4);

    for (let:i32 i = 0; i < 10; i++)
        Arr.Add($i);

    *i32*(Arr.GetEl(5)) = 20;

    for (let:i32 i = 0; i < 10; i++)
        Out(*i32*(Arr.GetEl(i)));

    Arr.Free();
    return 0;
}
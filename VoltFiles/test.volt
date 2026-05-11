fun:int Main()
{
    let:int num = 5;
    let:int$ ref = num;
    let:int$ ref1 = ref;
    ref1 = 10;
    Out(num);
    return 0;
}
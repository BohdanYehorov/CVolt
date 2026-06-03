fun:int Main()
{
    Out(2 == 2);
    Out(2 != 3);
    Out(5 < 10);
    Out(5 > 2);
    Out(5 <= 10);
    Out(5 <= 5);
    Out(5 >= 2);
    Out(5 >= 5);

    Out(2 == 3);
    Out(2 != 2);
    Out(5 > 10);
    Out(5 < 2);
    Out(5 >= 10);
    Out(5 <= 2);

    Out(2.f == 2.f);
    Out(2.f != 3.f);
    Out(5.f < 10.f);
    Out(5.f > 2.f);
    Out(5.f <= 10.f);
    Out(5.f <= 5.f);
    Out(5.f >= 2.f);
    Out(5.f >= 5.f);

    Out(2.f == 3.f);
    Out(2.f != 2.f);
    Out(5.f > 10.f);
    Out(5.f < 2.f);
    Out(5.f >= 10.f);
    Out(5.f <= 2.f);

	return 0;
}
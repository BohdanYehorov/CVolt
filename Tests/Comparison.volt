fun:i32 Main()
{
    OutLine(2 == 2);
    OutLine(2 != 3);
    OutLine(5 < 10);
    OutLine(5 > 2);
    OutLine(5 <= 10);
    OutLine(5 <= 5);
    OutLine(5 >= 2);
    OutLine(5 >= 5);

    OutLine(2 == 3);
    OutLine(2 != 2);
    OutLine(5 > 10);
    OutLine(5 < 2);
    OutLine(5 >= 10);
    OutLine(5 <= 2);

    OutLine(2.0 == 2.0);
    OutLine(2.0 != 3.0);
    OutLine(5.0 < 10.0);
    OutLine(5.0 > 2.0);
    OutLine(5.0 <= 10.0);
    OutLine(5.0 <= 5.0);
    OutLine(5.0 >= 2.0);
    OutLine(5.0 >= 5.0);

    OutLine(2.0 == 3.0);
    OutLine(2.0 != 2.0);
    OutLine(5.0 > 10.0);
    OutLine(5.0 < 2.0);
    OutLine(5.0 >= 10.0);
    OutLine(5.0 <= 2.0);

	return 0;
}
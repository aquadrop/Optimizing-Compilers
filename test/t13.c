

void use(int x)
{
	printf("%d", x);
}


int live_out_test1()
{
	int a=10, b=11, c=12, d=4, e=5, f=6;

	if(a >= 1)
	{
		use(b);
	}

	if(a >= 2)
	{
		use(c);
	}

	if(a >= 3)
	{
		use(d);
	}

	if(a >= 4)
	{
		use(e);
	}

	if(a >= 5)

	{
		use(f);
	}

	return f;
}


int live_out_test2()
{
	int a=10, b=11, c=12, d=4, e=5, f=6;


	if(a >= 1)
	{
		use(b);
		if(a >= 2)
		{
			use(c);
			if(a >= 3)
			{
				use(d);
				if(a >= 4)
				{
					use(e);
					if(a >= 5)
					{
					use(f);
					}

				}
			}
		}
	}
	return f;
}

int live_out_test3()
{
	int a=10, b=11, c=12, d=4, e=5, f=6;


	if(a >= 1)
	{
		use(b);
		b = 0;
		if(a >= 2)
		{
			use(c);
			c = 0;
			if(a >= 3)
			{
				use(d);
				d = 0;
				if(a >= 4)
				{
					use(e);
					e = 0;
					if(a >= 5)
					{
					use(f);
					f=0;
					}

				}
			}
		}
		a = 0;
	}
	return f;
}

int live_out_test4()
{
	int a=10, b=11, c=12, d=4, e=5, f=6;


	L1:
	if(a >= 1)
	{
		L2:
		use(b);
		b = 0;
		if(a >= 2)
		{
			L3:
			use(c);
			c = 0;
			if(a >= 3)
			{
				L4:
				use(d);
				d = 0;
				if(a >= 4)
				{
					L5:
					use(e);
					e = 0;
					if(a >= 5)
					{
						L6:
						use(f);
						f=0;
						if(a > 7) {a--; goto L5; }

					}
					if(a > 6) {a--; goto L4; }
				}
				if(a > 5) {a--; goto L3; }
			}
			if(a > 4) {a--; goto L2; }
		}
		if(a > 3) {a--; goto L1; }

		a = 0;
	}
	return f;
}



int
main(int argc, char *argv[])
{

	live_out_test1();
	printf("\n");
	live_out_test2();
	printf("\n");
	live_out_test3();
	printf("\n");
	live_out_test4();
	printf("\n");

	exit(0);
	return 0;
}
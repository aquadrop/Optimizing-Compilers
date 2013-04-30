

int ae_test()
{
	int x = 10;
	int a = 1, b = 2, c = 3, d = 4;


	d = a-b;
	d = b-c;
	d = c-a;

	switch(x) {
	case 0:
		d = a-b;
		d = b-c;
		d = c-a;
		break;
	case 1:
		d = a-b;
		d = b-c;
		d = c-a;
		break;
	default:
		d = a-b;
		d = b-c;
		d = c-a;
		break;
	}

	if(x > 0)
	{
		x--;
		d = c-a;
		d = b-c;
		d = a-b;
	}

	if(x > 0)
	{
		x--;
		a=1, b=2, c=3;
		d = a-b;
		d = b-c;
		d = c-a;
	}

	if(x > 0)
	{
		x--;
		d = a-b;
		d = b-c;
		d = c-a;
		a=1, b=2, c=3;
	}

	switch(x) {
	case 0:
		d = a-b;
		d = b-c;
		d = c-a;
		break;
	case 1:
		d = a-b;
		d = c-a;
		a = 0;
		break;
	default:
		d = a-b;
		d = b-c;
		d = c-a;
		break;
	}

	d = a-b;
	d = b-c;
	d = c-a;

	return d;

}



int
main(int argc, char *argv[])
{

	ae_test();

	exit(0);
	return 0;
}
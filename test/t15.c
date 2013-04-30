#include <stdlib.h>
#include <stdio.h>

int ae_test (int a)
{
        int b, c, d, e, f;

        c = a + b;
	switch(a)
	{
		case 1:		printf("*\n");
                                c = a + b;
		case 2:		printf("**\n");
                                d = a + b;
		case 3:		printf("***\n");
                                b = a + b;
		case 4:		printf("****\n");
                                e = a + b;
		default:	printf("*****\n");
                                f = a + b;
	}

	switch(a)
	{
		case 1:		printf("*\n");		b = a + b; break;
		case 2:		printf("**\n");		f = a + b; break;
		case 3:		printf("***\n");	b = a + b; break;
		case 4:		printf("****\n");	e = a + b; break;
		default:	printf("*****\n");	c = a + b; break;
	}

	return 0;
}


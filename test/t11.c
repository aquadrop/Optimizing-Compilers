#include <stdlib.h>
#include <stdio.h>

int ae_test (int a)
{

	int b, c, d, e, f;

        b = ~a;
        if (a == 1) return 0;
        c = ~a;
        if (a == 2) return 0;
        d = ~b;
        if (a == 3) e = b + a;;

        if (a == 4) return 0;
        f = a + b;

	return 0;
}


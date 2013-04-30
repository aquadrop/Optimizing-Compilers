
void
backwards_loop_test()
{

  int i;

  for(i = 0; i < 2; i++)
  {

	goto ML16;
	ML0:  printf("L0 "); goto END;
	ML1:  printf("L1 ");  goto ML0;
	ML2:  printf("L2 ");  goto ML1;
	ML3:  printf("L3 ");  goto ML2;
	ML4:  printf("L4 ");  goto ML3;
	ML5:  printf("L5 ");  goto ML4;
	ML6:  printf("L6 ");  goto ML5;
	ML7:  printf("L7 ");  goto ML6;
	ML8:  printf("L8 ");  goto ML7;
	ML9:  printf("L9 ");  goto ML8;
	ML10: printf("L10 "); goto ML9;
	ML11: printf("L11 "); goto ML10;
	ML12: printf("L12 "); goto ML11;
	ML13: printf("L13 "); goto ML12;
	ML14: printf("L14 "); goto ML13;
	ML15: printf("L15 "); goto ML14;
	ML16: printf("L16 "); goto ML15;
	END: printf("END\n");

  }

  return;
}

int
main(int argc, char *argv[]) {

	backwards_loop_test();

	exit(0);
	return 0;
}

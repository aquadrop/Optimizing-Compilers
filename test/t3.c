/* FIXME ECL this testcase causes SUIF to die
 * due to a glic memory corruption
 */

void infinity()
{
  int a,b,c,d,e,i;

  printf ("please wait for a long time...");
  e=1;
  while (e) {
    b = 5;
    a = b * 3;
    c = a;
    d = a + c - b / 2;
  }
  printf ("The world has ended.\n");

/*FIXME moved here to avoid glibc bug*/

  c = -3;
  for (i=0;i<c;i+=3) {
    a = 6;
    b = a / 3;
    c = c + b;
    d = 17;
  }
  printf ("come back real soon now\n");
}

/* FIXME the error happens before the next function is parsed */
/****************************************************
void never()
{
  int a,b,c,d,i;

  c = -3;
  for (i=0;i<c;i+=3) {
    a = 6;
    b = a / 3;
    c = c + b;
    d = 17;
  }
  printf ("come back real soon now\n");
}
****************************************************/


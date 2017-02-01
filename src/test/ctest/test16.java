/*
 * Test 16
 *
 * Testiamo le istruzioni di pre-incremento, pre-decremento, post-incremento
 * e post-decremento.
 */

class HelloWorld 
{
  static public void main(String[] arg)
  {
    int i;
    long j;

    i=0;
    j=0;

    while (i<100) i++;

    System.out.println(i);

    while (i<200) ++i;

    System.out.println(i);

    while (i>100) i--;

    System.out.println(i);

    while (i>0) --i;

    System.out.println(i);

    while (j<100) j++;

    System.out.println(j);

    while (j<200) ++j;

    System.out.println(j);

    while (j>100) j--;

    System.out.println(j);

    while (j>0) --j;

    System.out.println(j);

  }
}

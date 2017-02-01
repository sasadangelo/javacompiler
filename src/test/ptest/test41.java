/* Test 41:
 *
 * Il test controlla l' accesso ad array. 
 */

package gimnasium;

class s2 {
	static int k[];
	float[] j;
}

class s1 {

  void pippo() 
   {
	int[] a[];
	int[] b;
	int c;
	s2 t;

	if (a[10][20]>0);
	if (a[15][12]>b[10]);
	if (c[10]>12);        // errore!!! c non e' un array.
	if (s2.k[15]>3);      // l'accesso e' OK, peche' k e' static.

	if (t.j[1]>9);
   }
}








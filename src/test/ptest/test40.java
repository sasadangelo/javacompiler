/* Test 40:
 *
 * Il test controlla l' accesso a dati locali e campi di classi, siano essi
 * statici che non. 
 */

package gimnasium;

class s3 {
  int k;
  protected static float j;
  private static float z;

  void pippo() { }
}

class s2 extends s4 {
  int c;
  static float d;
  s3 p;

  void pluto()
    {
      if (super.x>10); 
    }
}

class s1 {
  int x;	
  void pippo() 
   {
	int a,b;
	boolean p;
	s2 t;
	int dodo[];
	s3 sss;
	
	if (!(a>b)); 
	if (this.x>1);
	if (s2.c>2);      // errore!!! rif. statico a un campo non statico.
 	if (s2.d>2);
	if (t.c>2);
	if (t.p.k>10);
	if (s3.j>11);     /* 
	                   * j e' accessibile perche' s1 e' definita nello
	                   * stesso pacchetto di s3.
	                   */

	if (s3.z>12);     // errore!!! campo inaccessibile.

	if (dodo.length==1);
	
	if (sss.lk > 10); // errore!!! campo inesistente. 
	
	if (sss.pippo > 12);
   }
}

class s4 {
  int x;
}








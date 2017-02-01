/* Test 56:
 *
 * Istruzione synchronized.
 */

package gimnasium;

class s1 { }

class s3 {
  void pippo() 
  {
    int [][] a;
    s1 p;
    
    synchronized (a)
      {
	return;
      }
    
    synchronized (p)
      {
	return; 
      }
    
  } 
}


















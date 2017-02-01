/*
 * test 83:
 *
 * Analogo a test 81, solo che controlla la correttezza di gestione per
 * ForStmtNoShortIf.
 */

class pippo
{
}

public class corinto
{
  void gigi() {}

  void proc()
  {	
	int m;
	pippo c;

	if (m>1)
	  for (int i; i<=10; i++);
	else
	  {}
	
	if (m>1)
	  for (float k=2; true ; i--, i=4)
	  {
		int j;
	
		j=9;
	  }
	else {}

	if (m>1)
	  for (c=new pippo(), gigi(); m<=10; m++, --m);
	else {}	
  }	
}






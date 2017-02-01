/*
 * test 81:
 *
 * Controlleremo ora l'uso di StmtExprList, cioe' un insieme di StmtExpr 
 * che si utilizzano solo all'interno di ForInit e ForUpdate.
 * Completeremo il controllo del ciclo for... il cui test e' stato molto
 * approssimato in test63.java proprio per mancanza di StmtExprList.
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

	for (int i; i<=10; i++);
	
	for (float k=2; true ; k--, k=4)
	{
		int j;
	
		j=9;
	}

	for (c=new pippo(), gigi(); m<=10; m++, --m); 
  }	
}






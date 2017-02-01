/*
 * file cphash.h
 *
 * descrizione: questo file definisce una tabella hash, chiamata cphash 
 *              (Constant Pool Hashtable), utilizzata durante la generazione 
 *              del codice JVM per evitare duplicazioni di item della
 *              Constant Pool.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it
 *                         Salvatore D'Angelo e-mail xc0261@xcom.it
 */

/*
 * Definizione del nodo della tabella Constant Pool Hashtable.
 */

class CPHNode : public HTNode
{
private:
  int index;
  int tag;  

public:
  CPHNode();
  CPHNode(String&,int);
  ~CPHNode();
  int  getindex();
  void setindex(int);
  int  gettag();
  void PrintNode(FILE *);
};

/*
 * Definizione della tabella Constant Pool Hashtable.
 */

class CPHash : public Hashtable
{
public:
  CPHash(int);
  ~CPHash();

  CPHNode *Install_Id(String&,int);
  CPHNode *LookUp(String&,int);
};












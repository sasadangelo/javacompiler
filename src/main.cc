/*
 * file: main.cc 
 *
 * descrizione: questo file contiene il main del compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * April 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it
 */

#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <java.tab.h>
#include <jvm.h>
#include <compile.h>
#include <errors.h>
using namespace std;

FILE *fTrace;		
FILE *fSTout;
FILE *fTree;
FILE *fParseTree;
FILE *fParseTreeLoaded;

CompileUnit *Unit;

String DebugDirectory;

extern String init_name; 
extern String clinit_name;

extern String ObjectName;
extern String ErrorName;
extern String RuntimeExceptionName;
extern String ThrowableName;
extern String CloneableName;

extern String ArrayLengthName;
extern Descriptor ArrayLengthDescriptor;

extern Descriptor DesNull;
extern Descriptor DesByte;
extern Descriptor DesChar;
extern Descriptor DesDouble;
extern Descriptor DesFloat;
extern Descriptor DesInt;
extern Descriptor DesLong;
extern Descriptor DesShort;
extern Descriptor DesBoolean;
extern Descriptor DesVoid;

extern Descriptor DesString;
extern Descriptor DesThrowable;
extern Descriptor DesStringBuffer;

extern STNode *ArrayLengthNode;

// extern int yydebug;

/*****************************************************************************
 * Main Program.                                                             *
 *****************************************************************************/

int main(int argc, char * argv[])
{
  // yydebug=1;

#ifdef linux
  setenv("CLASSPATH","/home/saldan/javaproj/lib",0);   
#endif

  // argc=3; argv[1]="-c"; argv[2]="HelloWorld";

  // argc=2; argv[1]="./test/ctest/test48.java";

  // argc=2; argv[1]="./test/applet/TumblingDuke/TumbleItem.java";

  DebugDirectory="/home/saldan/javaproj/src/debug";

  init_name="<init>";
  clinit_name="<clinit>";

  ObjectName="java/lang/Object";
  ErrorName="java/lang/Error";
  RuntimeExceptionName="java/lang/RuntimeException";
  ThrowableName="java/lang/Throwable";
  CloneableName="java/lang/Cloneable";

  ArrayLengthName="length";
  ArrayLengthDescriptor=DES_INT;
  ArrayLengthNode=new STNode(ArrayLengthName,0,0,ArrayLengthDescriptor);

  DesNull    = DES_NULL;
  DesByte    = DES_BYTE;
  DesChar    = DES_CHAR;
  DesDouble  = DES_DOUBLE;
  DesFloat   = DES_FLOAT;
  DesInt     = DES_INT;
  DesLong    = DES_LONG;
  DesShort   = DES_SHORT;
  DesBoolean = DES_BOOLEAN;
  DesVoid    = DES_VOID;

  DesString       = DES_STRING; 
  DesThrowable    = DES_THROWABLE;
  DesStringBuffer = DES_STRINGBUFFER;

  switch(argc)
    {
    case 2:
      if (!strcmp(argv[1],"-h"))
	{
	  fprintf(stderr,"Java Compiler - Decompiler.\n\n");
	  fprintf(stderr,"SINTASSI : %s -[c|p|h] [.class|.java]\n",argv[0]);
	  fprintf(stderr,"%s -h         - Print this help.\n",argv[0]);
	  fprintf(stderr,"%s .java      - Gencode the class.\n",argv[0]);
	  fprintf(stderr,"%s -c class   - Disassemble the class.\n",argv[0]);
	  fprintf(stderr,"%s -p class   - Load/Save creat Tree class.\n",
		  argv[0]);
	  fprintf(stderr,"\n By Marco - Salvatore - Michele MDR.\n\n");
	}
      else
	{
	  int len;
	  if (((len=strlen(argv[1])) >= 6) &&
	      ( (argv[1][len-1]=='a') &&
		(argv[1][len-2]=='v') &&
		(argv[1][len-3]=='a') &&
		(argv[1][len-4]=='j') &&
		(argv[1][len-5]=='.')))
	    {
	      
	      Unit=new CompileUnit(argv[1]);
	      fTrace=fopen("./debug/Trace","w");
	      fSTout=fopen("./debug/STable","w");
	      fTree=fopen("./debug/Tree","w");
	      fParseTree=fopen("./debug/ParseTree","w");
	      Unit->Parse();
	      fclose(fTrace);
	      fclose(fSTout);
	      fclose(fTree);
	      fclose(fParseTree);
	      //delete Unit;
	    }
	  else
	    fprintf(stderr,"Input must be a valid .java and no %s.\n",
		    argv[1]);
	}
      break;
    case 3:
      {
	String Name;

	Unit=new CompileUnit(argv[2]);
	if (!strcmp(argv[1],"-c"))
	  {
	    Name=argv[2];
	    Unit->GetClass()->ReadClass(Name,0);
	    if (Unit->GetErrors()->GetNumErrors() > 0)
	      Unit->GetErrors()->Print();
	    else
	      Unit->GetClass()->Info(cout);
	  }
	else
	  if (!strcmp(argv[1],"-p"))
	    {
	      Name=argv[2];
	      Unit->GetClass()->LoadClass(Name,0);
	      if (Unit->GetErrors()->GetNumErrors() > 0)
		Unit->GetErrors()->Print();
	      else
		Unit->GetClass()->Info(cout);
	    }
	//delete Unit;
	break;
      }
    default:
      fprintf(stderr,"SINTASSI : %s -[c|p|h] [.class|.java]\n",argv[0]);
    }
  
  /*
   * NON GESTITO!!!
   *
   * delete Unit presenta qualche errore sporadico. Per continuare il nostro
   * lavoro, e' stata momentaneamente abolita.
   */
    return 0;
}

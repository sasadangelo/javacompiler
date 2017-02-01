/*
 * file pcode.cc
 *
 * descrizione: questo file non e' molto interessante per la comprensione dei
 *              meccanismi interni del compilatore.
 *              Esso e' esclusivamente utilizzato per una stampa di debug
 *              del bytecode di una classe Java.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Luglio 1997, scritto da Michele Risi, e-mail micris@zoo.diaedu.unisa.it.
 */

#include <../dyn_table.cc>
#include <stdlib.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <jvm.h>
#include <code.h>

unsigned int printcode(_u2 x,_u1 lab,DYNTable<_u1> *ff,ostream &debug,
		       Constant_Pool *cp)
{
  unsigned int yy=0;

  debug << "   " << x << jvm_name[(int)lab];

  switch((int)lab)
    {
    case 169:
    case 58:
    case 57:
    case 56:
    case 55:
    case 54:
    case 25:
    case 24:
    case 23:
    case 22:
    case 21:
    case 18:
    case 16:
      {
	debug << " #" << (int)ff->DYNget(x+1);
	yy=1;
	break;
      }
    case 188:
      {
	debug << " type:";
	switch((int)ff->DYNget(x+1))
	  {
	  case 4:{debug << "< boolean >";break;}
	  case 5:{debug << "< char >";break;}
	  case 6:{debug << "< float >";break;}
	  case 7:{debug << "< double >";break;}
	  case 8:{debug << "< byte >";break;}
	  case 9:{debug << "< short >";break;}
	  case 10:{debug << "< int >";break;}
	  case 11:{debug << "< long >";break;}
	  }
	yy=1;
	break;
      }
    case 184:
    case 183:
    case 182:
      {
	_u2 t1;
	int val=(((int)ff->DYNget(x+1) << 8)|((int)ff->DYNget(x+2)));
	debug << " #"<< val;	
	t1=((Constant_Methodref_info *)cp->Get(val))->GetNameAndTypeIndex();
	t1=((Constant_Name_And_Type *)cp->Get(t1))->GetNameIndex();
	debug << " < " << ((Constant_Utf8 *)cp->Get(t1))->GetBytes() << " >";
	yy=2;
	break;
      }
    case 193:
    case 192:
    case 189:
    case 181:
    case 179:
    case 168:
    case 20:
    case 19:
    case 17:
      {
	debug << " #"<< (((int)ff->DYNget(x+1)<<8)|((int)ff->DYNget(x+2)));
	yy=2;
	break;
      }
    case 199:
    case 198:
    case 167:
    case 166:
    case 165:
    case 164:
    case 163:
    case 162:
    case 161:
    case 160:
    case 159:
    case 158:
    case 157:
    case 156:
    case 155:
    case 154:
    case 153:
      {
	debug << " #"<< x+(((int)ff->DYNget(x+1)<<8)|((int)ff->DYNget(x+2)));
	yy=2;
	break;
      }
    case 132:
      {
	debug << " #" << (int)ff->DYNget(x+1)<< "   +" <<(int)ff->DYNget(x+2);
	yy=2;
	break;
      }
    case 170:
      {
	int init=x;
	int mod=3-(x%4);
	x+=mod;

	int defval=
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));
	 
	int lowb=
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));
	int highb=
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));
	debug << lowb << " to " << highb << endl;
	yy=mod+12+(highb-lowb+1)*4;
	int val;
	for (int i=lowb;i<=highb;i++)
	  {
	    val= 
	      ((int)(ff->DYNget(++x)) << 24) |
	      ((int)(ff->DYNget(++x)) << 16) |
	      ((int)(ff->DYNget(++x)) << 8) |
	      ((int)(ff->DYNget(++x)));

	    debug << "       " << i << " : " << init+val << endl;
	  }
	debug << "     default : " << x+defval;
	break;
      }
    case 171:
      {
	int init=x;
	int mod=3-(x%4);
	x+=mod;
	int defval=
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));
	int npair=
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));
	debug <<" : " << npair << endl;
	yy=mod+(npair+1)*8;
	int a1,b1;
	for(int i=0;i< npair;i++)
	  {
	    a1=
	      ((int)(ff->DYNget(++x)) << 24) |
	      ((int)(ff->DYNget(++x)) << 16) |
	      ((int)(ff->DYNget(++x)) << 8) |
	      ((int)(ff->DYNget(++x)));
	    b1=
	      ((int)(ff->DYNget(++x)) << 24) |
	      ((int)(ff->DYNget(++x)) << 16) |
	      ((int)(ff->DYNget(++x)) << 8) |
	      ((int)(ff->DYNget(++x)));
	    debug << "        " << a1 << " : " << init+b1 << endl;
	  }
	debug << "     default : " << x+defval;
	break;
      }
    case 187:
      {
	_u2 t1;
	_u2 val = (((int)ff->DYNget(x+1) << 8 )|((int)ff->DYNget(x+2)));
	debug << " #"<< val;
	t1=((Constant_Class *)cp->Get(val))->GetNameIndex();
	debug << " < " << ((Constant_Utf8 *)cp->Get(t1))->GetBytes() << " >";
	yy=2;
	break;
      }
    case 178:
    case 180:
      {
	_u2 t1;
	_u2 val = ((int)(ff->DYNget(++x))<< 8)|((int)(ff->DYNget(++x)));
	debug << " #" << val;
       	t1=((Constant_Fieldref_info *)cp->Get(val))->GetNameAndTypeIndex();
	t1=((Constant_Name_And_Type *)cp->Get(t1))->GetNameIndex();
	debug << " < " << ((Constant_Utf8 *)cp->Get(t1))->GetBytes() << " >";
	yy=2;	
	break;
      }
    case 185:
      {
	_u2 t1;
	_u2 val = ((int)(ff->DYNget(++x))<< 8)|((int)(ff->DYNget(++x)));
	debug << " #" << val;
       	debug << " arg: " << (int)(ff->DYNget(++x));
	t1=((Constant_IntMethodref_info *)cp->Get(val))->GetNameAndTypeIndex();
	t1=((Constant_Name_And_Type *)cp->Get(t1))->GetNameIndex();
	debug << " < " << ((Constant_Utf8 *)cp->Get(t1))->GetBytes() << " >";
	yy=4;	
	break;
      }
    case 196:
      {
	int low;
	if ((low=(int)ff->DYNget(x+1)) == 132)
	  {
	    debug << " < IINC >";
	    _u2 val=((int)(ff->DYNget(++x))<<8)|((int)(ff->DYNget(++x)));
	    debug << " #" << val;
	    val = ((int)(ff->DYNget(++x))<<8)|((int)(ff->DYNget(++x)));
	    debug << " #" << val;
	    yy=5;
	  }
	else
	  {
	    debug << " <" << jvm_name[(int)low] << ">";
	    _u2 val=((int)(ff->DYNget(++x))<<8)|((int)(ff->DYNget(++x)));
	    debug << " #" << val;
	    yy=3;	
	  }
	break;
      }
    case 197:
      {
	_u2 val =((int)(ff->DYNget(++x))<<8)|((int)(ff->DYNget(++x)));
	debug << " #" << val;
	debug <<" dim: " << ((int)(ff->DYNget(++x)));
	yy=3;		
	break;
      }
    case 200:
      {
	_u4 seg=x;
	_u4 val= 
	  ((int)(ff->DYNget(++x)) << 24) |
	  ((int)(ff->DYNget(++x)) << 16) |
	  ((int)(ff->DYNget(++x)) << 8) |
	  ((int)(ff->DYNget(++x)));;
	debug << " #" << seg+val;
	yy=4;
	break;
      }
    }
  debug << endl;
  return yy;
}


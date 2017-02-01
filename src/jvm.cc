/*
 * file jvm.cc
 *
 * descrizione: questo file contiene l'implementazione della Java Virtual
 *              Machine.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it.
 */

/*
 * Questo file contiene pochi commenti di proposito, dato che il codice e'
 * semplice da capire. Per ulteriori chiarimenti l'utente puo' fare riferimento
 * a qualsiasi testo in cui e' riportato il formato bytecode di JVM. Uno di 
 * questi testi e' "The Java Virtual Machine Specification" , Tim Lindholm e 
 * Frank Yellin, Addison Wesley.
 */

#include <netinet/in.h>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <globals.h>
#include <access.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <cphash.h>
#include <table.h>
#include <tree.h>
#include <compile.h>
#include <environment.h>
#include <jvm.h>
#include <errors.h>

#include <../dyn_table.cc>

extern CompileUnit *Unit;
extern TreeNode *Dummy;
extern int Nest_lev;
extern int Index;
extern int LastIndex;
extern String DebugDirectory;
extern char *msg_errors[];
extern Descriptor DesDouble;
extern Descriptor DesString;
extern Descriptor DesLong;
extern unsigned int printcode(_u2,_u1,DYNTable<_u1>*,ostream &,Constant_Pool*);

/* 
 * Funzioni globali usate da Java Virtual Machine.
 */

int numint(_u4 num) { return (int)num; }

float numfloat(_u4 num)
{
  if ( num == 0x7f800000 )
    {
      cout << " + Infinity\n";
      return 0;
    }
  if ( num == 0x7ff80000 )
    {  
      cout << " - Infinity\n";
      return 0;
    }
  if ( ((num >= 0x7f800001) && (num <= 0x7fffffff)) ||
      ((num >= 0xff800001) && (num <= 0xffffffff)))
    {
      cout << " NaN  Not a number\n";
      return 0;
    }
  int s = (( num >> 31) ==0) ? 1 : -1;
  int e = (( num >> 23) & 0xff);
  int m = (e==0) ? ( num & 0x7fffff) << 1 :( num & 0x7fffff) | 0x800000 ;
  return  (float)s*m*pow(2,e-150);
}

long long numlong(_u4 low,_u4 high)
{
  return (long long)(((long long)high << 32)+low);
}

double numdouble(_u4 low,_u4 high)
{
  long long bits = ((long long)high << 32)+low;

  if ( bits == 0x7f80000000000000LL )
    {
      cout << " + Infinity\n";
      return 0;
    }  
  if ( bits == 0xff80000000000000LL )
    {
      cout << " - Infinity\n";
      return 0;
    }
  if (((bits >= 0x7ff0000000000001LL) && (bits <= 0x7fffffffffffffffLL)) ||
      ((bits >= 0xfff0000000000001LL) && (bits <= 0xffffffffffffffffLL)))
    {
      cout << " NaN - Not a number \n";
      return 0;
    }
  
  int s = ((bits >> 63) == 0) ? 1 : -1;
  int e = (int)((bits >> 52) & 0x7ffL);
  long long m = (e == 0) ? (bits & 0xfffffffffffffLL) << 1 :
    (bits & 0xfffffffffffffLL) | 0x10000000000000LL;
  
  return (double)s*m*pow(2,e-1075);  
}

/*
 * Nel file .class i numeri sono in formato little endian e per uniformarci
 * allo standard big endian del C++ e' necessario convertire tali valori.
 */

_u4 Revu4(char *buf)
{
  _u4 num;
  ((char *)&num)[3]  = buf[0];
  ((char *)&num)[2]  = buf[1];
  ((char *)&num)[1]  = buf[2];
  ((char *)&num)[0]  = buf[3];
  return num;
}

_u2 Revu2(char *buf)
{
  _u2 num;
  ((char *)&num)[1]  = buf[0];
  ((char *)&num)[0]  = buf[1];
  return num;
}

/*
 * Operatori di lettura dei numeri da stream che chiamano le funzioni di
 * Reverse sopra implementate.
 */

_u4 Readu4(istream &fi)
{
  _u4 tt;
  char *buf4 = new char[4];
  fi.read(buf4,4);
  tt= Revu4(buf4);
  return tt;
}

_u2 Readu2(istream& fi)
{
  _u2 tt;
  char *buf2 = new char[2];  
  fi.read(buf2,2);
  tt= Revu2(buf2);
  return tt;
}

_u1 Readu1(istream& fi)
{
  _u1 tt;
  fi.read(&tt,1);
  return tt;
}

/*
 * Scrittura valori numerici su stream .class.
 */

ostream &Shotu4(ostream &fi,_u4 num)
{
  char *buf = new char[4];
  buf[0]=b3(num) ;
  buf[1]=b2(num) ;
  buf[2]=b1(num) ;
  buf[3]=b0(num) ;
  fi.write(buf,4);
  delete buf;
  return fi;
}

ostream &Shotu2(ostream &fi,_u2 num)
{
  char *buf = new char[2];
  buf[0]=b1(num) ;
  buf[1]=b0(num) ;  
  fi.write(buf,2);
  delete buf;
  return fi;
}

ostream &Shotu1(ostream &fi,_u1 num)
{
  char *buf = new char[1];
  buf[0]=b0(num) ;
  fi.write(buf,1);
  delete buf;
  return fi;
}

/*
 * Scrittura valori numerici su stream .class. Funzioni prelevate da guavac
 * versione 1.0.
 */

void WriteJavaU4(ostream &os, _u4 num)
{
  unsigned char *pBuffer=(unsigned char *)&num;
  os << *pBuffer++;
  os << *pBuffer++;
  os << *pBuffer++;
  os << *pBuffer;
}

/*****************************************************************************
 * classe Entry_Pool                                                         *
 *****************************************************************************/

/*
 * costruttore /distruttore Entry_Pool. 
 */

Entry_Pool::Entry_Pool()         { }
Entry_Pool::Entry_Pool(_u1 ptag) { tag=ptag; }
Entry_Pool::~Entry_Pool()        { }

/*
 * Entry_Pool::GetTag
 * Entry_Pool::SetTag
 * 
 * get/set the Entry Pool identificator.
 */

_u1  Entry_Pool::GetTag()          { return tag; }
void  Entry_Pool::SetTag(_u1 ntag) { tag=ntag; }

/*
 * Entry_Pool::Write
 * Entry_Pool::Info
 *
 * Print stampa la classe su file binario chiamando l'operatore <<.
 * Il motivo per cui non si e' utilizzato direttamente << e' che con
 * esso non e' ammesso l'invocazione virtuale.
 * Info, invece, e' un metodo che esegue una stampa di debug. 
 * Questi due metodi sono presenti in tutte le classi della JVM, per
 * cui eviteremo in seguito di commentarli.
 */

void Entry_Pool::Write(ostream& os) {}
void Entry_Pool::Info(ostream& os)  {}

/*
 * operator <<
 *
 * Stampa della classe su stream binario. Questi e' un metodo privato 
 * invocato dalla funzione Write.
 */

ostream& operator << (ostream& os,Entry_Pool& ep)
{
  Shotu1(os,ep.tag);
  return os;
}

/****************************************************************************
 * classe Constant_Class.                                                   *
 ****************************************************************************/

Constant_Class::Constant_Class(_u2 index):Entry_Pool(CONSTANT_Class)
{ 
  name_index=index;
}

Constant_Class::~Constant_Class()        { }
_u2 Constant_Class::GetNameIndex()       { return name_index; }
void Constant_Class::Write(ostream &os)  { os << *this;       }
ostream& operator << (ostream& os,Constant_Class& cc)
{
  os << *(Entry_Pool *)&cc;
  Shotu2(os,cc.name_index);
  return os;
}

void Constant_Class::Info(ostream& os)
{
  os << "Constant_Class" << endl;
  os << " name_index : " << name_index << endl;
}

/*****************************************************************************
 * classe Constant_Fieldref_info.                                            *
 *****************************************************************************/

Constant_Fieldref_info::Constant_Fieldref_info( _u2 name,_u2 cl_ind)
:Entry_Pool(CONSTANT_Fieldref)
{
  class_index=cl_ind;
  name_and_type_index=name;
}

Constant_Fieldref_info::~Constant_Fieldref_info() {}
_u2 Constant_Fieldref_info::GetClassIndex()      { return class_index;        }
_u2 Constant_Fieldref_info::GetNameAndTypeIndex(){ return name_and_type_index;}
void Constant_Fieldref_info::SetClassIndex(_u2 index)   { class_index=index;  }
void Constant_Fieldref_info::SetNameAndTypeIndex(_u2 index) 
  { name_and_type_index=index; }

void Constant_Fieldref_info::Write(ostream &os)  {os << *this;                }

ostream& operator << (ostream& os,Constant_Fieldref_info& cfi)
{
  os << *(Entry_Pool *)&cfi;
  Shotu2(os,cfi.class_index);     
  Shotu2(os,cfi.name_and_type_index);
  return os;
}

void Constant_Fieldref_info::Info(ostream& os)
{
  os << "Constant_Fieldref_Info" << endl;
  os << "class_index          : " << class_index << endl;     
  os << "name_and_type_index  : " << name_and_type_index << endl;
}

/*****************************************************************************
 * classe Constant_Methodref_info.                                           *
 *****************************************************************************/

Constant_Methodref_info::Constant_Methodref_info( _u2 name,_u2 cl_ind)
:Entry_Pool(CONSTANT_Methodref)
{
  class_index=cl_ind;
  name_and_type_index=name;
}

Constant_Methodref_info::~Constant_Methodref_info()          {}

_u2 Constant_Methodref_info::GetClassIndex()       { return class_index;     }
_u2 Constant_Methodref_info::GetNameAndTypeIndex() 
  { return name_and_type_index; }
void Constant_Methodref_info::SetClassIndex(_u2 index)  { class_index=index; }
void Constant_Methodref_info::SetNameAndTypeIndex(_u2 index) 
  { name_and_type_index=index; }

void Constant_Methodref_info::Write(ostream& os)   { os << *this;            }

ostream& operator << (ostream& os,Constant_Methodref_info& cmi)
{
  os << *(Entry_Pool *)&cmi;
  Shotu2(os,cmi.class_index);    
  Shotu2(os,cmi.name_and_type_index);
  return os;
}

void Constant_Methodref_info::Info(ostream& os)
{
  os << "Constant_Methodref_Info" << endl;
  os << "class_index          : " << class_index << endl;     
  os << "name_and_type_index  : " << name_and_type_index << endl;
}

/****************************************************************************
 * classe Constant_IntMethodref_info.                                       *
 ****************************************************************************/

Constant_IntMethodref_info::Constant_IntMethodref_info( _u2 name,_u2 cl_ind)
:Entry_Pool(CONSTANT_IntMethodref)
{
  class_index=cl_ind;
  name_and_type_index=name;
}

Constant_IntMethodref_info::~Constant_IntMethodref_info()       {}

_u2 Constant_IntMethodref_info::GetClassIndex()     { return class_index; }
_u2 Constant_IntMethodref_info::GetNameAndTypeIndex()
  { return name_and_type_index; }
void Constant_IntMethodref_info::SetClassIndex(_u2 index)       
  { class_index=index; }
void Constant_IntMethodref_info::SetNameAndTypeIndex(_u2 index) 
  { name_and_type_index=index; }

void Constant_IntMethodref_info::Write(ostream& os) { os << *this;        }

ostream& operator << (ostream& os,Constant_IntMethodref_info& cii)
{
  os << *(Entry_Pool *)&cii;
  Shotu2(os,cii.class_index);
  Shotu2(os,cii.name_and_type_index);
  return os;
}

void Constant_IntMethodref_info::Info(ostream& os)
{
  os << "Constant_IntMethodref_Info" << endl;
  os << "class_index          : " << class_index << endl;     
  os << "name_and_type_index  : " << name_and_type_index << endl;
}

/****************************************************************************
 * classe Constant_String.                                                  *
 ****************************************************************************/

Constant_String::Constant_String(_u2 str) : Entry_Pool(CONSTANT_String)
{ 
  string_index=str; 
}

Constant_String::~Constant_String() {}

_u2 Constant_String::GetStringIndex()           { return string_index; }
void Constant_String::SetStringIndex(_u2 index) { string_index=index;  }

void Constant_String::Write(ostream& os)        { os << *this;         }

ostream& operator << (ostream& os,Constant_String& cs)
{
  os << *(Entry_Pool *)&cs;
  Shotu2(os,cs.string_index);
  return os;
}

void Constant_String::Info(ostream& os)
{
  os << "Constant_String" << endl;
  os << "string_index : " << string_index << endl;
}

/*****************************************************************************
 * classe Constant_Name_And_Type.                                            *
 *****************************************************************************/

Constant_Name_And_Type::Constant_Name_And_Type(_u2 des,_u2 name) 
:Entry_Pool(CONSTANT_NameAndType)
{
  name_index=name;
  descriptor_index=des;
}

Constant_Name_And_Type::~Constant_Name_And_Type() {}

_u2 Constant_Name_And_Type::GetNameIndex()        { return name_index;       }
_u2 Constant_Name_And_Type::GetDescriptorIndex()  { return descriptor_index; }
void Constant_Name_And_Type::SetNameIndex(_u2 index)  { name_index=index;    }
void Constant_Name_And_Type::SetDescriptorIndex(_u2 index) 
  { descriptor_index=index; }

void Constant_Name_And_Type::Write(ostream& os)   { os << *this;             }

ostream& operator << (ostream& os,Constant_Name_And_Type& cnt)
{
  os << *(Entry_Pool *)&cnt;
  Shotu2(os,cnt.name_index);
  Shotu2(os,cnt.descriptor_index);
  return os;
}

void Constant_Name_And_Type::Info(ostream& os)
{
  os << "Constant_Name_And_Type" << endl;
  os << "name_index       : " << name_index << endl;
  os << "descriptor_index : " << descriptor_index << endl;
}

/*****************************************************************************
 * classe Constant_Utf8.                                                     *
 *****************************************************************************/

Constant_Utf8::Constant_Utf8(char *str) : Entry_Pool(CONSTANT_Utf8)
{
  length=strlen(str);
  bytes = new _u1[length+1];
  strcpy((char *)bytes,str);
}

Constant_Utf8::~Constant_Utf8()        { delete bytes; }
_u1 *Constant_Utf8::GetBytes()         { return bytes; }

void Constant_Utf8::Write(ostream &os) { os << *this;  }

ostream& operator << (ostream& os,Constant_Utf8& utf)
{ 
  os << *(Entry_Pool *)&utf;
  Shotu2(os,utf.length);
  os << utf.bytes;
  return os;
}

void Constant_Utf8::Info(ostream& os)
{ 
  os << "Constant_Utf8" << endl;
  os << "length : " << length << endl;
  os << "bytes  : " << bytes << endl;
}

/*****************************************************************************
 * classe Constant_Integer.                                                  *
 *****************************************************************************/

Constant_Integer::Constant_Integer(_u4 val):Entry_Pool(CONSTANT_Integer) 
{
  bytes=val;
}

Constant_Integer::~Constant_Integer()        { }

_u4 Constant_Integer::GetBytes()             { return bytes; }
void Constant_Integer::SetBytes(_u4 info)    { bytes=info;   }

void Constant_Integer::Write(ostream& os)    { os << *this;  }

ostream& operator << (ostream& os,Constant_Integer& ci)
{
  os << *(Entry_Pool *)&ci;
  Shotu4(os,ci.bytes);
  return os;
}

void Constant_Integer::Info(ostream& os)
{
  os << "Constant_Integer" << endl;
  os << "bytes  : " << bytes << endl;
  os << "valore : " << numint(bytes) << endl;
}

/*****************************************************************************
 * classe Constant_Float.                                                    *
 *****************************************************************************/

Constant_Float::Constant_Float(_u4 val):Entry_Pool(CONSTANT_Float) 
{
  bytes=val;
}

Constant_Float::~Constant_Float()         { }

_u4 Constant_Float::GetBytes()            { return bytes; }
void Constant_Float::SetBytes(_u4 info)   { bytes=info;   }

void Constant_Float::Write(ostream& os)   { os << *this;  }

ostream& operator << (ostream& os,Constant_Float& cf)
{
  os << *(Entry_Pool *)&cf;
  WriteJavaU4(os,cf.bytes);
  return os;
}

void Constant_Float::Info(ostream& os)
{
  os << "Constant_Float" << endl;
  os << "bytes  : " << bytes << endl;
  os << "valore : " << numfloat(bytes) << endl;
}

/*****************************************************************************
 * classe Constant_Long.                                                     *
 *****************************************************************************/

Constant_Long::Constant_Long(_u4 low,_u4 high):Entry_Pool(CONSTANT_Long)
{
  high_bytes=high;
  low_bytes=low;
}

Constant_Long::~Constant_Long() {}

_u4 Constant_Long::GetHighBytes()          { return high_bytes; }
_u4 Constant_Long::GetLowBytes()           { return low_bytes;  }
void Constant_Long::SetHighBytes(_u4 info) { high_bytes=info;   }
void Constant_Long::SetLowBytes(_u4 info)  { low_bytes=info;    }

void Constant_Long::Write(ostream& os)     { os << *this;       }

ostream& operator << (ostream& os,Constant_Long& cl)
{
  os << *(Entry_Pool *)&cl;

  #ifdef linux

  Shotu4(os,cl.high_bytes);
  Shotu4(os,cl.low_bytes);

  #else

  Shotu4(os,cl.low_bytes);
  Shotu4(os,cl.high_bytes);

  #endif

  return os;
}

void Constant_Long::Info(ostream& os)
{
  os << "Constant_Long" << endl;
  os << "high_bytes : " << high_bytes << endl;
  os << "low_bytes  : " << low_bytes << endl;
  os << "valore     : " << numlong(low_bytes,high_bytes) << endl;
}

/*****************************************************************************
 * classe Constant_Double.                                                   *
 *****************************************************************************/

Constant_Double::Constant_Double(_u4 low,_u4 high):Entry_Pool(CONSTANT_Double)
{
  high_bytes=high;
  low_bytes=low;
}

Constant_Double::~Constant_Double() { }

_u4 Constant_Double::GetHighBytes()          { return high_bytes; }
_u4 Constant_Double::GetLowBytes()           { return low_bytes;  }
void Constant_Double::SetHighBytes(_u4 info) { high_bytes=info;   }
void Constant_Double::SetLowBytes(_u4 info)  { low_bytes=info;    }

void Constant_Double::Write(ostream& os)     { os << *this;       }

ostream& operator << (ostream& os,Constant_Double& cd)
{
  os << *(Entry_Pool *)&cd;

  #ifdef linux 

  WriteJavaU4(os,cd.high_bytes);
  WriteJavaU4(os,cd.low_bytes);

  #else

  WriteJavaU4(os,cd.low_bytes);
  WriteJavaU4(os,cd.high_bytes);

  #endif

  return os;
}

void Constant_Double::Info(ostream& os)
{
  os << "Constant_Double" << endl;
  os << "high_bytes : " << high_bytes << endl;
  os << "low_bytes  : " << low_bytes << endl;
  os << "valore     : " << numdouble(low_bytes,high_bytes) << endl;
}

/*****************************************************************************
 * classe Constant_Pool.                                                     *
 *****************************************************************************/

Constant_Pool::Constant_Pool()  { table = new DYNTable<Entry_Pool *>(1); }

Constant_Pool::~Constant_Pool() { delete table;                          }

_u2 Constant_Pool::AddInPool(Entry_Pool *entry)
{
  _u1 tag;

  table->DYNinsert(entry);
  if ((tag=entry->GetTag())==CONSTANT_Long || tag==CONSTANT_Double)
    {
      table->DYNinsert(entry);
      return table->DYNgetdim()-1;
    }
  return table->DYNgetdim();
}

Entry_Pool *Constant_Pool::Get(_u2 i) { return table->DYNget(i-1);       }

_u2 Constant_Pool::GetLen()     { return table->DYNgetdim()+1;           }

ostream& operator << (ostream& os,Constant_Pool& cp)
{
  _u1 lab;
  for (unsigned i=1 ; i <= cp.table->DYNgetdim() ; i++)
    {
      lab=cp.table->DYNget(i-1)->GetTag();
      cp.table->DYNget(i-1)->Write(os);
      if((lab==CONSTANT_Double) || (lab==CONSTANT_Long))
	i++;
    }
  return os;
}

void Constant_Pool::Info(ostream& os)
{
  _u1 lab;
  for (unsigned i=1 ; i <= table->DYNgetdim() ; i++)
    {
      os << "--------------------------------------------------\n";
      os << i << "  ";
      lab=table->DYNget(i-1)->GetTag();
      table->DYNget(i-1)->Info(os);
      if((lab==CONSTANT_Double) || (lab==CONSTANT_Long))
	i++;
    }
  os << "--------------------------------------------------\n";
}

/*****************************************************************************
 * classe Interfaces_Pool.                                                   *
 *****************************************************************************/

Interfaces_Pool::Interfaces_Pool()  { interfaces_table = new DYNTable<_u2>(); }
Interfaces_Pool::~Interfaces_Pool() { delete interfaces_table;                }

_u2 Interfaces_Pool::AddInterfaces(_u2 val)
{
  interfaces_table->DYNinsert(val);
  return interfaces_table->DYNgetdim()-1;
}

_u2 Interfaces_Pool::GetInterface(_u2 val) 
{
  return interfaces_table->DYNget(val);
}

ostream& operator << (ostream& os,Interfaces_Pool& ip)
{
  unsigned i;
  for (i=0;i<ip.interfaces_table->DYNgetdim();i++)
    {
      Shotu2(os,ip.interfaces_table->DYNget(i));
    }
  return os;
}

void Interfaces_Pool::Info(ostream& os,Constant_Pool *cp)
{
  os << "Number of Interfacess  #" << interfaces_table->DYNgetdim() << endl;
  os << endl;
  for (unsigned i=0;i<interfaces_table->DYNgetdim();i++)
    {
      _u2 vl=((Constant_Class *)
	      (cp->Get(interfaces_table->DYNget(i))))->GetNameIndex();
      os <<" Interface : ";
      os << ((Constant_Utf8 *)(cp->Get(vl)))->GetBytes() << endl;
    }
}

/*****************************************************************************
 * classe Entry_Attribute.                                                   *
 *****************************************************************************/

/*
 * Costruttore/distruttore attributo.
 */

Entry_Attribute::Entry_Attribute()             
{
  attribute_name_index=0;
  attribute_length=0;
}

Entry_Attribute::Entry_Attribute(_u2 ind, _u4 len)                       
{
  attribute_name_index=ind;
  attribute_length=len;
}

Entry_Attribute::~Entry_Attribute() { }

/*
 * Alcuni semplici metodi.
 */

void Entry_Attribute::SetAttributeNameIndex(_u2 ind) 
  { attribute_name_index=ind; }  
void Entry_Attribute::SetAttributeLength(_u4 attr) { attribute_length=attr;  }
_u2  Entry_Attribute::GetAttributeNameIndex() { return attribute_name_index; }
_u4  Entry_Attribute::GetAttributeLength()    { return attribute_length;     }
void Entry_Attribute::Write(ostream& os)      { os << *this;                 }
void Entry_Attribute::Info(ostream& os,Constant_Pool *cp) {}

ostream& operator << (ostream& os,Entry_Attribute& ea)
{
  Shotu2(os,ea.attribute_name_index); 
  Shotu4(os,ea.attribute_length);
  return os;
}
 
/*****************************************************************************
 * classe Source_File_Attribute.                                             *
 *****************************************************************************/

Source_File_Attribute::Source_File_Attribute():Entry_Attribute(0,2) {}

Source_File_Attribute::Source_File_Attribute(_u2 val):Entry_Attribute(0,2)
{
  source_file_index = val;
}

Source_File_Attribute::Source_File_Attribute(_u2 val,_u2 val1)
:Entry_Attribute(val1,2)
{
  source_file_index = val;
}

Source_File_Attribute::~Source_File_Attribute() {}

void Source_File_Attribute::SetSourceFileIndex(_u2 attr) 
  { source_file_index=attr; }
_u2  Source_File_Attribute::GetSourceFileIndex() { return source_file_index; }

void Source_File_Attribute::Write(ostream& os)   { os << *this;              }

void Source_File_Attribute::Info(ostream& os,Constant_Pool *cp)
{
  os << endl;
  os << "SourceFile" << endl;
  os << " Sorce File Name  : ";
  os << ((Constant_Utf8 *)(cp->Get(source_file_index)))->GetBytes() <<endl;
}

ostream& operator << (ostream& os,Source_File_Attribute& sfa)
{
  os << *(Entry_Attribute*)&sfa;
  Shotu2(os,sfa.source_file_index);
  return os;
}

/*****************************************************************************
 * classe Constant_Value_Attribute.                                          *
 *****************************************************************************/

Constant_Value_Attribute::Constant_Value_Attribute() :Entry_Attribute(0,2) {}

Constant_Value_Attribute::Constant_Value_Attribute(_u2 val) 
:Entry_Attribute(0,2) 
{
  constant_value_index= val;
}

Constant_Value_Attribute::Constant_Value_Attribute(_u2 val1 ,_u2 val2) 
:Entry_Attribute(val2,2) 
{
  constant_value_index= val1;
}

Constant_Value_Attribute::~Constant_Value_Attribute()         {}

void Constant_Value_Attribute::SetConstantValueIndex(_u2 val) 
  { constant_value_index=val;    }

_u2 Constant_Value_Attribute::GetConstantValueIndex()         
  { return constant_value_index; }

void Constant_Value_Attribute::Write(ostream& os) { os << *this; }

void Constant_Value_Attribute::Info(ostream& os,Constant_Pool *cp)
{
  os << endl;
  os << "ConstantValue" << endl;
  os << endl;
  cp->Get(constant_value_index)->Info(os);
  os << endl;
}

ostream& operator << (ostream& os,Constant_Value_Attribute& cva)
{
  os << *(Entry_Attribute*)&cva; 
  Shotu2(os,cva.constant_value_index);
  return os;
}

/*****************************************************************************
 * classe Exception_Parameter.                                               *
 *****************************************************************************/

Exception_Parameter::Exception_Parameter()       
{
  start_pc=0;
  end_pc=0;
  handler_pc=0;
  catch_type=0;
}

Exception_Parameter::~Exception_Parameter()      {}

_u2 Exception_Parameter::GetStartPc()            { return start_pc; }
_u2 Exception_Parameter::GetEndPc()              { return end_pc; }
_u2 Exception_Parameter::GetHandlerPc()          { return handler_pc; }
_u2 Exception_Parameter::GetCatchType()          { return catch_type; }
void Exception_Parameter::SetStartPc(_u2 pc)     { start_pc=pc; }
void Exception_Parameter::SetEndPc(_u2 pc)       { end_pc=pc; }
void Exception_Parameter::SetHandlerPc(_u2 pc)   { handler_pc=pc; }
void Exception_Parameter::SetCatchType(_u2 pc)   { catch_type=pc; }

ostream& operator << (ostream& os,Exception_Parameter& ep)
{
  Shotu2(os,ep.start_pc);
  Shotu2(os,ep.end_pc);
  Shotu2(os,ep.handler_pc);
  Shotu2(os,ep.catch_type);
  return os;
}

/*****************************************************************************
 * classe Code_Attribute.                                                    *
 *****************************************************************************/

Code_Attribute::Code_Attribute():Entry_Attribute()
{
  max_stack=0;
  max_locals=0;
  code_length=0;
  exception_table_length=0;
  code = new DYNTable<_u1>();
  exception_table = new DYNTable<Exception_Parameter *>();
  attributes_count=0;
}

Code_Attribute::~Code_Attribute()
{
  delete code;
  delete exception_table;
}

_u2 Code_Attribute::GetMaxStack()            { return max_stack;              }
_u2 Code_Attribute::GetMaxLocals()           { return max_locals;             }
_u4 Code_Attribute::GetCodeLength()          { return code_length;            }
_u2 Code_Attribute::GetExceptionTableLength(){ return exception_table_length; }
_u2 Code_Attribute::GetAttributesCount()     { return attributes_count;       }

Exception_Parameter *Code_Attribute::GetExceptionParameter(int i) 
{ 
  return exception_table->DYNget(i);
}

void Code_Attribute::SetExceptionParameter(Exception_Parameter *ep, int i) 
{ 
  exception_table->DYNset(i,ep);
}

_u4 Code_Attribute::SetCode(_u1 opcode)                   
{ 
  code->DYNinsert(opcode);
  return code->DYNgetdim()-1;
}

/*
 * Code_Attribute::Gen
 *
 * Metodi per la generazione di istruzioni JVM. Di questo metodo ne scriviamo
 * piu' versioni cosi' da adattarlo alle varie istruzioni JVM. Si ricordi che
 * questo metodo verra' utilizzato, durante la fase di generazione del codice
 * o nel caricamento di un file .class.
 */

/*
 * Utilizzato per generare: RETURN, xSTORE_<i>, xCONST_<i>, xRETURN, 
 * MONITORENTER, MONITOREXIT, ATHROW, LOOKUPSWITCH, IOR, LOR, IXOR, LXOR,
 * IAND, LAND, LCMPL, DCMPL, ISHL, ISHR, IUSHR, LSHL, LSHR, LUSHR, IADD, LADD
 * FADD, DADD, ISUB, LSUB, FSUB, DSUB, IMUL, FMUL, LMUL, DMUL, IDIV, IREM,
 * LDIV, LREM, FDIV, FREM, DDIV, DREM, INEG, LNEG, FNEG, DNEG, D2F, D2I, D2L,
 * F2D, F2I, F2L, I2B, I2C, I2D, I2F, I2L, I2S, L2D, L2F, L2I, ACONST_NULL,
 * AALOAD, BALOAD, CALOAD, DALOAD, FALOAD, IALOAD, LALOAD, SALOAD, AASTORE,
 * BASTORE, IASTORE, LASTORE, CASTORE, DASTORE, FASTORE, SASTORE.
 */

void Code_Attribute::Gen(_u1 opcode) 
{
  code->DYNinsert(opcode);
  Unit->pc_count++;
}

/*
 * Utilizzato per generare: xSTORE index, xLOAD index, NEWARRAY atype, 
 * BIPUSH, LDC index.
 */

void Code_Attribute::Gen(_u1 opcode, _u1 index)
{
  code->DYNinsert(opcode);
  code->DYNinsert(index);
  Unit->pc_count+=2;
}

/*
 * Utilizzato per generare: IF_<cond> label, GOTO label, IFNULL label,
 * IFNONNULL label, IF_ACMP<cond> label, IF_ICMP<cond> label,
 * INSTANCEOF index, GETSTATIC index, GETFIELD index, INVOKESPECIAL index,
 * INVOKESTATIC index, INVOKEVIRTUAL index, ANEWARRAY index, LDC_W index
 * LDC2_W, PUTSTATIC index, PUTFIELD index.
 */

void Code_Attribute::Gen(_u1 opcode, _u2 label)
{
  code->DYNinsert(opcode);
  code->DYNinsert((_u1)(label >> 8));
  code->DYNinsert((_u1)(label & 0x00FF));
  Unit->pc_count+=3;
}

/*
 * Utilizzato per generare: IINC.
 */

void Code_Attribute::Gen(_u1 opcode, _u1 index, char constant)
{
  code->DYNinsert(opcode);
  code->DYNinsert(index);
  code->DYNinsert(constant);
  Unit->pc_count+=3;
}

/*
 * Utilizzato per generare: INVOKEINTERFACE index nargs, MULTINEWARRAY index
 * dimension.
 */

void Code_Attribute::Gen(_u1 opcode, _u2 index, _u1 val)
{
  code->DYNinsert(opcode);
  code->DYNinsert((_u1)(index >> 8));
  code->DYNinsert((_u1)(index & 0x00FF));
  code->DYNinsert(val);
  Unit->pc_count+=4;
}

/*
 * Code_Attribute::Gen_to
 *
 * Genera un'istruzione JVM a partire da un certo indirizzo in poi.
 */

void Code_Attribute::Gen_to(_u4 label, _u1 opcode, _u2 index)
{
  _u2 offset=index-label;
  code->DYNset(label,opcode);
  code->DYNset(label+1,(_u1)(offset >> 8));
  code->DYNset(label+2,(_u1)(offset & 0x00FF));
}

void Code_Attribute::Gen_to(_u4 label, _u4 index)
{
  code->DYNset(label  ,(_u1)(index >> 24));
  code->DYNset(label+1,(_u1)(index >> 16) & 0x000000FF);
  code->DYNset(label+2,(_u1)(index >> 8) & 0x000000FF);
  code->DYNset(label+3,(_u1)(index & 0x000000FF));
}

_u1 Code_Attribute::GetCode(_u2 val)             { return code->DYNget(val);}
void Code_Attribute::SetMaxStack(_u2 max)        { max_stack=max;           }
void Code_Attribute::SetMaxLocals(_u2 max )      { max_locals=max;          }
void Code_Attribute::SetCodeLength(_u4 len)      { code_length=len;         }

void Code_Attribute::SetExceptionTableLength()  
{ 
  exception_table_length=exception_table->DYNgetdim(); 
}

void Code_Attribute::SetAttributesCount(_u2 count) { attributes_count=count; }

void Code_Attribute::AddException(Exception_Parameter * pp)
{
  exception_table->DYNinsert(pp);
}

void Code_Attribute::Write(ostream& os) { os << *this; }

void Code_Attribute::Info(ostream& os,Constant_Pool *cp)
{
  _u1 lab;
  os << endl;
  os << "Max Stack : "<< max_stack << endl;
  os << "Max Local : "<<  max_locals << endl;
  os << "Code" << endl;
  for (_u2 y=0 ; y < code_length ; y++)
    {
      y+=printcode(y,GetCode(y),code,os,cp);  
    }
  if (exception_table_length > 0)
    {
      os << "\nException Table:" << endl;
      for (int i=0; i<exception_table_length; i++)
	{
	  Exception_Parameter *ep=exception_table->DYNget(i);
	  os << ep->GetStartPc()   << "  " ;
	  os << ep->GetEndPc()     << "  " ;
	  os << ep->GetHandlerPc() << "  " ;
	  os << ep->GetCatchType() << endl ;
	}
    }
  os << endl;
}

ostream& operator << (ostream& os,Code_Attribute& ca)
{
  os << *(Entry_Attribute*)&ca;
  Shotu2(os,ca.max_stack);
  Shotu2(os,ca.max_locals);
  Shotu4(os,ca.code_length);
  if (ca.code_length > 0)
    os << *ca.code;
  Shotu2(os,ca.exception_table_length);
  if (ca.exception_table_length  > 0)
    for (int i=0; i< ca.exception_table_length; i++)
      os << *(ca.exception_table->DYNget(i));
  Shotu2(os,ca.attributes_count);  
  return os;
}

/*****************************************************************************
 * classe Exception_Attribute.                                               *
 *****************************************************************************/

Exceptions_Attribute::Exceptions_Attribute():Entry_Attribute(0,0)  
{
  number_of_exceptions=0;
  table = new DYNTable<_u2>();
}

Exceptions_Attribute::Exceptions_Attribute(_u2 num):Entry_Attribute(0,0) 
{
  table = new DYNTable<_u2>(num);
  number_of_exceptions= num;
} 

Exceptions_Attribute::~Exceptions_Attribute()
{
  delete table;
}

void Exceptions_Attribute::SetAttributeNameIndexLength(_u2 index,_u4 len)
{
  attribute_name_index=index;
  attribute_length=len;
};

void Exceptions_Attribute::SetNumberOfExceptions(_u2 exc)
{
  number_of_exceptions=exc;
}

_u2 Exceptions_Attribute::GetNumberOfExceptions()
{
  return number_of_exceptions;
}

_u2 Exceptions_Attribute::GetException(_u2 num)
{
  return table->DYNget(num);
}

_u2 Exceptions_Attribute::SetException(_u2 num)
{
  table->DYNinsert(num);
  return (table->DYNgetdim())-1;
}

void Exceptions_Attribute::Write(ostream& os)
{
  os << *this;
}

void Exceptions_Attribute::Info(ostream& os,Constant_Pool *cp)
{
  _u2 vl;
  os << endl;
  os << "Exceptions" << endl;
  os << "Number of Exceptions #" << number_of_exceptions <<endl;
  for (int y=0 ; y < number_of_exceptions ; y++)
    {
      os << y+1 << " Exception : ";  
      vl=((Constant_Class *)(cp->Get(GetException(y))))->GetNameIndex();
      os << ((Constant_Utf8 *)(cp->Get(vl)))->GetBytes() << endl;
    }
}

ostream& operator << (ostream& os,Exceptions_Attribute& ea)
{
  os << *(Entry_Attribute*)&ea;
  Shotu2(os,ea.number_of_exceptions);
  if (ea.number_of_exceptions > 0)
    {
      for(_u2 i=0; i < ea.number_of_exceptions; i++)
	{
	  Shotu2(os,ea.table->DYNget(i));
	}
    }  
  return os;
}

/*****************************************************************************
 * classe Method_Info.                                                       *
 *****************************************************************************/

Method_Info::Method_Info()    
{
  access_flags=0;
  name_index=0;
  descriptor_index=0;
  attributes_count=0;
}  

Method_Info::~Method_Info()                       
{ 
  delete attributes;
}

void Method_Info::SetArray(_u2 gra) {attributes = new Entry_Attribute*[gra]();}
_u2 Method_Info::GetAccessFlags()                 { return access_flags;      }
_u2 Method_Info::GetNameIndex()                   { return name_index;        }
_u2 Method_Info::GetDescriptorIndex()             { return descriptor_index;  }
_u2 Method_Info::GetAttributesCount()             { return attributes_count;  }
Entry_Attribute *Method_Info::GetAttribute(_u2 i) { return attributes[i];     }
void Method_Info::SetAccessFlags(_u2 flag)        { access_flags=flag;        }
void Method_Info::SetNameIndex(_u2 name)          { name_index=name;          }
void Method_Info::SetDescriptorIndex(_u2 des)     { descriptor_index=des;     }
void Method_Info::SetAttributesCount(_u2 count)   { attributes_count=count;   }
void Method_Info::SetAttribute(Entry_Attribute *ai,_u2 i) {attributes[i]=ai;  }

ostream& operator << (ostream& os,Method_Info& mi)
{
  Shotu2(os,mi.access_flags);
  Shotu2(os,mi.name_index);
  Shotu2(os,mi.descriptor_index);
  Shotu2(os,mi.attributes_count);
  if (mi.attributes_count > 0)
    {
      mi.attributes[0]->Write(os);
      if (mi.attributes_count == 2)
	mi.attributes[1]->Write(os);
    }  
  return os;
}

void Method_Info::Info(ostream &os,Constant_Pool *cp)
{
  os << " Method : ";
  os << ((Constant_Utf8 *)(cp->Get(name_index)))->GetBytes();
  os << "  -  ";
  os << ((Constant_Utf8 *)(cp->Get(descriptor_index)))->GetBytes() << endl;
  os << " Number of Attributes  # " << attributes_count << endl;
  if (attributes_count > 0)
    {
      attributes[0]->Info(os,cp);
      if (attributes_count == 2)
	attributes[1]->Info(os,cp);
    }
}

/*****************************************************************************
 * classe Method_Pool.                                                       *
 *****************************************************************************/

Method_Pool::Method_Pool()
{
  method_table = new DYNTable<Method_Info *>();
}

Method_Pool::~Method_Pool()
{
  delete method_table;
}

_u2 Method_Pool::AddMethod(Method_Info *val)
{
  method_table->DYNinsert(val);
  return (method_table->DYNgetdim())-1;
}

DYNTable<Method_Info *> *Method_Pool::GetTable() { return method_table;}

Method_Info *Method_Pool::GetMethod(_u2 val)
{
  return method_table->DYNget(val);
}

ostream& operator << (ostream& os,Method_Pool& ip)
{
  for (unsigned i=0;i<ip.method_table->DYNgetdim();i++)
    os << *ip.method_table->DYNget(i);
  return os;
}

void Method_Pool::Info(ostream& os,Constant_Pool *cp)
{
  os << "Number of Methods  #" << method_table->DYNgetdim() << endl;
  for (unsigned i=0; i<method_table->DYNgetdim(); i++)
    {
      os << i+1 ;
      method_table->DYNget(i)->Info(os,cp);
      os << endl;
    }
  os << endl;
}

/*****************************************************************************
 * classe Field_Pool.                                                        *
 *****************************************************************************/

Field_Pool::Field_Pool()  { field_table = new DYNTable<Field_Info *>(); }
Field_Pool::~Field_Pool() { delete field_table;                         }

_u2 Field_Pool::AddField(Field_Info *val)
{
  field_table->DYNinsert(val);
  return (field_table->DYNgetdim())-1;
}

DYNTable<Field_Info *> *Field_Pool::GetTable() { return field_table;}  

Field_Info *Field_Pool::GetField(_u2 val)
{
  return field_table->DYNget(val);
}

ostream& operator << (ostream& os,Field_Pool& ip)
{
  for (unsigned i=0;i<ip.field_table->DYNgetdim();i++)
    os << *ip.field_table->DYNget(i);
  return os;
}

void Field_Pool::Info(ostream& os,Constant_Pool *cp)
{
  os << "Number of Fields  #" << field_table->DYNgetdim() << endl;
  for (unsigned i=0;i<field_table->DYNgetdim();i++)
    {
      os << i+1;
      field_table->DYNget(i)->Info(os,cp);
      os << endl;
    }
  os << endl;
}

/*****************************************************************************
 * classe Field_Info.                                                        *
 *****************************************************************************/

Field_Info::Field_Info()            
{
  access_flags=0;
  name_index=0;
  descriptor_index=0;
  attributes_count=0;
}

Field_Info::~Field_Info()                        
{ 
  delete attributes;
}

void Field_Info::SetArray(_u2 gra) {attributes = new Entry_Attribute*[gra]();}
_u2 Field_Info::GetAccessFlags()                 { return access_flags;      }
_u2 Field_Info::GetNameIndex()                   { return name_index;        }
_u2 Field_Info::GetDescriptorIndex()             { return descriptor_index;  }
_u2 Field_Info::GetAttributesCount()             { return attributes_count;  }
Entry_Attribute *Field_Info::GetAttribute(_u2 i) { return attributes[i];     }
void Field_Info::SetAccessFlags(_u2 flag)        { access_flags=flag;        }
void Field_Info::SetNameIndex(_u2 name)          { name_index=name;          }
void Field_Info::SetDescriptorIndex(_u2 des)     { descriptor_index=des;     }
void Field_Info::SetAttributesCount(_u2 count)   { attributes_count=count;   }
void Field_Info::SetAttribute(Entry_Attribute *ai,_u2 i)  { attributes[i]=ai;}

ostream& operator << (ostream& os,Field_Info& fi)
{  
  Shotu2(os,fi.access_flags);
  Shotu2(os,fi.name_index);
  Shotu2(os,fi.descriptor_index);
  Shotu2(os,fi.attributes_count);
  if (fi.attributes_count > 0)
    fi.attributes[0]->Write(os);
  return os;
}

void Field_Info::Info(ostream &os,Constant_Pool *cp)
{
  os << " Field : ";
  os << ((Constant_Utf8 *)(cp->Get(name_index)))->GetBytes();
  os << "  -  ";
  os << ((Constant_Utf8 *)(cp->Get(descriptor_index)))->GetBytes() << endl;
  os << " Number of Attributes  # " << attributes_count << endl;
  if (attributes_count > 0)
      attributes[0]->Info(os,cp);
}

/*****************************************************************************
 * classe Class_File.                                                        *
 *****************************************************************************/

Class_File::Class_File()
{
  magic=0xCAFEBABE;
  minor_version=3;
  major_version=45;
  constant_pool_count=0;
  c_pool = new Constant_Pool();
  access_flags=0;
  this_class=0;
  super_class=0;
  interfaces_count=0;
  i_pool = new Interfaces_Pool();
  field_count=0;
  f_pool = new Field_Pool();
  method_count=0;
  m_pool = new Method_Pool();
  attributes_count=0;
  attr   = new Entry_Attribute*[1]();
}

Class_File::~Class_File()
{
  delete c_pool;
  delete i_pool;
  delete f_pool;
  delete m_pool;
  delete attr;
}

_u4  Class_File::Getmagic()               { return magic;                    }
_u2  Class_File::Getminor()               { return minor_version;            }
_u2  Class_File::Getmajor()               { return major_version;            }
_u2  Class_File::Getconstantpoolcount()   { return constant_pool_count;      }
Constant_Pool *Class_File::Getconstantpool()     { return c_pool;            }
_u2  Class_File::Getaccessflags()                { return access_flags;      }
_u2  Class_File::Getthisclass()                  { return this_class;        }
_u2  Class_File::Getsuperclass()                 { return super_class;       }
_u2  Class_File::Getinterfacescount()            { return interfaces_count;  }
Interfaces_Pool *Class_File::Getinterfaces()     { return i_pool;            }
_u2  Class_File::Getfieldcount()                 { return field_count;       }
Field_Pool *Class_File::Getfieldinfo()           { return f_pool;            }
_u2  Class_File::Getmethodcount()                { return method_count;      }
Method_Pool *Class_File::Getmethodinfo()         { return m_pool;            }
_u2  Class_File::Getattributescount()            { return attributes_count;  }
Entry_Attribute *Class_File::Getattributeinfo()  { return attr[0];           }
void Class_File::Setmagic(_u4 mag)               { magic=mag;                }
void Class_File::Setminor(_u2 minor)             { minor_version=minor;      }
void Class_File::Setmajor(_u2 major)             { major_version=major;      }
void Class_File::Setconstantpoolcount(_u2 count) { constant_pool_count=count;}
unsigned Class_File::Setconstantpool(Entry_Pool *ep) { c_pool->AddInPool(ep);}
void Class_File::Setaccessflags(_u2 flag)        { access_flags=flag;        }
void Class_File::Setthisclass(_u2 who)           { this_class=who;           }
void Class_File::Setsuperclass(_u2 who)          { super_class=who;          }
void Class_File::Setinterfacescount(_u2 count)   { interfaces_count=count;   }
_u2  Class_File::Setinterfaces(_u2 value) 
  { return i_pool->AddInterfaces(value); }
void Class_File::Setfieldcount(_u2 count)        { field_count=count;        }
_u2  Class_File::Setfieldinfo(Field_Info *fi)    
  { return f_pool->AddField(fi); }
void Class_File::Setmethodcount(_u2 count)       { method_count=count;       }
_u2  Class_File::Setmethodinfo(Method_Info *mi) 
  { return m_pool->AddMethod(mi); }
void Class_File::Setattributescount(_u2 count)   { attributes_count=count;   }
void Class_File::SetAttributes(Entry_Attribute *ai) { attr[0]=ai;            }

/*
 * Class_File::Info
 * 
 * stampa informazioni di debug del file .class.
 */

void Class_File::Info(ostream& os)
{
  os << endl;
  os <<"/********************************************************/" <<endl;
  os <<"/* File .class                                          */" <<endl;
  os <<"/********************************************************/" <<endl;
  os << endl <<" Number of element in the Constant Pool : ";
  os << Getconstantpoolcount() << endl;
  c_pool->Info(os);
  os << endl <<" This: " << Getthisclass() << endl;
  os << endl <<" Super: " << Getsuperclass() << endl << endl;
  i_pool->Info(os,c_pool);
  f_pool->Info(os,c_pool);
  m_pool->Info(os,c_pool);
  os << endl;
  os << " Attribute Table  #" << attributes_count << endl;
  if(attributes_count >0)
    attr[0]->Info(os,c_pool);
}

/*
 * Class_File::ReadClass
 *
 * Legge il bytecode di un file .class e carica in questa struttura tutte le
 * informazioni ivi contenute.
 * Se il file viene letto senza problemi, allora la routine restituira' TRUE,
 * altrimenti FALSE. Chi chiamera' ReadClass, nel momento in cui ricevera'
 * FALSE interrompera' l'intera compilazione.
 */

int Class_File::ReadClass(String& Name, int lib)
{
  int esito=FALSE;    // Restituisce l'esito del load.
    
  _u1 tag;
  _u2 cpool_size;    // taglia constant pool.
  _u2 ipool_size;    // taglia pool di interfacce impl. dalla classe.
  _u2 fpool_size;    // taglia pool di campi della classe.
  _u2 mpool_size;    // taglia pool di metodi della classe.

  _u2 a_count;      // num. attributi.
    
  _u2 a_name_index; // indice nome attributo.
  _u4 a_length;     // taglia attributo. 
    
  String NameClass;
  String ClassName;
  
  NameClass= Name+".class";

  if (lib)
    ClassName=ENVIRONMENT->GetClassPath()+"/"+NameClass;
  else
    ClassName=NameClass;

  ifstream cstream(ClassName.to_char(),ios::in);
  
  if (!cstream)
    return FALSE;
  
  if((Readu4(cstream)!= Getmagic() ) || (Readu2(cstream)!= Getminor()) || 
     (Readu2(cstream)!= Getmajor()))
    {
      Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],NameClass.to_char(),
		      "...");
      goto quit;
    }
  
  /*
   * Carico l'intera constant pool. Si ricordi che la prima entry della
   * constant pool (cpool[0]) e' riservata.
   */
  
  Setconstantpoolcount(cpool_size=Readu2(cstream));  

  for (int i=1; i< cpool_size; i++)
    {
      Entry_Pool *base;
      _u2 b1, b2;
      
      switch(tag=Readu1(cstream))
	{
	case CONSTANT_Class :
	  {
	    base = new Constant_Class(Readu2(cstream));
	    break;
	  }
	case CONSTANT_Fieldref :
	  {
#ifdef linux
	    base = new Constant_Fieldref_info(Readu2(cstream),Readu2(cstream));
#else
	    b1=Readu2(cstream); 
	    b2=Readu2(cstream);
	    base = new Constant_Fieldref_info(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_Methodref :
	  {
#ifdef linux
	    base=new Constant_Methodref_info(Readu2(cstream),Readu2(cstream));
#else
	    b1=Readu2(cstream);
	    b2=Readu2(cstream);
	    base = new Constant_Methodref_info(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_IntMethodref :
	  {
#ifdef linux
	  base=new Constant_IntMethodref_info(Readu2(cstream),Readu2(cstream));
#else
	    b1=Readu2(cstream);
	    b2=Readu2(cstream);
	    base = new Constant_IntMethodref_info(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_String :
	  {
	    base = new Constant_String(Readu2(cstream));
	    break;
	  }	
	case CONSTANT_Integer :
	  {
	    base = new Constant_Integer(Readu4(cstream));
	    break;
	  }	
	case CONSTANT_Float : 
	  {
	    base = new Constant_Float(Readu4(cstream));
	    break;
	  }	
	case CONSTANT_Long :
	  {
#ifdef linux 
	    base=new Constant_Long(Readu4(cstream),Readu4(cstream));
#else
	    b2=Readu4(cstream);
	    b1=Readu4(cstream);
	    base = new Constant_Long(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_Double :
	  {
#ifdef linux
	    base=new Constant_Double(Readu4(cstream),Readu4(cstream));
#else
	    b2=Readu4(cstream);
	    b1=Readu4(cstream);
	    base = new Constant_Double(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_NameAndType :
	  {
#ifdef linux
	    base=new Constant_Name_And_Type(Readu2(cstream),Readu2(cstream));
#else
	    b1=Readu2(cstream);
	    b2=Readu2(cstream);
	    base = new Constant_Name_And_Type(b2,b1);
#endif
	    break;
	  }	
	case CONSTANT_Utf8 :
	  {
	    int length;
	      
	    length=Readu2(cstream);
	    char *buff = new char[length+1];
	    cstream.read(buff,length);
	    buff[length]='\0';	   
	    base =  new Constant_Utf8(buff);
	    break;
	  }	
	default :
	  {
	    Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],NameClass.to_char(),
			    "...");
	    goto quit;
	  }
	}
      
      Setconstantpool(base);

      if (tag == CONSTANT_Double || tag == CONSTANT_Long)
	{
	  // Setconstantpool(new Entry_Pool(0));
      	  i++;
      	}
    }

  Setaccessflags(Readu2(cstream));
  Setthisclass(Readu2(cstream));   
  Setsuperclass(Readu2(cstream));  
  
  Setinterfacescount(ipool_size=Readu2(cstream));
  
  /*
   * Carico il pool delle interfacce controllando anche l'integrita' del 
   * bytecode.
   */
  
  for (int i=0 ; i < ipool_size ; i++)
    {
      _u2 addrs_intf;
      
      Setinterfaces(addrs_intf=Readu2(cstream));
      if ((Getconstantpool()->Get(addrs_intf)->GetTag())==CONSTANT_Class)
	{
	  int addrs_name;
	  
	  addrs_name=((Constant_Class *)
		      Getconstantpool()->Get(addrs_intf))->GetNameIndex();
	  if ((Getconstantpool()->Get(addrs_name)->GetTag())!=CONSTANT_Utf8)
	    {
	      Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],
			      NameClass.to_char(),"CONSTANT_Utf8","...");
	      goto quit;
	    }
	}
      else
	{
	  Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],NameClass.to_char(),
			  "CONSTANT_Class","...");
	  goto quit;
	}
    }
  
  Entry_Attribute *attload;
  Setfieldcount(fpool_size=Readu2(cstream));    
  
  for (int i=0 ; i < fpool_size ; i++)
    {
      _u2 addrs_name, attr_size;
      
      Field_Info *field = new Field_Info(); 
      field->SetAccessFlags(Readu2(cstream));
      field->SetNameIndex(addrs_name=Readu2(cstream));
      if ((Getconstantpool()->Get(addrs_name)->GetTag())!=CONSTANT_Utf8)
	{
	  Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],NameClass.to_char(),
			  "Constant_Utf8","...");
	  goto quit;
	}
      
      field->SetDescriptorIndex(Readu2(cstream));
      attr_size=Readu2(cstream);
      field->SetAttributesCount(attr_size);
      field->SetArray(attr_size);
      
      for(int j=0 ; j < attr_size ; j++)
	{
	  a_name_index=Readu2(cstream);
	  if ((a_length=Readu4(cstream))!=2)
	    {
	      Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],
			      NameClass.to_char(),"...");
	      goto quit;
	    }
	  
	  if (Getconstantpool()->Get(a_name_index)->GetTag()==CONSTANT_Utf8)
	    {
	      Constant_Utf8 *utf8=(Constant_Utf8 *)
		(Getconstantpool()->Get(a_name_index));
	      String name;

	      name=(char *) utf8->GetBytes();
	      
	      /*
	       * L'unico attributo ammesso per un campo, e' ConstantValue.
	       */
	      
	      if (name=="ConstantValue")
		{
		  _u2 k;
		  
		  attload = new Constant_Value_Attribute(k=Readu2(cstream));
		  attload->SetAttributeNameIndex(a_name_index);
		  attload->SetAttributeLength(a_length);
		  field->SetAttribute(attload,j);
		  
		  tag=Getconstantpool()->Get(k)->GetTag();
		  
		  switch(tag) 
		    {
		    case CONSTANT_Long:
		      {
			Constant_Long *cl =(Constant_Long *)
			  Getconstantpool()->Get(k);
			break;
		      }
		    case CONSTANT_Float:
		      {
			Constant_Float *cl =(Constant_Float *)
			  Getconstantpool()->Get(k);
			break;
		      }
		    case CONSTANT_Double:
		      {
			Constant_Double *cl =(Constant_Double *)
			  Getconstantpool()->Get(k);
			break;
		      }	    
		    case CONSTANT_Integer:
		      {
			Constant_Integer *cl =(Constant_Integer *)
			  Getconstantpool()->Get(k);
			break;
		      }
		    case CONSTANT_String:
		      {
			Constant_String *cl =(Constant_String *)
			  Getconstantpool()->Get(k);
			break;
		      }	    
		    default:
		      {
			Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],
					NameClass.to_char(),"...");
			goto quit;
		      }
		    }
		}
	      else
		{
		  Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],
				  NameClass.to_char(),"...");
		  goto quit;
		}
	    }
	  else
	    {
	      Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],
			      NameClass.to_char(),"CONSTANT_Utf8","...");
	      goto quit;
	    }
	}
      Setfieldinfo(field); 
    }
  
  Setmethodcount(mpool_size=Readu2(cstream));
  
  for (int i=0 ; i < mpool_size ; i++)
    {
      _u2 k;
      
      Method_Info *mi = new Method_Info();      
      mi->SetAccessFlags(Readu2(cstream));
      mi->SetNameIndex(k=Readu2(cstream));
      
      if ((Getconstantpool()->Get(k)->GetTag())!=CONSTANT_Utf8)
	{
	  Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],NameClass.to_char(),
			  "CONSTANT_Utf8","...");
	  goto quit;
	}
      
      mi->SetDescriptorIndex(k=Readu2(cstream));
      
      if ((Getconstantpool()->Get(k)->GetTag())!=CONSTANT_Utf8)
	{
	  Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],NameClass.to_char(),
			  "CONSTANT_Utf8","...");
	  goto quit;
	}
      
      k=Readu2(cstream);
      mi->SetAttributesCount(k);
      mi->SetArray(k);
      
      _u2 attr_count=k;
      
      /*
       * Il numero degli attributi di un metodo, vanno da 0 a 2. Gli attribu-
       * ti ammessi, sono Code e Exceptions. Il primo conterra' il codice 
       * del metodo, mentre il secondo la lista delle eccezioni.
       */
      
      for(int j=0 ; j < attr_count ; j++)
	{
	  _u2 exceptions_length;
	  _u2 num_exceptions;
	  
	  a_name_index=Readu2(cstream);
	  a_length=Readu4(cstream);
	  
	  if ((Getconstantpool()->Get(a_name_index)->GetTag())==
	      CONSTANT_Utf8)
	    {
	      Constant_Utf8 *utf8=(Constant_Utf8 *)
		(Getconstantpool()->Get(a_name_index));
	      
	      String name;
	      
	      name=(char *)utf8->GetBytes();

	      if (name=="Code")
		{
		  int code_length;
		  
		  Code_Attribute *code = new Code_Attribute();
		  code->SetAttributeNameIndex(a_name_index);
		  code->SetAttributeLength(a_length);
		  code->SetMaxStack(Readu2(cstream));
		  code->SetMaxLocals(Readu2(cstream));
		  code_length=Readu4(cstream);
		  code->SetCodeLength(code_length);
		  
		  /*
		   * Carico il bytecode del metodo.
		   */
		  
		  for (int z=0 ; z < code_length ; z++)
		    code->SetCode(Readu1(cstream));
		  
		  exceptions_length=Readu2(cstream);
		  
		  if (exceptions_length > 0)
		    {

		      /*
		       * Carico la tabella delle eccezioni.
		       */
		      
		      for (int x=0 ;x < exceptions_length; x++)
			{
			  Exception_Parameter *ep=new Exception_Parameter();
			  
			  ep->SetStartPc(Readu2(cstream));
			  ep->SetEndPc(Readu2(cstream));
			  ep->SetHandlerPc(Readu2(cstream));
			  ep->SetCatchType(Readu2(cstream));
			  
			  code->AddException(ep);
			}
		    }

		  code->SetExceptionTableLength();

		  a_count=Readu2(cstream);
		  code->SetAttributesCount(0);
		  for(int x=0 ; x < a_count ; x++)
		    {
		      Readu2(cstream);
		      _u4 val=Readu4(cstream);
		      cstream.seekg(val,ios::cur);
		    }
		  mi->SetAttribute(code,j);	
		}
	      else
		if (name=="Exceptions")
		  {
		    num_exceptions=Readu2(cstream);
		    Exceptions_Attribute *exceptions=
		      new Exceptions_Attribute(num_exceptions);
		    exceptions->SetAttributeNameIndexLength(a_name_index,
							    a_length);
		    
		    /*
		     * Legge la tabella delle eccezioni, controllando che
		     * ogni indirizzamento alla constant pool faccia 
		     * riferimento a un'entry CONSTANT_Class.
		     */
		    
		    for(int yy=0; yy < num_exceptions ; yy++)
		      {
			int index;
			
			exceptions->SetException(index=Readu2(cstream));
			if ((Getconstantpool()->Get(index)->GetTag())!= 
			    CONSTANT_Class)
			  {
			    Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],
					    NameClass.to_char(),
					    "CONSTANT_Class","...");
			    goto quit;
			  }
		      }
		    mi->SetAttribute(exceptions,j);
		  }
		else
		  {
		    Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],
				    NameClass.to_char(),"...");
		    goto quit;
		  }
	    }
	  else
	    {
	      Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],
			      NameClass.to_char(),"CONSTANT_Utf8","...");
	      goto quit;
	    }
	}
      Setmethodinfo(mi);
    }
  
  /*
   * Ora viene letto l'attributo del file .class, che e' unico ed e' di
   * tipo SourceFile. Esso contiene il nome del file .java sorgente da
   * cui e' stato derivato il bytecode corrente.
   */
  
  a_count=Readu2(cstream);
  Setattributescount(a_count);
  
  a_name_index=Readu2(cstream);
  a_length=Readu4(cstream);
  
  if ((Getconstantpool()->Get(a_name_index)->GetTag())==CONSTANT_Utf8)
    {
      Constant_Utf8 *utf8=
	(Constant_Utf8 *)(Getconstantpool()->Get(a_name_index));
      String name;

      name=(char *)utf8->GetBytes();
      
      if (name=="SourceFile")
	{
	  _u2 source_index;
	  
	  attload = new Source_File_Attribute(source_index=Readu2(cstream));
	  
	  if ((Getconstantpool()->Get(source_index)->GetTag())!=
	      CONSTANT_Utf8)
	    {
	      Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],
			      NameClass.to_char(),"CONSTANT_Utf8","...");
	      goto quit;
	    }
	  
	  attload->SetAttributeNameIndex(a_name_index);
	  attload->SetAttributeLength(a_length);
	}
      else
	{
	  Unit->MsgErrors(0,msg_errors[ERR_CLASS_FORMAT],NameClass.to_char(),
			  "...");
	  goto quit;
	}
      
      SetAttributes(attload);
    }
  else
    {
      Unit->MsgErrors(0,msg_errors[ERR_WRONG_CLASS],NameClass.to_char(),
		      "CONSTANT_Utf8","...");
      goto quit;
    } 
  
  esito=TRUE;

quit:

  cstream.close();
  return esito;
}

/*
 * Class_File::LoadClass
 *
 * Carica una classe nella struttura dati corrente che fungera' da buffer
 * temporaneo. Questo metodo e' direttamente invocato da 
 * CompileUnit::LoadClass.
 */

STNode *Class_File::LoadClass(String& Name, int Index)
{ 
  _u1    tag;

  String NameCode;
  String NameTree;
  String FullName;
  
  STNode *ClassNode;

  int    is_interface=FALSE;
  
  TreeNode *tfield  = Dummy;
  TreeNode *tmethod = Dummy;
  TreeNode *tbody   = Dummy;
  TreeNode *ttrow   = Dummy;
  TreeNode *tintf   = Dummy;
  TreeNode *tsuper  = Dummy;
  TreeNode *tparams = Dummy;

  TreeNode *tree    = Dummy;
  
  NameCode=DebugDirectory+"/"+Name.to_name()+".code";
  NameTree=DebugDirectory+"/"+Name.to_name()+".tree";

  if ((ClassNode=STABLE->LookUp(Name,SECOND_LVL))!=NULL)
    {
      if (ClassNode->getlevel()==FIRST_LVL)
	{
	  /*
	   * Classe da caricare.
	   */

	  FullName=ClassNode->getfullname(); 
	  
	  if (!ReadClass(FullName,1))
	    {
	      --LastIndex;
	      return NULL;
	    }
  
	  Descriptor descriptor;
	  
	  descriptor.build_link(FullName);
	  STABLE->Discard(ClassNode);
	  ClassNode=STABLE->Install_Id(FullName.to_name(),SECOND_LVL,Index,
				       descriptor);
	  ClassNode->setfullname(FullName);
	}
      else
	{
	  --LastIndex;
	  return ClassNode;
	}
    }
  else
    {
      /*
       * Noi lavoriamo con l'ipotesi che quando si invoca loadclass, lo si
       * invoca sempre con un qualified name.
       */
      
      if (!ReadClass(Name,1))
	{
	  --LastIndex;
	  return NULL;
	}
  
      Descriptor descriptor;
      
      descriptor.build_link(Name);
      STABLE->Discard(ClassNode);
      ClassNode=STABLE->Install_Id(Name.to_name(),SECOND_LVL,Index,
				   descriptor);
      ClassNode->setfullname(Name);        
    }

  ofstream fCode(NameCode.to_char(),ios::out);
  
  if(!fCode)
    Unit->MsgErrors(0,msg_errors[ERR_CANT_WRITE],NameCode.to_char());
  
  Info(fCode);

  fCode.close();
 
  ClassNode->setaccess(Getaccessflags());

  if ( Getaccessflags() & ACC_INTERFACE)
    is_interface=TRUE;

  /* 
   * Da qui inizia la trasformazione del file .class in parse-tree.
   * Iniziamo con i campi della classe.
   */

  Field_Pool *fields=Getfieldinfo();
  Field_Info *field; 
  _u4 field_count=Getfieldcount();

  for (_u4 i=0; i < field_count; i++)
    {
      _u2 name_index;
      TreeNode *CVNode=Dummy;

      field=fields->GetField(i);
      name_index=field->GetNameIndex();
      Constant_Utf8 *utf8=
	(Constant_Utf8 *)(Getconstantpool()->Get(name_index));
      String name;

      name=(char *)utf8->GetBytes();

      utf8=(Constant_Utf8 *)
	(Getconstantpool()->Get(field->GetDescriptorIndex()));

      Descriptor descriptor;

      descriptor=(char *)(utf8->GetBytes());
      
      STNode *fnode=STABLE->Install_Id(name,THIRD_LVL,Index,descriptor);
      fnode->setaccess(field->GetAccessFlags());
      fnode->setmyclass(ClassNode);

      for(_u4 j=0 ; j < field->GetAttributesCount() ; j++)
	{
       	  Constant_Value_Attribute *attribute=
	    (Constant_Value_Attribute* )(field->GetAttribute(j));
	 
	  _u2 op=attribute->GetConstantValueIndex();
	  _u1 tag=Getconstantpool()->Get(op)->GetTag();

	  switch(tag) 
	    {
	    case CONSTANT_Long:
	      {
		Constant_Long *cl=(Constant_Long*)Getconstantpool()->Get(op);
		CVNode=new INUMNode(numlong(cl->GetHighBytes(),
					    cl->GetLowBytes()));
		CVNode->SetDescriptor(DesLong);
		break;
	      }
	    case CONSTANT_Float:
	      {
		Constant_Float *cl=(Constant_Float*)Getconstantpool()->Get(op);
		CVNode=new FNUMNode(numfloat(cl->GetBytes()));
		break;
	      }
	    case CONSTANT_Double:
	      {
		Constant_Double *cl=
		  (Constant_Double*)Getconstantpool()->Get(op);
		CVNode=new FNUMNode(numdouble(cl->GetHighBytes(),
					      cl->GetLowBytes()));
		CVNode->SetDescriptor(DesDouble);
		break;
	      }	    
	    case CONSTANT_Integer:
	      {
		Constant_Integer *cl=
		  (Constant_Integer*)Getconstantpool()->Get(op);
		CVNode=new INUMNode(numint(cl->GetBytes()));
		break;
	      }
	    case CONSTANT_String:
	      {
		Constant_String *cl=
		  (Constant_String*)Getconstantpool()->Get(op);
		CVNode=new STRINGNode((char *)(((Constant_Utf8 *)
		   Getconstantpool()->Get(cl->GetStringIndex()))->GetBytes()));
		CVNode->SetDescriptor(DesString);
		break;
	      }	    
	    }
	}

      tfield=new EXPNode(FieldDeclOp,tfield,new EXPNode(CommaOp,new 
							IDNode(fnode),CVNode));
    } 

  if(Getfieldcount() > 0 && !tfield->IsDummy())
    if(!is_interface)
      tbody = new EXPNode(ClassBodyOp,tbody,tfield);
    else
      tbody = new EXPNode(InterfaceBodyOp,tbody,tfield);

  /*
   * Costruisce il parse-tree dei metodi dichiarati nella classe.
   */
  
  Method_Pool *meths=Getmethodinfo();
  Method_Info *meth;
  _u2         meth_count=Getmethodcount();

  for (_u4 i=0; i < meth_count ; i++)
    {
      _u2 name_index;

      ttrow=Dummy;
      tmethod=Dummy;
      tparams=Dummy;

      meth=meths->GetMethod(i);
      name_index=meth->GetNameIndex();

      Constant_Utf8 *utf8=
	(Constant_Utf8 *)(Getconstantpool()->Get(name_index));
      String name;

      name=(char *)utf8->GetBytes();

      utf8=
	(Constant_Utf8 *)(Getconstantpool()->Get(meth->GetDescriptorIndex()));

      Descriptor descriptor;

      descriptor=(char *)(utf8->GetBytes());
      
      /*
       * Dal descrittore costruisco l'albero dei parametri con IDNode dal
       * nome fittizio, che mi serviranno solo per fare funzionare l'algoritmo
       * di Applicabilita' di invocazione di un metodo.
       */

      for (int z=1; z<=descriptor.num_args(); z++)
	{
	  String NullName;
	  Descriptor dparam;

	  NullName="<parameter null>";
	  dparam=descriptor.get_arg(z);
	  STNode *pnode=STABLE->Install_Id(NullName,FOURTH_LVL,Index,dparam);
	  STABLE->Discard(pnode);
	  if (tparams->IsDummy())
	    tparams=new EXPNode(ParameterOp,new IDNode(pnode),Dummy);
	  else
	    {
	      TreeNode *t=tparams;
	      for (; !t->GetRightC()->IsDummy(); t=t->GetRightC());
	      t->SetRightC(new EXPNode(ParameterOp,new IDNode(pnode),Dummy));
	    }
	}

      STNode *mnode=STABLE->Install_Id(name,THIRD_LVL,Index,descriptor);

      mnode->setaccess(meth->GetAccessFlags());

      for (_u4 j=0 ; j < meth->GetAttributesCount(); j++)
	{
	  _u2 po=meth->GetAttribute(j)->GetAttributeNameIndex();

	  utf8 =(Constant_Utf8 *)(Getconstantpool()->Get(po));
	  name=(char *)utf8->GetBytes();
	  
	  if (name=="Exceptions")
	    {
	      Exceptions_Attribute *eatt=
		(Exceptions_Attribute *) meth->GetAttribute(j);
	      for (_u4 k=0; k<eatt->GetNumberOfExceptions(); k++)
		{
		  _u2 con=eatt->GetException(k);

		  _u2 noi=((Constant_Class *)
			   Getconstantpool()->Get(con))->GetNameIndex();
		  utf8 = (Constant_Utf8 *)
		    (Getconstantpool()->Get(noi));
		  
		  name=(char *) utf8->GetBytes();

		  STNode *tnode;

		  if ((tnode=Unit->LoadClass(name,++LastIndex))==NULL)
		    {
		      Descriptor descriptor;

		      descriptor.build_link(name);
 		      tnode=STABLE->Install_Id(name,-1,-1,descriptor);
		      STABLE->Discard(tnode);
		      Unit->MsgErrors(0,msg_errors[ERR_CLASS_NOT_FOUND],
				      Name.to_char(),"load class");
		    }
		  
		  ttrow = new EXPNode(ThrowsOp,ttrow,new IDNode(tnode)); 
		}
	    }
	}
      
      TreeNode *metheader = new EXPNode(MethodHeaderOp,
					new EXPNode(CommaOp, 
						    new IDNode(mnode),
						    tparams),
					ttrow);
      
      tmethod = new EXPNode(MethodDeclOp,metheader,Dummy);

      mnode->setmyheader(tmethod);

      mnode->setmyclass(ClassNode);

      if(Getmethodcount() > 0)
	{
	  if(!is_interface)
	    tbody = new EXPNode(ClassBodyOp,tbody,tmethod);
	  else
	    tbody = new EXPNode(InterfaceBodyOp,tbody,tmethod);
	}
    }

  /*
   * Dopo i campi e i metodi tocca alle informazioni inerenti alla classe
   * (o interfaccia) corrente.
   */

  for (_u4 i=0 ; i < Getinterfacescount() ; i++)
    {
      _u2 intf_index;

      intf_index=Getinterfaces()->GetInterface(i);

      Constant_Class *cclass=
	(Constant_Class *)(Getconstantpool()->Get(intf_index));
      Constant_Utf8  *utf8=
	(Constant_Utf8 *)(Getconstantpool()->Get(cclass->GetNameIndex()));
      String name;

      name=(char *)utf8->GetBytes();

      STNode *inode;

      if ((inode=Unit->LoadClass(name,++LastIndex))==NULL)
	{
	  Descriptor descriptor;

	  descriptor.build_link(name);
	  inode=STABLE->Install_Id(name,-1,-1,descriptor);
	  STABLE->Discard(inode);
	  Unit->MsgErrors(0,msg_errors[ERR_CLASS_NOT_FOUND],Name.to_char(),
			  "load class");
	}

      /*
       * Se stiamo caricando una classe, allora le interfacce che stiamo
       * caricando, sono quelle da lei implementate. Se stiamo in un'interfac-
       * cia, allora stiamo caricando le interfacce ereditate.
       */

      if (!is_interface)
	tintf = new EXPNode(ImplementsOp,tintf,new IDNode(inode)); 
      else
	tintf = new EXPNode(ExtendsOp,tintf,new IDNode(inode));
    }

  /*
   * Se stiamo in una classe, bisogna caricare la sua superclasse.
   */

  _u2 super_index;

  if ((super_index=Getsuperclass()) > 0 )
    {
      if(!is_interface)
	{
	  Constant_Class *sclass=
	    (Constant_Class *)(Getconstantpool()->Get(super_index));
	  Constant_Utf8 *utf8=(Constant_Utf8 *)
	    (Getconstantpool()->Get(sclass->GetNameIndex()));
	  String name;

	  name=(char *)utf8->GetBytes(); 
	  
	  STNode *snode;

	  if ((snode=Unit->LoadClass(name,++LastIndex))==NULL)
	    {
	      Descriptor descriptor;

	      descriptor.build_link(name);
	      snode=STABLE->Install_Id(name,-1,-1,descriptor);
	      STABLE->Discard(snode);
	      Unit->MsgErrors(0,msg_errors[ERR_CLASS_NOT_FOUND],Name.to_char(),
			      "load class");
	    }

	  tsuper = new EXPNode(ExtendsOp,Dummy,new IDNode(snode));
	}
    }
  else
    {
      /*
       * La classe in questione e' java/lang/Object. In questa versione noi
       * non compileremo mai java/lang/Object, per cui non si verifichera'
       * mai questa condizione.
       */

      tsuper = new EXPNode(ExtendsOp,Dummy,Dummy);
    }  

  if(!is_interface)
    tsuper = new EXPNode(CommaOp,tsuper,tintf);
   
  /*
   * Carico le informazioni della classe o interfaccia.
   */

  if(!is_interface)
    tree=new EXPNode(ClassOp,
		     new EXPNode(ClassHeaderOp,new IDNode(ClassNode),tsuper),
		     tbody);
  else
    tree=new EXPNode(InterfaceOp,
		     new EXPNode(InterfaceHeaderOp,
				 new IDNode(ClassNode),tintf),
		     tbody);

  ClassNode->setmyheader(tree);

  FILE *stream;

  stream=fopen(NameTree.to_char(),"w");
  tree->PrintNode(stream,0);
  fclose(stream);

  return ClassNode;
}

/*****************************************************************************
 * Qui inizia l'implementazione dei metodi di Class_File utilizzati nella    *
 * fase di generazione del codice. I metodi:                                 *
 *                                                                           *
 * Class_File::Load_Constant_Utf8                                            *
 * Class_File::Load_Constant_Class                                           *
 * Class_File::Load_Constant_NameAndType                                     *
 * Class_File::Load_Fieldref                                                 *
 * Class_File::Load_Methodref                                                *
 * Class_File::Load_IntfMethodref                                            *
 * Class_File::Load_Constant_Integer                                         *
 * Class_File::Load_Constant_Float                                           *
 * Class_File::Load_Constant_String                                          *
 * Class_File::Load_Constant_Double                                          *
 * Class_File::Load_Constant_Long                                            *
 *                                                                           *
 * sono utilizzati per inserire gli item nella constant pool evitando dupli- *
 * volendo si potrebbe anche fare a meno di essi, pero' si e' visto che con  *
 * tali metodi risultava piu' semplice implementare la fase di generazione   *
 * del codice e la sua leggibilita'.                                         *
 *****************************************************************************/

int Class_File::Load_Constant_Utf8(String& Name)
{
  CPHash *cphash=Unit->GetCPHash();
  CPHNode *n;
  Constant_Utf8 *utf8;
  int index;

  if ((n=cphash->LookUp(Name,CONSTANT_Utf8)))
    return n->getindex();
  
  utf8=new Constant_Utf8(Name.to_char());
  index=Setconstantpool(utf8);
  n=cphash->Install_Id(Name,CONSTANT_Utf8);
  n->setindex(index);
  return index;
}

int Class_File::Load_Constant_Class(String& Name)
{
  CPHash *cphash=Unit->GetCPHash();
  CPHNode *n;
  Constant_Class *c_class;
  int index;

  if ((n=cphash->LookUp(Name,CONSTANT_Class)))
    return n->getindex();

  c_class=new Constant_Class(Load_Constant_Utf8(Name));
  index=Setconstantpool(c_class);
  n=cphash->Install_Id(Name,CONSTANT_Class);
  n->setindex(index);
  return index;
}

int Class_File::Load_Constant_NameAndType(String& key)
{
  int i, index;
  CPHash *cphash=Unit->GetCPHash();
  Constant_Name_And_Type *name_and_type;
  String Name;
  String descriptor;
  CPHNode *n;

  if ((n=cphash->LookUp(Name,CONSTANT_NameAndType)))
    return n->getindex();

  for (i=1; i<=key.getlength(); i++)
    if (key[i]==',')
      break;

  Name=key.cut(1,i-1);
  descriptor=key.cut(i+1,key.getlength());

  name_and_type=new Constant_Name_And_Type(Load_Constant_Utf8(descriptor),
					   Load_Constant_Utf8(Name));
  index=Setconstantpool(name_and_type);
  n=cphash->Install_Id(key,CONSTANT_NameAndType);
  n->setindex(index);
  return index;
}

int Class_File::Load_Fieldref(String& key)
{
  int i, j, index;
  CPHash *cphash=Unit->GetCPHash();
  String NameAndType, ClassName;
  CPHNode *n;
  Constant_Fieldref_info *field;
  
  if ((n=cphash->LookUp(key,CONSTANT_Fieldref)))
    return n->getindex();

  for (i=1; i<=key.getlength(); i++)
    if (key[i]==',')
      break;

  ClassName=key.cut(1,i-1);
  NameAndType=key.cut(i+1,key.getlength());

  field=new Constant_Fieldref_info(Load_Constant_NameAndType(NameAndType),
				   Load_Constant_Class(ClassName));
  index=Setconstantpool(field);
  n=cphash->Install_Id(key,CONSTANT_Fieldref);
  n->setindex(index);
  return index;  
}

int Class_File::Load_Methodref(String& key)
{
  int i, j, index;
  CPHash *cphash=Unit->GetCPHash();
  String NameAndType, ClassName;
  CPHNode *n;
  Constant_Methodref_info *meth;
  
  if ((n=cphash->LookUp(key,CONSTANT_Methodref)))
    return n->getindex();

  for (i=1; i<=key.getlength(); i++)
    if (key[i]==',')
      break;

  ClassName=key.cut(1,i-1);
  NameAndType=key.cut(i+1,key.getlength());

  meth=new Constant_Methodref_info(Load_Constant_NameAndType(NameAndType),
				   Load_Constant_Class(ClassName));
  index=Setconstantpool(meth);
  n=cphash->Install_Id(key,CONSTANT_Methodref);
  n->setindex(index);
  return index;  
}

int Class_File::Load_IntfMethodref(String& key)
{
  int i, j, index;
  CPHash *cphash=Unit->GetCPHash();
  String NameAndType, ClassName;
  CPHNode *n;
  Constant_IntMethodref_info *meth;
  
  if ((n=cphash->LookUp(key,CONSTANT_IntMethodref)))
    return n->getindex();

  for (i=1; i<=key.getlength(); i++)
    if (key[i]==',')
      break;

  ClassName=key.cut(1,i-1);
  NameAndType=key.cut(i+1,key.getlength());

  meth=new Constant_IntMethodref_info(Load_Constant_NameAndType(NameAndType),
				      Load_Constant_Class(ClassName));
  index=Setconstantpool(meth);
  n=cphash->Install_Id(key,CONSTANT_IntMethodref);
  n->setindex(index);
  return index;  
}

int Class_File::Load_Constant_String(String& Name)
{
  CPHash *cphash=Unit->GetCPHash();
  CPHNode *n;
  Constant_String *str;
  int index;

  if ((n=cphash->LookUp(Name,CONSTANT_String)))
    return n->getindex();
  
  str=new Constant_String(Load_Constant_Utf8(Name));
  index=Setconstantpool(str);
  n=cphash->Install_Id(Name,CONSTANT_String);
  n->setindex(index);
  return index;
}

int Class_File::Load_Constant_Integer(long valore)
{
  int index;
  String key;
  char *str=new char[20];
  CPHNode *n;
  CPHash *cphash=Unit->GetCPHash();
  Constant_Integer *valint;

  sprintf(str,"%d",valore);
  key=str;
  delete str;

  if ((n=cphash->LookUp(key,CONSTANT_Integer)))
    return n->getindex();

  valint=new Constant_Integer(valore);
  index=Setconstantpool(valint);
  n=cphash->Install_Id(key,CONSTANT_Integer);
  n->setindex(index);
  return index;
}

int Class_File::Load_Constant_Long(long long valore)
{
  int index;
  String key;
  char *str=new char[20];
  CPHNode *n;
  CPHash *cphash=Unit->GetCPHash();
  Constant_Long *valint;

  sprintf(str,"%d",valore);
  key=str;
  delete str;

  if ((n=cphash->LookUp(key,CONSTANT_Long)))
    return n->getindex();

  valint=new Constant_Long(valore & 0xFFFFFFFF, valore >> 32);
  index=Setconstantpool(valint);
  n=cphash->Install_Id(key,CONSTANT_Long);
  n->setindex(index);
  return index;
}

#include <netinet/in.h>

int Class_File::Load_Constant_Float(double valore)
{
  int index;
  String key;
  char *str=new char[20];
  CPHNode *n;
  CPHash *cphash=Unit->GetCPHash();
  Constant_Float *valfloat;

  sprintf(str,"%f",valore);
  key=str;
  delete str;

  if ((n=cphash->LookUp(key,CONSTANT_Float)))
    return n->getindex();

  _u4 bytes;

  float val=(float)valore;

  memcpy(&bytes, ((char *)&val), 4);

  bytes = htonl(bytes);

  valfloat=new Constant_Float(bytes);
  index=Setconstantpool(valfloat);
  n=cphash->Install_Id(key,CONSTANT_Float);
  n->setindex(index);
  return index;
}

int Class_File::Load_Constant_Double(double valore)
{
  int index;
  String key;
  char *str=new char[20];
  CPHNode *n;
  CPHash *cphash=Unit->GetCPHash();
  Constant_Double *valfloat;

  /*
   * Probabilmente valore va convertito secondo le specifiche JVM. Vedi
   * il libro: " The Java Language Specification; pag. 97.
   */

  sprintf(str,"%f",valore);
  key=str;
  delete str;

  if ((n=cphash->LookUp(key,CONSTANT_Double)))
    return n->getindex();

  _u4 high_byte, low_byte;

  memcpy(&high_byte, ((char *)&valore), 4);
  memcpy(&low_byte, ((char *)&valore)+4, 4);

  high_byte=ntohl(high_byte);
  low_byte=ntohl(low_byte);

  valfloat=new Constant_Double(high_byte,low_byte);  // da correggere
  index=Setconstantpool(valfloat);
  n=cphash->Install_Id(key,CONSTANT_Double);
  n->setindex(index);
  return index;
}

/*
 * Class_File::WriteClass
 *
 * Scarica il bytecode di una classe su disco in un file .class.
 * Chiaramente questo metodo lavora con un class file precedentemente ordinato.
 */

void Class_File::WriteClass(String& Name)
{
  String NameClass;

  NameClass=Name+".class";

  ofstream cFile(NameClass.to_char(),ios::out);
  if (! cFile)
    {
      Unit->MsgErrors(0,msg_errors[ERR_CANT_WRITE],NameClass.to_char());
      return;
    }
   
  cFile << *this;
  &flush(cFile);
  cFile.close();
}

ostream& operator << ( ostream& os, Class_File& cp)
{
  Shotu4(os,cp.magic);
  Shotu2(os,cp.minor_version);
  Shotu2(os,cp.major_version);
  Shotu2(os,cp.constant_pool_count);
  if (cp.constant_pool_count > 0)
    os << *cp.c_pool;
  Shotu2(os,cp.access_flags);
  Shotu2(os,cp.this_class);
  Shotu2(os,cp.super_class);
  Shotu2(os,cp.interfaces_count);  
  if (cp.interfaces_count  > 0)
    os << *cp.i_pool;
  Shotu2(os,cp.field_count);
  if (cp.field_count > 0)
    os << *cp.f_pool;
  Shotu2(os,cp.method_count);
  if (cp.method_count > 0)
    os << *cp.m_pool;
  Shotu2(os,cp.attributes_count);
  if (cp.attributes_count > 0)
  cp.attr[0]->Write(os);
  return os;
}


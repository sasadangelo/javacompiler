/*
 * file jvm.h
 *
 * descrizione: questo file definisce l'interfaccia alla JVM.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it
 */

#ifndef JVM_H
#define JVM_H

/* 
 * tag della CONSTANT POOL
 */

#define CONSTANT_Class        0x07
#define CONSTANT_Fieldref     0x09
#define CONSTANT_Methodref    0x0A
#define CONSTANT_IntMethodref 0x0B
#define CONSTANT_String       0x08
#define CONSTANT_Integer      0x03
#define CONSTANT_Float        0x04
#define CONSTANT_Long         0x05
#define CONSTANT_Double       0x06
#define CONSTANT_NameAndType  0x0C
#define CONSTANT_Utf8         0x01

#define b0(w) ((char *)&w)[0];
#define b1(w) ((char *)&w)[1];
#define b2(w) ((char *)&w)[2];
#define b3(w) ((char *)&w)[3];

#include <dyn_table.h>

class TreeNode;

class Entry_Pool
{
private:
  _u1 tag;

  friend ostream& operator << (ostream&,Entry_Pool&);

public:
  Entry_Pool();         
  Entry_Pool(_u1);             
  ~Entry_Pool();
  _u1  GetTag();
  void SetTag(_u1);
  virtual void Write(ostream &);
  virtual void Info(ostream &);
};

class Constant_Class : public Entry_Pool
{
private:
    _u2 name_index;

    friend ostream& operator << (ostream&,Constant_Class&);
  
public:
  Constant_Class(_u2);
  ~Constant_Class();
  _u2  GetNameIndex();
  void Write(ostream &);
  void Info(ostream &);
};

class Constant_Fieldref_info : public Entry_Pool
{
private:
  _u2 class_index;
  _u2 name_and_type_index;

  friend ostream& operator << (ostream&,Constant_Fieldref_info&);
  
public:
  Constant_Fieldref_info(_u2, _u2);
  ~Constant_Fieldref_info();
  _u2  GetClassIndex();
  _u2  GetNameAndTypeIndex();
  void SetClassIndex(_u2);
  void SetNameAndTypeIndex(_u2);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_Methodref_info : public Entry_Pool
{
private:
  _u2 class_index;
  _u2 name_and_type_index;

  friend ostream& operator << (ostream&,Constant_Methodref_info&);
  
public:
  Constant_Methodref_info(_u2, _u2);
  ~Constant_Methodref_info();
  _u2  GetClassIndex();
  _u2  GetNameAndTypeIndex();
  void SetClassIndex(_u2); 
  void SetNameAndTypeIndex(_u2);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_IntMethodref_info : public Entry_Pool
{
private:
  _u2 class_index;
  _u2 name_and_type_index;

  friend ostream& operator << (ostream&,Constant_IntMethodref_info&);
  
public:
  Constant_IntMethodref_info(_u2, _u2);
  ~Constant_IntMethodref_info();
  _u2  GetClassIndex();
  _u2  GetNameAndTypeIndex();
  void SetClassIndex(_u2);
  void SetNameAndTypeIndex(_u2);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_String : public Entry_Pool
{
private:
  _u2 string_index;

  friend ostream& operator << (ostream&,Constant_String& );
  
public:
  Constant_String(_u2);
  ~Constant_String();
  _u2  GetStringIndex();
  void SetStringIndex(_u2);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_Name_And_Type : public Entry_Pool
{
private:
  _u2 name_index;
  _u2 descriptor_index;

  friend ostream& operator << (ostream& ,Constant_Name_And_Type&);

public:
  Constant_Name_And_Type(_u2,_u2);
  ~Constant_Name_And_Type();
  _u2  GetNameIndex();
  _u2  GetDescriptorIndex();
  void SetNameIndex(_u2);
  void SetDescriptorIndex(_u2);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_Utf8 : public Entry_Pool
{
private:
  _u2 length;
  _u1 *bytes;

  friend ostream& operator << (ostream& ,Constant_Utf8&);

public:
  Constant_Utf8(char *);
  ~Constant_Utf8();             
  _u1 *GetBytes();
  void Write(ostream &);
  void Info(ostream &);
};

class Constant_Integer : public Entry_Pool
{
private:
  _u4 bytes;

  friend ostream& operator << (ostream&,Constant_Integer&);

public:
  Constant_Integer(_u4);
  ~Constant_Integer();
  _u4  GetBytes();
  void SetBytes(_u4);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_Float : public Entry_Pool
{
private:
  _u4 bytes;

  friend ostream& operator << (ostream&,Constant_Float&);

public:
  Constant_Float(_u4);
  ~Constant_Float();
  _u4  GetBytes();
  void SetBytes(_u4);
  void Write(ostream&);
  void Info(ostream&);
};

class Constant_Long : public Entry_Pool
{
private:
  _u4 high_bytes;
  _u4 low_bytes;

  friend ostream& operator << (ostream&,Constant_Long&);

public:
  Constant_Long(_u4,_u4);
  ~Constant_Long();
  _u4  GetHighBytes();
  _u4  GetLowBytes();
  void SetHighBytes(_u4);
  void SetLowBytes(_u4);
  void Write(ostream &);
  void Info(ostream &);
};

class Constant_Double : public Entry_Pool
{
private:
  _u4 high_bytes;
  _u4 low_bytes;

  friend ostream& operator << (ostream&,Constant_Double&);

public:
  Constant_Double(_u4,_u4);
  ~Constant_Double();
  _u4  GetHighBytes();
  _u4  GetLowBytes();
  void SetHighBytes(_u4);
  void SetLowBytes(_u4);
  void Write(ostream &);
  void Info(ostream &);
};

class Constant_Pool
{
private:
  DYNTable<Entry_Pool *> *table;

  friend ostream& operator << (ostream&,Constant_Pool&);
  
public:
  Constant_Pool();
  ~Constant_Pool();
  _u2  AddInPool(Entry_Pool *);
  Entry_Pool *Get(_u2);
  _u2  GetLen();
  void Info(ostream &);
};

class Interfaces_Pool
{
private:
  DYNTable<_u2> *interfaces_table;

  friend ostream& operator << (ostream& ,Interfaces_Pool& );

public:
  Interfaces_Pool();
  ~Interfaces_Pool();
  _u2  AddInterfaces(_u2 val);
  _u2  GetInterface(_u2 val);
  void Info(ostream &,Constant_Pool *);
};

class Entry_Attribute
{
protected:
    _u2 attribute_name_index;
    _u4 attribute_length;

  friend ostream& operator << (ostream&,Entry_Attribute&);

public:
  Entry_Attribute();
  Entry_Attribute(_u2, _u4);                       
  ~Entry_Attribute();
  void SetAttributeNameIndex(_u2);
  void SetAttributeLength(_u4);
  _u2  GetAttributeNameIndex();
  _u4  GetAttributeLength();
  virtual void Write(ostream &);
  virtual void Info(ostream &,Constant_Pool *);
};
 
class Source_File_Attribute : public Entry_Attribute
{
private:
  _u2 source_file_index;

  friend ostream& operator << (ostream&,Source_File_Attribute&);

public:
  Source_File_Attribute();
  Source_File_Attribute(_u2);
  Source_File_Attribute(_u2,_u2);
  ~Source_File_Attribute();
  void SetSourceFileIndex(_u2);
  _u2  GetSourceFileIndex();
  void Write(ostream &);
  void Info(ostream &,Constant_Pool *);
};

class Constant_Value_Attribute : public Entry_Attribute
{
private:
  _u2 constant_value_index;

  friend ostream& operator << (ostream&,Constant_Value_Attribute&);

public:
  Constant_Value_Attribute();
  Constant_Value_Attribute(_u2);
  Constant_Value_Attribute(_u2,_u2);
  ~Constant_Value_Attribute();
  void SetConstantValueIndex(_u2);
  _u2  GetConstantValueIndex();
  void Write(ostream&);
  void Info(ostream &,Constant_Pool *);
};

class Exception_Parameter
{
private:
  _u2 start_pc;
  _u2 end_pc;
  _u2 handler_pc;
  _u2 catch_type;

  friend ostream& operator << (ostream& ,Exception_Parameter& );

public:
  Exception_Parameter();
  ~Exception_Parameter();
  _u2  GetStartPc();
  _u2  GetEndPc();
  _u2  GetHandlerPc();
  _u2  GetCatchType();
  void SetStartPc(_u2);
  void SetEndPc(_u2);
  void SetHandlerPc(_u2);
  void SetCatchType(_u2);
  void Write(ostream&);
  void Info(ostream &,Constant_Pool *);
};

class Code_Attribute : public Entry_Attribute
{
private:
  _u2 max_stack;
  _u2 max_locals;
  _u4 code_length;
  DYNTable<_u1> *code;
  _u2 exception_table_length;

  DYNTable<Exception_Parameter *> *exception_table;

  _u2 attributes_count;

  friend ostream& operator << (ostream&,Code_Attribute&);

public:
  Code_Attribute();
  ~Code_Attribute();
  _u2  GetMaxStack();
  _u2  GetMaxLocals();
  _u4  GetCodeLength();
  _u2  GetExceptionTableLength();
  Exception_Parameter *GetExceptionParameter(int);
  void SetExceptionParameter(Exception_Parameter*, int);
  _u2  GetAttributesCount();
  _u1  GetCode(_u2);

  _u4  SetCode(_u1);           

  void Gen(_u1);
  void Gen(_u1, _u1);
  void Gen(_u1, _u2);  
  void Gen(_u1, _u1, char);
  void Gen(_u1, _u2, _u1);

  void Gen_to(_u4, _u1, _u2);
  void Gen_to(_u4, _u4);

  void SetMaxStack(_u2);
  void SetMaxLocals(_u2);
  void SetCodeLength(_u4);
  void SetExceptionTableLength();
  void SetAttributesCount(_u2);
  void AddException(Exception_Parameter *);
  void Write(ostream&);
  void Info(ostream &,Constant_Pool *);
};

class Exceptions_Attribute : public Entry_Attribute
{
private:
  _u2 number_of_exceptions;
  DYNTable<_u2> *table;

  friend ostream& operator << (ostream&,Exceptions_Attribute&);

public:
  Exceptions_Attribute();
  Exceptions_Attribute(_u2);
  ~Exceptions_Attribute();
  void SetAttributeNameIndexLength(_u2,_u4);
  void SetNumberOfExceptions(_u2);
  _u2  SetException(_u2);
  _u2  GetException(_u2);
  _u2  GetNumberOfExceptions();
  void Write(ostream&);
  void Info(ostream &,Constant_Pool *);
};

class Method_Info
{
private:
  _u2 access_flags;
  _u2 name_index;
  _u2 descriptor_index;
  _u2 attributes_count;
  Entry_Attribute **attributes;

  friend ostream& operator << (ostream&,Method_Info&); 

public:
  Method_Info();
  ~Method_Info();

  _u2  GetAccessFlags();
  _u2  GetNameIndex();
  _u2  GetDescriptorIndex();
  _u2  GetAttributesCount();
  Entry_Attribute *GetAttribute(_u2);
  void SetArray(_u2);
  void SetAccessFlags(_u2);
  void SetNameIndex(_u2);
  void SetDescriptorIndex(_u2);
  void SetAttributesCount(_u2);
  void SetAttribute(Entry_Attribute *,_u2);
  void Info(ostream&,Constant_Pool *);
  void Write(ostream&);
};

class Method_Pool
{
private:
  DYNTable<Method_Info *> *method_table;

  friend ostream& operator << (ostream& ,Method_Pool& );

public:
  Method_Pool();
  ~Method_Pool();
  _u2  AddMethod(Method_Info *);
  DYNTable<Method_Info *> *GetTable();
  Method_Info *GetMethod(_u2);
  void Info(ostream&,Constant_Pool *);
  void Write(ostream&);
};

class Field_Info
{
  private :
  _u2 access_flags;
  _u2 name_index;
  _u2 descriptor_index;
  _u2 attributes_count;
  Entry_Attribute **attributes;

  friend ostream& operator << (ostream&,Field_Info&); 

  public :
  Field_Info();
  ~Field_Info();

  _u2  GetAccessFlags();
  _u2  GetNameIndex();
  _u2  GetDescriptorIndex();
  _u2  GetAttributesCount();
  Entry_Attribute *GetAttribute(_u2);
  void SetArray(_u2);
  void SetAccessFlags(_u2);
  void SetNameIndex(_u2);
  void SetDescriptorIndex(_u2);
  void SetAttributesCount(_u2);
  void SetAttribute(Entry_Attribute *,_u2);
  void Info(ostream&,Constant_Pool *);
  void Write(ostream&);
};

class Field_Pool
{
private:
  DYNTable<Field_Info *> *field_table;

  friend ostream& operator << (ostream& ,Field_Pool&);

public:
  Field_Pool();
  ~Field_Pool();
  _u2 AddField(Field_Info *);
  DYNTable<Field_Info *> *GetTable();
  Field_Info *GetField(_u2);
  void Info(ostream&,Constant_Pool *);
  void Write(ostream&);
};

class Class_File
{
private:
  _u4 magic;
  _u2 minor_version;
  _u2 major_version;
  _u2 constant_pool_count;
  Constant_Pool *c_pool;
  _u2 access_flags;
  _u2 this_class;
  _u2 super_class;
  _u2 interfaces_count;
  Interfaces_Pool *i_pool;
  _u2 field_count;
  Field_Pool *f_pool;
  _u2 method_count;
  Method_Pool *m_pool;
   
  _u2 attributes_count;
  Entry_Attribute **attr;

  friend ostream& operator << (ostream&,Class_File&);
 
public:
  Class_File();
  ~Class_File();
 
  _u4  Getmagic();
  _u2  Getminor();
  _u2  Getmajor();
  _u2  Getconstantpoolcount();
  Constant_Pool *Getconstantpool();
  _u2  Getaccessflags();
  _u2  Getthisclass();
  _u2  Getsuperclass();
  _u2  Getinterfacescount();
  Interfaces_Pool *Getinterfaces();
  _u2  Getfieldcount();
  Field_Pool *Getfieldinfo();
  _u2  Getmethodcount();
  Method_Pool *Getmethodinfo();
  _u2  Getattributescount();
  Entry_Attribute *Getattributeinfo();
  void Setmagic(_u4);
  void Setminor(_u2);
  void Setmajor(_u2);
  void Setconstantpoolcount(_u2);
  unsigned Setconstantpool(Entry_Pool *); 
  void Setaccessflags(_u2);
  void Setthisclass(_u2);
  void Setsuperclass(_u2);
  void Setinterfacescount(_u2);
  _u2  Setinterfaces(_u2); 
  void Setfieldcount(_u2);
  _u2  Setfieldinfo(Field_Info *);
  void Setmethodcount(_u2 );
  _u2  Setmethodinfo(Method_Info *);
  void Setattributescount(_u2);
  void SetAttributes(Entry_Attribute *);

  void   Info(ostream&);
  void   WriteClass(String&);
  int    ReadClass(String&,int); 
  STNode *LoadClass(String&,int);

  int Load_Constant_Utf8(String&);
  int Load_Constant_Class(String&);
  int Load_Constant_NameAndType(String&);
  int Load_Fieldref(String&);
  int Load_Methodref(String&);
  int Load_IntfMethodref(String&);
  int Load_Constant_String(String&);
  int Load_Constant_Integer(long);
  int Load_Constant_Long(long long);
  int Load_Constant_Float(double);
  int Load_Constant_Double(double);
};  

#endif

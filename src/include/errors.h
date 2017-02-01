/*
 * file errors.h
 *
 * descrizione: definizione della classe ListErrors e dei codici di errore
 *              del compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it
 */

#ifndef ERRORS_H
#define ERRORS_H

#define  ERR_EOF_IN_COMMENT              0
#define  ERR_EOF_IN_STRING               1
#define  ERR_NEWLINE_IN_STRING           2
#define  ERR_INVALID_CHAR_CONSTANT       3
#define  ERR_UNBALANCED_PAREN            4
#define  ERR_INVALID_ESCAPE_CHAR         5
#define  ERR_INVALID_OCTAL_NUMBER        6
#define  ERR_INVALID_NUMBER              7
#define  ERR_FUNNY_CHAR                  8
#define  ERR_FLOAT_FORMAT                9
#define  ERR_OVERFLOW                   10
#define  ERR_UNDERFLOW                  11
#define  ERR_TOKEN_EXPECTED             12
#define  ERR_STATEMENT_EXPECTED         13
#define  ERR_TYPE_EXPECTED              14
#define  ERR_IDENTIFIER_EXPECTED        15
#define  ERR_CLASS_EXPECTED             16
#define  ERR_TOPLEVEL_EXPECTED          17
#define  ERR_MISSING_TERM               18
#define  ERR_ELSE_WITHOUT_IF            19
#define  ERR_CATCH_WITHOUT_TRY          20
#define  ERR_FINALLY_WITHOUT_TRY        21
#define  ERR_TRY_WITHOUT_CATCH_FINALLY  22
#define  ERR_CASE_WITHOUT_SWITCH        23
#define  ERR_DEFAULT_WITHOUT_SWITCH     24
#define  ERR_IO_EXCEPTION               25
#define  ERR_ARRAY_INDEX_REQUIRED       26
#define  ERR_NOT_ARRAY                  27
#define  ERR_ARRAY_DIM_IN_DECL          28
#define  ERR_ARRAY_DIM_IN_TYPE          29
#define  ERR_INVALID_ARRAY_EXPR         30
#define  ERR_INVALID_ARRAY_INIT         31
#define  ERR_INVALID_LHS_ASSIGNMENT     32
#define  ERR_INVALID_ARGS               33
#define  ERR_INVALID_CAST               34
#define  ERR_INVALID_INSTANCEOF         35
#define  ERR_INVALID_TYPE_EXPR          36
#define  ERR_INVALID_FIELD_REFERENCE    37
#define  ERR_NO_SUCH_FIELD              38
#define  ERR_NO_FIELD_ACCESS            39 
#define  ERR_NO_STATIC_FIELD_ACCESS     40
#define  ERR_AMBIG_FIELD                41
#define  ERR_INVALID_FIELD              42
#define  ERR_ASSIGN_TO_FINAL            43
#define  ERR_UNDEF_VAR                  44
#define  ERR_VAR_NOT_INITIALIZED        45
#define  ERR_ACCESS_INST_BEFORE_SUPER   46
#define  ERR_AMBIG_CLASS                47
#define  ERR_INVALID_ARG                48
#define  ERR_INVALID_ARG_TYPE           49
#define  ERR_INVALID_LENGTH             50
#define  ERR_INVALID_CONSTR_INVOKE      51
#define  ERR_CONSTR_INVOKE_NOT_FIRST    52
#define  ERR_INVALID_METHOD_INVOKE      53
#define  ERR_UNDEF_METH                 54
#define  ERR_NO_METH_ACCESS             55
#define  ERR_NO_STATIC_METH_ACCESS      56
#define  ERR_INVALID_PROTECTED_METHOD_USE  57
#define  ERR_INVALID_PROTECTED_FIELD_USE   58
#define  ERR_INVALID_METHOD             59
#define  ERR_INVALID_ARRAY_DIM          60
#define  ERR_AMBIG_CONSTR               61
#define  ERR_EXPLICIT_CAST_NEEDED       62
#define  ERR_INCOMPATIBLE_TYPE          63
#define  ERR_INVALID_TERM               64
#define  ERR_ABSTRACT_CLASS             65
#define  ERR_ABSTRACT_CLASS_NOT_FINAL   66
#define  ERR_NEW_INTF                   67
#define  ERR_INVOKE_ABSTRACT            68
#define  ERR_UNMATCHED_METH             69
#define  ERR_UNMATCHED_CONSTR           70
#define  ERR_FORWARD_REF                71
#define  ERR_ARRAY_DIM_MISSING          72
#define  ERR_NEW_ABSTRACT               73
#define  ERR_LABEL_NOT_FOUND            74
#define  ERR_INVALID_BREAK              75
#define  ERR_INVALID_CONTINUE           76
#define  ERR_INVALID_DECL               77
#define  ERR_RETURN_WITH_VALUE          78
#define  ERR_RETURN_WITHOUT_VALUE       79
#define  ERR_RETURN_INSIDE_STATIC_INITIALIZER  80
#define  ERR_INVALID_LABEL              81
#define  ERR_RETURN_REQUIRED_AT_END     82
#define  ERR_DUPLICATE_LABEL            83
#define  ERR_SWITCH_OVERFLOW            84
#define  ERR_CONST_EXPR_REQUIRED        85
#define  ERR_DUPLICATE_DEFAULT          86
#define  ERR_NOT_SUPPORTED              87
#define  ERR_RETURN_WITH_VALUE_CONSTR   88
#define  ERR_PACKAGE_REPEATED           89
#define  ERR_CLASS_MULTIDEF             90
#define  ERR_CLASS_MULTIDEF_IMPORT      91
#define  ERR_FINAL_METH_OVERRIDE        92
#define  ERR_REDEF_RETURN_TYPE          93
#define  ERR_OVERRIDE_STATIC_METH       94
#define  ERR_OVERRIDE_INSTANCE_METHOD_STATIC  95
#define  ERR_OVERRIDE_PUBLIC            96
#define  ERR_OVERRIDE_PROTECTED         97
#define  ERR_OVERRIDE_PRIVATE           98
#define  ERR_INTF_CONSTRUCTOR           99
#define  ERR_CONSTR_MODIFIER           100
#define  ERR_INTF_INITIALIZER          101
#define  ERR_INTF_MODIFIER_METHOD      102
#define  ERR_INTF_MODIFIER_FIELD       103
#define  ERR_TRANSIENT_METH            104 
#define  ERR_VOLATILE_METH             105
#define  ERR_STATIC_MODIFIER           106
#define  ERR_INVALID_METH_BODY         107
#define  ERR_VAR_MODIFIER              108
#define  ERR_TRANSIENT_MODIFIER        109
#define  ERR_INITIALIZER_NEEDED        110
#define  ERR_METH_MULTIDEF             111
#define  ERR_METH_REDEF_RETTYPE        112
#define  ERR_VAR_MULTIDEF              113
#define  ERR_INTF_SUPER_CLASS          114
#define  ERR_CANT_ACCESS_CLASS         115
#define  ERR_REPEATED_MODIFIER         116
#define  ERR_SUPER_IS_FINAL            117
#define  ERR_SUPER_IS_INTF             118
#define  ERR_CYCLIC_SUPER              119
#define  ERR_CYCLIC_INTF               120
#define  ERR_NOT_INTF                  121
#define  ERR_FINAL_INTF                122
#define  ERR_INTF_IMPL_INTF            123
#define  ERR_MULTIPLE_INHERIT          124
#define  ERR_INTF_REPEATED             125
#define  ERR_CLASS_FORMAT              126
#define  ERR_NO_METH_BODY              127
#define  ERR_NO_CONSTRUCTOR_BODY       128
#define  ERR_VOID_INST_VAR             129
#define  ERR_INVALID_METHOD_DECL       130
#define  ERR_SUPER_NOT_FOUND           131
#define  ERR_INTF_NOT_FOUND            132
#define  ERR_FINAL_ABSTRACT            133
#define  ERR_VOID_ARGUMENT             134
#define  ERR_INVALID_EXPR              135
#define  ERR_CATCH_NOT_REACHED         136
#define  ERR_STAT_NOT_REACHED          137
#define  ERR_ARITHMETIC_EXCEPTION      138
#define  ERR_GENERIC                   139
#define  ERR_PUBLIC_CLASS_FILE         140
#define  ERR_LOSE_PRECISION            141
#define  ERR_DUPLICATE_ARGUMENT        142
#define  ERR_LOCAL_REDEFINED           143
#define  ERR_PRIVATE_CLASS             144
#define  ERR_RECURSIVE_CONSTR          145
#define  ERR_WRONG_CLASS               146
#define  ERR_CLASS_NOT_FOUND           147
#define  ERR_PACKAGE_NOT_FOUND         148
#define  ERR_INVALID_THROWS            149
#define  ERR_THROWS_NOT_THROWABLE      150
#define  ERR_THROW_NOT_THROWABLE       151
#define  ERR_CATCH_NOT_THROWABLE       152
#define  ERR_INITIALIZER_EXCEPTION     153
#define  ERR_CANT_READ                 154
#define  ERR_CANT_WRITE                155
#define  ERR_FATAL_ERROR               156 
#define  ERR_FATAL_EXCEPTION           157
#define  ERR_UNCAUGHT_EXCEPTION        158
#define  ERR_CATCH_NOT_THROWN          159
#define  ERR_INTERNAL                  160        

class NodeErrors {

private:
  int line;
  char *message;
  NodeErrors *leftnode, *rightnode;

public:
  NodeErrors(int, char*);
  ~NodeErrors();

  NodeErrors *GetLeftNode();
  void SetLeftNode(NodeErrors *);

  NodeErrors *GetRightNode();
  void SetRightNode(NodeErrors *);

  int GetLine();
  void Print();
};

class ListErrors {
private:
  NodeErrors *root;
  int count_errors;

public:
  ListErrors(NodeErrors *);
  ListErrors();
  ~ListErrors();
  NodeErrors *GetRoot();
  int GetNumErrors();
  void InsertMsg(int,char*);
  void Print();
};

#endif

class String;
class STNode;
class TreeNode;
class Descriptor;

typedef union {
  String *str;
  int    valint;
  float  valfloat;
  char   ascii;
  TreeNode *tree;
  Descriptor *descriptor;
  STNode *sym;
 } YYSTYPE;

#define	ABSTRACT	258
#define	BOOLEAN	259
#define	BREAK	260
#define	BYTE	261
#define	BYVALUE	262
#define	CASE	263
#define	CAST	264
#define	CATCH	265
#define	CHAR	266
#define	CLASS	267
#define	CONST	268
#define	CONTINUE	269
#define	DEFAULT	270
#define	DO	271
#define	DOUBLE	272
#define	ELSE	273
#define	EXTENDS	274
#define	FINAL	275
#define	FINALLY	276
#define	FLOAT	277
#define	FOR	278
#define	FUTURE	279
#define	GENERIC	280
#define	GOTO	281
#define	IF	282
#define	IMPLEMENTS	283
#define	IMPORT	284
#define	INNER	285
#define	INT	286
#define	INTERFACE	287
#define	INSTANCEOF	288
#define	LONG	289
#define	NATIVE	290
#define	NEW	291
#define	NULLTOKEN	292
#define	OPERATOR	293
#define	OUTER	294
#define	PACKAGE	295
#define	PRIVATE	296
#define	PROTECTED	297
#define	PUBLIC	298
#define	REST	299
#define	RETURN	300
#define	SHORT	301
#define	STATIC	302
#define	SUPER	303
#define	SWITCH	304
#define	SYNCHRONIZED	305
#define	THIS	306
#define	THROW	307
#define	THROWS	308
#define	TRANSIENT	309
#define	TRY	310
#define	VAR	311
#define	VOID	312
#define	VOLATILE	313
#define	WHILE	314
#define	IDEN	315
#define	BLITERAL	316
#define	ILITERAL	317
#define	CLITERAL	318
#define	FLITERAL	319
#define	SLITERAL	320
#define	EQUAL	321
#define	ASSIGNOP	322
#define	OROR	323
#define	ANDAND	324
#define	EQUOP	325
#define	RELOP	326
#define	SHIFTOP	327
#define	DIVOP	328
#define	INCOP	329
#define	UNOP	330


extern YYSTYPE yylval;

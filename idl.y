%error-verbose
%{
#include <stdio.h>
#include "idl.h"

static int enum_current_val = 1;
extern int yylineno;

static int iCaseValues[256];
static int iNumCaseValue = 0;
%}

%union {
  char s[256];
  int i;
  struct {
    IDL_ParamDir dir;
    char iid[256];
    char size_is[256];
    int size_ptrlevel;
  } param_attrib;
}

%token KEYWORD_INTERFACE
%token KEYWORD_UUID
%token KEYWORD_OBJECT
%token KEYWORD_HELPSTRING
%token KEYWORD_NONCREATABLE
%token KEYWORD_POINTERDEFAULT
%token KEYWORD_IN
%token KEYWORD_OUT
%token KEYWORD_RETVAL
%token KEYWORD_TYPEDEF
%token KEYWORD_PUBLIC
%token KEYWORD_STRUCT
%token KEYWORD_UNION
%token KEYWORD_ENUM
%token KEYWORD_V1ENUM
%token KEYWORD_SIZEIS
%token KEYWORD_IIDIS
%token KEYWORD_PROPGET
%token KEYWORD_PROPSET
%token KEYWORD_PROPPUT
%token KEYWORD_CASE
%token KEYWORD_COCLASS
%token KEYWORD_DEFAULT
%token KEYWORD_SOURCE
%token KEYWORD_SWITCHTYPE
%token KEYWORD_SWITCHIS
%token KEYWORD_HRESULT
%token KEYWORD_IMPORT
%token KEYWORD_IMPORTLIB
%token KEYWORD_INCLUDE
%token KEYWORD_LIBRARY
%token KEYWORD_VERSION

%token <s> TYPE_INT
%token <s> TYPE_BSTR
%token <s> TYPE_LONG
%token <s> TYPE_HYPER
%token <s> TYPE_UNSIGNED
%token <s> TYPE_SIGNED
%token <s> TYPE_REFIID
%token <s> TYPE_SHORT
%token <s> TYPE_VOID
%token <s> TYPE_CHAR
%token <s> TYPE_BOOLEAN
%token <s> TYPE_FLOAT
%token <s> TYPE_DOUBLE

%token <i> NUMBER
%token <i> HEXNUMBER
%token STRING
%token <s> UUID
%token <s> IDENTIFIER

%token LEFT_PARENTHESIS
%token RIGHT_PARENTHESIS
%token LEFT_SQUARE_BRACKET
%token RIGHT_SQUARE_BRACKET
%token LEFT_BRACKET
%token RIGHT_BRACKET
%token STAR
%token PLUS
%token SLASH
%token MINUS
%token COMMA
%token EQUAL
%token SEMICOLON
%token COLON
%token POINT

%token UNKNOWN

%type <i> ptrdef
%type <i> star
%type <i> arraysize
%type <s> datatype
%type <s> structmembermodifiers
%type <s> structmembermodifier
%type <s> structmembermodifier_int
%type <s> optional_identifier
%type <s> header
%type <s> header_body
%type <s> header_item
%type <param_attrib> interface_funcparam_modifier
%type <param_attrib> interface_funcparam_modifiers

%%

/* whole idl */
idl_whole: idl_body
	| header librarydef
	| importidl header librarydef
	;

librarydef:	KEYWORD_LIBRARY IDENTIFIER LEFT_BRACKET idl_body RIGHT_BRACKET SEMICOLON
	;

idl_body:	idlsection
	| idlsection idl_body
	;

/* one section in idl (struct, enum, class, etc.) */
idlsection: importlibs
	| structforwarddecl
	| structtypedef
	| structdef
	| enumdef
	| uniondef
	| interfacedef
	| coclassdef
	;

importidl: importidl_def
	| importidl_def importidl
	;

importidl_def: KEYWORD_IMPORT STRING SEMICOLON
	| KEYWORD_INCLUDE STRING
	; /* ignore for now */

importlibs: KEYWORD_IMPORTLIB LEFT_PARENTHESIS STRING RIGHT_PARENTHESIS SEMICOLON
	; /* ignore for now */

/* ====================================================================================== */

/* forward declartion of struct */
structforwarddecl: KEYWORD_STRUCT IDENTIFIER SEMICOLON
	{
	  IDL_AddForwardDecl($2);
	}
	;

/* typedef of struct */
structtypedef: KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC RIGHT_SQUARE_BRACKET KEYWORD_STRUCT IDENTIFIER IDENTIFIER SEMICOLON
	{
	  IDL_AddStructTypeDef($6, $7);
	}
	;

/* struct */
structdef: KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC RIGHT_SQUARE_BRACKET KEYWORD_STRUCT optional_identifier LEFT_BRACKET structbody RIGHT_BRACKET IDENTIFIER SEMICOLON
	{
	  IDL_AddStructDef($6, $10, "");
	}
	| KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC RIGHT_SQUARE_BRACKET KEYWORD_STRUCT optional_identifier LEFT_BRACKET structbody RIGHT_BRACKET IDENTIFIER COMMA STAR IDENTIFIER SEMICOLON
	{
	  IDL_AddStructDef($6, $10, $13);
	}
	;

/* body of a struct. */
structbody:
	| structmember structbody
	;

/* member defination in a struct. */
structmember: structmembermodifiers datatype ptrdef IDENTIFIER arraysize SEMICOLON
	{
	  if ($5 == -1)
	    {
	      IDL_AddStructMember($4, $2, $1, $3 + 1, 0);
	    }
	  else
	    {
	      IDL_AddStructMember($4, $2, $1, $3, $5);
	    }
	}
	;

structmembermodifiers:                                                   { $$[0] = '\0'; }
	| LEFT_SQUARE_BRACKET structmembermodifier RIGHT_SQUARE_BRACKET  { strcpy($$, $2); }
	;

structmembermodifier: structmembermodifier_int                           { if ($1[0]) strcpy($$, $1); }
	| COMMA structmembermodifier structmembermodifier_int            { if ($2[0]) strcpy($$, $2); }
	;

/* switch_is or size_is */
structmembermodifier_int: KEYWORD_SWITCHIS LEFT_PARENTHESIS IDENTIFIER RIGHT_PARENTHESIS   { $$[0] = '\0'; }
	| KEYWORD_SIZEIS LEFT_PARENTHESIS IDENTIFIER RIGHT_PARENTHESIS                     { strcpy($$, $3); }
	;

/* ====================================================================================== */

/* enum */
enumdef:
	KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC COMMA KEYWORD_V1ENUM RIGHT_SQUARE_BRACKET KEYWORD_ENUM optional_identifier LEFT_BRACKET enumbody RIGHT_BRACKET IDENTIFIER SEMICOLON
	{
	  IDL_AddEnumDef($8, $12);
	  enum_current_val = 1;
	}
	| KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC RIGHT_SQUARE_BRACKET KEYWORD_ENUM optional_identifier LEFT_BRACKET enumbody RIGHT_BRACKET IDENTIFIER SEMICOLON
	{
	  IDL_AddEnumDef($6, $10);
	  enum_current_val = 1;
	}
	;

enumbody:
	enummember
	| enummember COMMA
	| enummember COMMA enumbody
	;

enummember:
	IDENTIFIER
	{
	  IDL_AddEnumValue($1, enum_current_val++);
	}
	| IDENTIFIER EQUAL NUMBER
	{
	  enum_current_val = $3;
	  IDL_AddEnumValue($1, enum_current_val++);
	}
	| IDENTIFIER EQUAL HEXNUMBER
	{
	  enum_current_val = $3;
	  IDL_AddEnumValue($1, enum_current_val++);
	}
	| IDENTIFIER EQUAL IDENTIFIER
	{
	  enum_current_val = IDL_AddEnumRef($1, $3);
	}
	;

/* ====================================================================================== */

/* union */
uniondef: KEYWORD_TYPEDEF LEFT_SQUARE_BRACKET KEYWORD_PUBLIC RIGHT_SQUARE_BRACKET LEFT_SQUARE_BRACKET KEYWORD_SWITCHTYPE LEFT_PARENTHESIS datatype RIGHT_PARENTHESIS RIGHT_SQUARE_BRACKET KEYWORD_UNION IDENTIFIER LEFT_BRACKET unionbody RIGHT_BRACKET IDENTIFIER SEMICOLON
	{
	  IDL_AddUnionDef($12, $16);
	}
	;

unionbody: unionmember
	| unionmember unionbody;

unionmember: unionmodifier datatype ptrdef IDENTIFIER arraysize SEMICOLON
	{
	  if ($5 == -1)
	    {
	      IDL_AddUnionMember($4, $2, ""/*TODO: is size_is necessary?*/, $3 + 1, 0, iCaseValues, iNumCaseValue);
	      iNumCaseValue = 0;
	    }
	  else
	    {
	      IDL_AddUnionMember($4, $2, ""/*TODO: is size_is necessary?*/, $3, $5, iCaseValues, iNumCaseValue);
	      iNumCaseValue = 0;
	    }
	}
	| unionmodifier SEMICOLON { iNumCaseValue = 0; }
	;

unionmodifier:                { iNumCaseValue = 0; }
	| LEFT_SQUARE_BRACKET KEYWORD_CASE LEFT_PARENTHESIS unionmodifier_casevalues RIGHT_PARENTHESIS RIGHT_SQUARE_BRACKET
	| LEFT_SQUARE_BRACKET KEYWORD_DEFAULT RIGHT_SQUARE_BRACKET { iNumCaseValue = 0; }
	;

unionmodifier_casevalues:	unionmodifier_casevalue
	| unionmodifier_casevalue COMMA unionmodifier_casevalues
	;

unionmodifier_casevalue:	IDENTIFIER	{ iCaseValues[iNumCaseValue++] = IDL_GetEnumValue($1); }
	| NUMBER		{ iCaseValues[iNumCaseValue++] = $1; }
	| HEXNUMBER		{ iCaseValues[iNumCaseValue++] = $1; }
	;

/* ====================================================================================== */

/* interface */

interfacedef: header KEYWORD_INTERFACE IDENTIFIER COLON IDENTIFIER LEFT_BRACKET interface_body RIGHT_BRACKET optional_semicolon
	{
	  IDL_AddInterfaceDef($3, $1);
	}
	;

interface_body: interface_funcdef
	| interface_funcdef interface_body
	;

interface_funcdef: KEYWORD_HRESULT IDENTIFIER LEFT_PARENTHESIS interface_funcparamlist RIGHT_PARENTHESIS SEMICOLON
	{
	  IDL_AddInterfaceMethodDef($2);
	}
	| LEFT_SQUARE_BRACKET KEYWORD_PROPGET RIGHT_SQUARE_BRACKET KEYWORD_HRESULT IDENTIFIER LEFT_PARENTHESIS interface_funcparamlist RIGHT_PARENTHESIS SEMICOLON
	{
	  char buf[256];
	  sprintf(buf, "get_%s", $5);
	  IDL_AddInterfaceMethodDef(buf);
	}
	| LEFT_SQUARE_BRACKET KEYWORD_PROPSET RIGHT_SQUARE_BRACKET KEYWORD_HRESULT IDENTIFIER LEFT_PARENTHESIS interface_funcparamlist RIGHT_PARENTHESIS SEMICOLON
	{
	  char buf[256];
	  sprintf(buf, "set_%s", $5);
	  IDL_AddInterfaceMethodDef(buf);
	}
	| LEFT_SQUARE_BRACKET KEYWORD_PROPPUT RIGHT_SQUARE_BRACKET KEYWORD_HRESULT IDENTIFIER LEFT_PARENTHESIS interface_funcparamlist RIGHT_PARENTHESIS SEMICOLON
	{
	  char buf[256];
	  sprintf(buf, "put_%s", $5);
	  IDL_AddInterfaceMethodDef(buf);
	}
	;

interface_funcparamlist: 
	| interface_funcparamlist_internal
	;

interface_funcparamlist_internal: interface_funcparam
	| interface_funcparam COMMA interface_funcparamlist
	;

interface_funcparam: LEFT_SQUARE_BRACKET interface_funcparam_modifiers RIGHT_SQUARE_BRACKET datatype ptrdef IDENTIFIER arraysize
	{
	  if ($7 == -1)
	    {
	      IDL_AddInterfaceMethodParam($6, $4, $2.size_is, $2.size_ptrlevel, $2.iid, $5 + 1, 0, $2.dir);
	    }
	  else
	    {
	      IDL_AddInterfaceMethodParam($6, $4, $2.size_is, $2.size_ptrlevel, $2.iid, $5, $7, $2.dir);
	    }
	}
	;

interface_funcparam_modifiers: interface_funcparam_modifier
	{
	  memset(&$$, 0, sizeof($$));

	  if ((int)$1.dir != 0) $$.dir = $1.dir;
	  if ($1.iid[0] != 0) strcpy($$.iid, $1.iid);
	  if ($1.size_is[0] != 0) strcpy($$.size_is, $1.size_is);
	  if ($1.size_ptrlevel != 0) $$.size_ptrlevel = $1.size_ptrlevel;
	}
	| interface_funcparam_modifier COMMA interface_funcparam_modifiers
	{
	  memset(&$$, 0, sizeof($$));

	  if ((int)$1.dir != 0) $$.dir = $1.dir;
	  if ($1.iid[0] != 0) strcpy($$.iid, $1.iid);
	  if ($1.size_is[0] != 0) strcpy($$.size_is, $1.size_is);
	  if ($1.size_ptrlevel != 0) $$.size_ptrlevel = $1.size_ptrlevel;

	  if ((int)$3.dir != 0) $$.dir = $3.dir;
	  if ($3.iid[0] != 0) strcpy($$.iid, $3.iid);
	  if ($3.size_is[0] != 0) strcpy($$.size_is, $3.size_is);
	  if ($3.size_ptrlevel != 0) $$.size_ptrlevel = $3.size_ptrlevel;
	}
	;

interface_funcparam_modifier: KEYWORD_IN                { memset(&$$, 0, sizeof($$)); $$.dir = IN; }
	| KEYWORD_OUT                                   { memset(&$$, 0, sizeof($$)); $$.dir = OUT; }
	| KEYWORD_SIZEIS LEFT_PARENTHESIS ptrdef IDENTIFIER RIGHT_PARENTHESIS           { memset(&$$, 0, sizeof($$)); $$.size_ptrlevel = $3; strcpy($$.size_is, $4); }
	| KEYWORD_SIZEIS LEFT_PARENTHESIS COMMA ptrdef IDENTIFIER RIGHT_PARENTHESIS     { memset(&$$, 0, sizeof($$)); $$.size_ptrlevel = $4; strcpy($$.size_is, $5); }
	| KEYWORD_IIDIS LEFT_PARENTHESIS IDENTIFIER RIGHT_PARENTHESIS                   { memset(&$$, 0, sizeof($$)); strcpy($$.iid, $3); }
	| KEYWORD_RETVAL				{ memset (&$$, 0, sizeof($$)); /* ignore for now */ }
	;

/* ====================================================================================== */

/* coclass */

coclassdef: header KEYWORD_COCLASS IDENTIFIER LEFT_BRACKET coclass_body RIGHT_BRACKET SEMICOLON
	{
		IDL_AddCoClass($3, $1);
	}
	;

coclass_body: coclass_interface
	| coclass_interface coclass_body
	;

coclass_interface: coclass_modifier KEYWORD_INTERFACE IDENTIFIER SEMICOLON
	;

coclass_modifier:
	| LEFT_SQUARE_BRACKET coclass_modifiers RIGHT_SQUARE_BRACKET
	;

coclass_modifiers: coclass_modifier_word
	| coclass_modifier_word COMMA coclass_modifiers
	;

coclass_modifier_word: KEYWORD_DEFAULT
	| KEYWORD_SOURCE
	;

/* ====================================================================================== */

/* misc */

ptrdef:              { $$ = 0; }
	| star           { $$ = $1; }
	;

star: STAR           { $$ = 1; }
	| star STAR      { $$++; }
	;

datatype: IDENTIFIER                          { strcpy($$, $1); }
	| KEYWORD_STRUCT IDENTIFIER	      { strcpy($$, "struct "); strcat($$, $2); }
	| KEYWORD_UNION IDENTIFIER	      { strcpy($$, "union "); strcat($$, $2); }
	| KEYWORD_ENUM IDENTIFIER	      { strcpy($$, "enum "); strcat($$, $2); }
	| TYPE_INT                                { strcpy($$, $1); }
	| TYPE_BSTR                               { strcpy($$, $1); }
	| TYPE_LONG                               { strcpy($$, $1); }
	| TYPE_HYPER                              { strcpy($$, $1); }
	| TYPE_UNSIGNED                           { strcpy($$, $1); }
	| TYPE_REFIID                             { strcpy($$, $1); }
	| TYPE_SHORT                              { strcpy($$, $1); }
	| TYPE_VOID                               { strcpy($$, $1); }
	| TYPE_CHAR                               { strcpy($$, $1); }
	| TYPE_BOOLEAN                            { strcpy($$, $1); }
	| TYPE_FLOAT                              { strcpy($$, $1); }
	| TYPE_DOUBLE                             { strcpy($$, $1); }
	| TYPE_UNSIGNED TYPE_INT                  { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_LONG                 { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_HYPER                { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_SHORT                { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_LONG TYPE_INT        { sprintf($$, "%s %s %s", $1, $2, $3); }
	| TYPE_UNSIGNED TYPE_SHORT TYPE_INT       { sprintf($$, "%s %s %s", $1, $2, $3); }
	| TYPE_UNSIGNED TYPE_CHAR                 { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_FLOAT                { sprintf($$, "%s %s", $1, $2); }
	| TYPE_UNSIGNED TYPE_DOUBLE               { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_INT                    { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_LONG                   { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_SHORT                  { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_LONG TYPE_INT          { sprintf($$, "%s %s %s", $1, $2, $3); }
	| TYPE_SIGNED TYPE_SHORT TYPE_INT         { sprintf($$, "%s %s %s", $1, $2, $3); }
	| TYPE_SIGNED TYPE_CHAR                   { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_FLOAT                  { sprintf($$, "%s %s", $1, $2); }
	| TYPE_SIGNED TYPE_DOUBLE                 { sprintf($$, "%s %s", $1, $2); }
	;

header: LEFT_SQUARE_BRACKET header_body RIGHT_SQUARE_BRACKET           { strcpy($$, $2); }
	;

header_body: header_item                                               { strcpy($$, $1); }
	| header_item COMMA header_body                                { strcpy($$, $1); if ($3[0]) strcpy($$, $3); }
	;

/* only need to care about the UUID */
header_item: KEYWORD_UUID LEFT_PARENTHESIS UUID RIGHT_PARENTHESIS      { strcpy($$, $3); }
	| KEYWORD_HELPSTRING LEFT_PARENTHESIS STRING RIGHT_PARENTHESIS { $$[0] = '\0'; }
	| KEYWORD_VERSION LEFT_PARENTHESIS NUMBER POINT NUMBER RIGHT_PARENTHESIS { $$[0] = '\0'; }
	| KEYWORD_OBJECT                                               { $$[0] = '\0'; }
	| KEYWORD_POINTERDEFAULT LEFT_PARENTHESIS IDENTIFIER RIGHT_PARENTHESIS  { $$[0] = '\0'; }
	| KEYWORD_NONCREATABLE				               { $$[0] = '\0'; }
	;

arraysize:                                                      { $$ = 0; }
	| LEFT_SQUARE_BRACKET IDENTIFIER RIGHT_SQUARE_BRACKET       { $$ = IDL_GetEnumValue($2); }
	| LEFT_SQUARE_BRACKET NUMBER RIGHT_SQUARE_BRACKET           { $$ = $2; }
	| LEFT_SQUARE_BRACKET HEXNUMBER RIGHT_SQUARE_BRACKET        { $$ = $2; }
	| LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET		{ $$ = -1; /* indicate ptr */ }
	;

optional_identifier:               { $$[0] = '\0'; }
	| IDENTIFIER               { strcpy($$, $1); }
	;

optional_semicolon:
	| SEMICOLON
	;

%%

int
yyerror(char *s)
{
  fprintf(stderr, "error (%d): %s\n", yylineno, s);
  exit(255);
}

%option noyywrap
%option nounistd
%option yylineno

%x COMMENT
%x IFZERO

%{
#include "idl.h"
#include "idl.tab.h"
#include <ctype.h>

char* stoupper( char* s )
{
  char* p = s;
  while (*p = toupper(*p)) p++;
  return s;
}
%}

%%

"//".*		{ /* comment, do nothing */ }

"/*"			{ BEGIN(COMMENT); }
<COMMENT>([^*]|\n)+|.	{ /* comment, do nothing */ }
<COMMENT>"*/"		{ BEGIN(INITIAL); }

"#if"[ \t]+"0"[ \t]+.*\n	{ BEGIN(IFZERO); }
<IFZERO>([^#]|\n)+|.		{ /* comment, do nothing */ }
<IFZERO>"#endif".*\n		{ BEGIN(INITIAL); }

"interface"	{ return KEYWORD_INTERFACE; }
"object"	{ return KEYWORD_OBJECT; }
"uuid"		{ return KEYWORD_UUID; }
"helpstring"	{ return KEYWORD_HELPSTRING; }
"noncreatable"  { return KEYWORD_NONCREATABLE; }
"pointer_default"	{ return KEYWORD_POINTERDEFAULT; }
"in"		{ return KEYWORD_IN; }
"out"		{ return KEYWORD_OUT; }
"retval"	{ return KEYWORD_RETVAL; }
"typedef"	{ return KEYWORD_TYPEDEF; }
"public"	{ return KEYWORD_PUBLIC; }
"struct"	{ return KEYWORD_STRUCT; }
"union"		{ return KEYWORD_UNION; }
"enum"		{ return KEYWORD_ENUM; }
"v1_enum"	{ return KEYWORD_V1ENUM; }
"size_is"	{ return KEYWORD_SIZEIS; }
"iid_is"	{ return KEYWORD_IIDIS; }
"propget"	{ return KEYWORD_PROPGET; }
"propset"	{ return KEYWORD_PROPSET; }
"propput"	{ return KEYWORD_PROPPUT; }
"case"		{ return KEYWORD_CASE; }
"coclass"	{ return KEYWORD_COCLASS; }
"default"	{ return KEYWORD_DEFAULT; }
"source"	{ return KEYWORD_SOURCE; }
"switch_type"	{ return KEYWORD_SWITCHTYPE; }
"switch_is"	{ return KEYWORD_SWITCHIS; }
"HRESULT"	{ return KEYWORD_HRESULT; }
"import"	{ return KEYWORD_IMPORT; }
"importlib"	{ return KEYWORD_IMPORTLIB; }
"library"	{ return KEYWORD_LIBRARY; }
"version"	{ return KEYWORD_VERSION; }
"#include"	{ return KEYWORD_INCLUDE; }

"int"		{ strcpy(yylval.s, yytext); return TYPE_INT; }
"long"		{ strcpy(yylval.s, yytext); return TYPE_LONG; }
"hyper"		{ strcpy(yylval.s, yytext); return TYPE_HYPER; }
"BSTR"		{ strcpy(yylval.s, yytext); return TYPE_BSTR; }
"unsigned"	{ strcpy(yylval.s, yytext); return TYPE_UNSIGNED; }
"signed"	{ strcpy(yylval.s, yytext); return TYPE_SIGNED; }
"REFIID"	{ strcpy(yylval.s, yytext); return TYPE_REFIID; }
"short"		{ strcpy(yylval.s, yytext); return TYPE_SHORT; }
"void"		{ strcpy(yylval.s, yytext); return TYPE_VOID; }
"char"		{ strcpy(yylval.s, yytext); return TYPE_CHAR; }
"boolean"	{ strcpy(yylval.s, yytext); return TYPE_BOOLEAN; }
"float"		{ strcpy(yylval.s, yytext); return TYPE_FLOAT; }
"double"	{ strcpy(yylval.s, yytext); return TYPE_DOUBLE; }

"(" 		{ return LEFT_PARENTHESIS; }
")" 		{ return RIGHT_PARENTHESIS; }
"[" 		{ return LEFT_SQUARE_BRACKET; }
"]" 		{ return RIGHT_SQUARE_BRACKET; }
"{" 		{ return LEFT_BRACKET; }
"}" 		{ return RIGHT_BRACKET; }
\".*\"		{ return STRING; }
"*"		{ return STAR; }
"+"		{ return PLUS; }
"-"		{ return MINUS; }
"/"		{ return SLASH; }
","		{ return COMMA; }
"="		{ return EQUAL; }
";"		{ return SEMICOLON; }
":"		{ return COLON; }
"\."		{ return POINT; }
[0-9]+		{ yylval.i = atoi(yytext); return NUMBER; }
\'.\'		{ yylval.i = (int)yytext[1]; return NUMBER; }
0[xX][0-9A-Fa-f]+	{ sscanf(yytext + 2, "%x", &yylval.i); return HEXNUMBER; }
[A-Fa-f0-9]{8}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{4}-[A-Fa-f0-9]{12}	{ strcpy(yylval.s, yytext); stoupper(yylval.s); return UUID; }
[A-Za-z_][A-Za-z0-9_]*	{ strcpy(yylval.s, yytext); return IDENTIFIER; }

[ \t\n\r]+	{ /* whitespaces, do nothing */ }

.		{ printf("UNKNOWN: %s\n", yytext); return UNKNOWN; }

%%

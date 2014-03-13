#include "idl.h"
#include <stdio.h>

extern "C" int yyparse();
extern FILE *yyin;

int main(int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf(stderr, "Usage: %s <input.idl> <output.h>\n", argv[0]);
      return 255;
    }

  yyin = fopen(argv[1], "r");
  if (yyin == NULL)
    {
      fprintf(stderr, "ERROR: cannot open %s\n", argv[1]);
      return 255;
    }

  printf("IDL: %s -> %s...\n", argv[1], argv[2]);
  yyparse();

  IDL_DumpToCHeader(argv[2]);

  fclose(yyin);
  yyin = stdin;

  return 0;
}


%option noyywrap
%{
#include <stdio.h>
#include <string.h>
#include "parse.h"
#include "instru_parse.h"
#include "misc.h"

extern int line;

int
instru_parse(instru_t *i, char *file_name)
{
    FILE *file;
    ck_err(instru);
    //   ck_err(!(file = fopen(file_name, "r")));
    ck_err(!(file = fopen(file_name, "r")));
    instru = i;
    yyrestart(file);
    yyparse();
    fclose(file);
    instru = 0;
    return 0;
  error:
    return -1;
}

void yyerror(const char *str)
{
    fprintf(stderr, "PARSE ERROR: line: %d: %s\n", line, str);
    abort();
}

%}

%%
instru			return INSTRU_TOKEN;
buffer			return BUFFER_TOKEN;
value			return VALUE_TOKEN;
voice			return VOICE_TOKEN;
data			return DATA_TOKEN;
in			return IN_TOKEN;
out			return OUT_TOKEN;
local			return LOCAL_TOKEN;
global			return GLOBAL_TOKEN;

[#_a-zA-Z0-9\.]*	yylval.string=strdup(yytext); return WORD_TOKEN;

\n			line++;
[\;,:.\{\}\(\)=]	return *yytext;
[ \t]+                  /* ignore whitespace */;
%%

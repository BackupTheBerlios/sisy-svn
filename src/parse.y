%union{
    char *string;
    int integer;
}

%token INSTRU_TOKEN VOICE_TOKEN DATA_TOKEN VALUE_TOKEN BUFFER_TOKEN
%token IN_TOKEN OUT_TOKEN LOCAL_TOKEN GLOBAL_TOKEN INT_TOKEN FLOAT_TOKEN
%token <string> WORD_TOKEN

%start instru

%type <integer> scope type
%type <string>  value

%{
#include "instru_parse.h"
#include "bank.h"
%}
%%

value: WORD_TOKEN {$$=$1;};/* | INT_TOKEN | FLOAT_TOKEN; */
key_peer: WORD_TOKEN ':' value {key_peer($1, $3);};
list_param: | key_peer | list_param ',' key_peer;
module_begin: WORD_TOKEN '(' {module_begin($1);};
module_end: ')' {module_end();};
module: module_begin list_param module_end;

scope: /* default */ {$$=SIAD_SCOPE_LOCAL;}
|      IN_TOKEN {$$=SIAD_SCOPE_IN;}
|      OUT_TOKEN {$$=SIAD_SCOPE_OUT;}
|      LOCAL_TOKEN {$$=SIAD_SCOPE_LOCAL;}
|      GLOBAL_TOKEN {$$=SIAD_SCOPE_GLOBAL;};

type: VALUE_TOKEN {$$=SIAD_TYPE_VALUE;}
|     BUFFER_TOKEN {$$=SIAD_TYPE_BUFFER;};

data_statement:	scope type WORD_TOKEN ';' {declare($1, $2, $3, 0);}
| scope type WORD_TOKEN '=' WORD_TOKEN ';' {declare($1, $2, $3, $5);};
data_statement_list: | data_statement_list data_statement;
data_begin: DATA_TOKEN {data_begin();};
data_section: | data_begin '{' data_statement_list '}' {data_end();};

process_statement: module ';';
process: | process process_statement;
process_section: '{' data_section process '}';

voice_begin: VOICE_TOKEN {voice_begin();};
voice_process: voice_begin process_section {voice_end();};

global_begin: GLOBAL_TOKEN {global_begin();};
global_process: global_begin process_section {global_end();};

instru_statement: data_section | voice_process | global_process;
instru_statement_list : | instru_statement_list instru_statement;
instru_begin: INSTRU_TOKEN WORD_TOKEN {instru_begin($2);};
instru_section: instru_begin '{' data_section instru_statement_list '}';

instru: | instru instru_section {instru_end();};
%%

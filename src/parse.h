/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INSTRU_TOKEN = 258,
     VOICE_TOKEN = 259,
     DATA_TOKEN = 260,
     VALUE_TOKEN = 261,
     BUFFER_TOKEN = 262,
     IN_TOKEN = 263,
     OUT_TOKEN = 264,
     LOCAL_TOKEN = 265,
     GLOBAL_TOKEN = 266,
     INT_TOKEN = 267,
     FLOAT_TOKEN = 268,
     WORD_TOKEN = 269
   };
#endif
#define INSTRU_TOKEN 258
#define VOICE_TOKEN 259
#define DATA_TOKEN 260
#define VALUE_TOKEN 261
#define BUFFER_TOKEN 262
#define IN_TOKEN 263
#define OUT_TOKEN 264
#define LOCAL_TOKEN 265
#define GLOBAL_TOKEN 266
#define INT_TOKEN 267
#define FLOAT_TOKEN 268
#define WORD_TOKEN 269




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 1 "parse.y"
typedef union YYSTYPE {
    char *string;
    int integer;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 70 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;




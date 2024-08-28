/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_MISC_PARSER_HPP_INCLUDED
# define YY_YY_MISC_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EOL = 258,
    COMMA = 259,
    LPAR = 260,
    RPAR = 261,
    EQ = 262,
    IMM = 263,
    PER = 264,
    TO = 265,
    PLUS = 266,
    HALT_T = 267,
    INT_T = 268,
    IRET_T = 269,
    CALL_T = 270,
    RET_T = 271,
    JMP_T = 272,
    BEQ_T = 273,
    BNE_T = 274,
    BGT_T = 275,
    PUSH_T = 276,
    POP_T = 277,
    XCHG_T = 278,
    ADD_T = 279,
    SUB_T = 280,
    MUL_T = 281,
    DIV_T = 282,
    NOT_T = 283,
    AND_T = 284,
    OR_T = 285,
    XOR_T = 286,
    SHL_T = 287,
    SHR_T = 288,
    LD_T = 289,
    ST_T = 290,
    CSRRD_T = 291,
    CSRWR_T = 292,
    DIR = 293,
    IDENT = 294,
    IMMOPR = 295,
    REG = 296,
    CSR = 297
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 13 "./misc/parser.y"

  struct Argument* a;

#line 104 "./misc/parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_MISC_PARSER_HPP_INCLUDED  */

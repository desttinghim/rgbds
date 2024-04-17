/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_D7545668D604872D_PARSER_H_INCLUDED
# define YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_D7545668D604872D_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    T_EOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_NUMBER = 258,                /* "number"  */
    T_STRING = 259,                /* "string"  */
    T_PERIOD = 260,                /* "."  */
    T_COMMA = 261,                 /* ","  */
    T_COLON = 262,                 /* ":"  */
    T_LBRACK = 263,                /* "["  */
    T_RBRACK = 264,                /* "]"  */
    T_LPAREN = 265,                /* "("  */
    T_RPAREN = 266,                /* ")"  */
    T_NEWLINE = 267,               /* "newline"  */
    T_OP_LOGICNOT = 268,           /* "!"  */
    T_OP_LOGICAND = 269,           /* "&&"  */
    T_OP_LOGICOR = 270,            /* "||"  */
    T_OP_LOGICGT = 271,            /* ">"  */
    T_OP_LOGICLT = 272,            /* "<"  */
    T_OP_LOGICGE = 273,            /* ">="  */
    T_OP_LOGICLE = 274,            /* "<="  */
    T_OP_LOGICNE = 275,            /* "!="  */
    T_OP_LOGICEQU = 276,           /* "=="  */
    T_OP_ADD = 277,                /* "+"  */
    T_OP_SUB = 278,                /* "-"  */
    T_OP_OR = 279,                 /* "|"  */
    T_OP_XOR = 280,                /* "^"  */
    T_OP_AND = 281,                /* "&"  */
    T_OP_SHL = 282,                /* "<<"  */
    T_OP_SHR = 283,                /* ">>"  */
    T_OP_USHR = 284,               /* ">>>"  */
    T_OP_MUL = 285,                /* "*"  */
    T_OP_DIV = 286,                /* "/"  */
    T_OP_MOD = 287,                /* "%"  */
    T_OP_NOT = 288,                /* "~"  */
    NEG = 289,                     /* NEG  */
    T_OP_EXP = 290,                /* "**"  */
    T_OP_DEF = 291,                /* "DEF"  */
    T_OP_BANK = 292,               /* "BANK"  */
    T_OP_ALIGN = 293,              /* "ALIGN"  */
    T_OP_SIZEOF = 294,             /* "SIZEOF"  */
    T_OP_STARTOF = 295,            /* "STARTOF"  */
    T_OP_SIN = 296,                /* "SIN"  */
    T_OP_COS = 297,                /* "COS"  */
    T_OP_TAN = 298,                /* "TAN"  */
    T_OP_ASIN = 299,               /* "ASIN"  */
    T_OP_ACOS = 300,               /* "ACOS"  */
    T_OP_ATAN = 301,               /* "ATAN"  */
    T_OP_ATAN2 = 302,              /* "ATAN2"  */
    T_OP_FDIV = 303,               /* "FDIV"  */
    T_OP_FMUL = 304,               /* "FMUL"  */
    T_OP_FMOD = 305,               /* "FMOD"  */
    T_OP_POW = 306,                /* "POW"  */
    T_OP_LOG = 307,                /* "LOG"  */
    T_OP_ROUND = 308,              /* "ROUND"  */
    T_OP_CEIL = 309,               /* "CEIL"  */
    T_OP_FLOOR = 310,              /* "FLOOR"  */
    T_OP_HIGH = 311,               /* "HIGH"  */
    T_OP_LOW = 312,                /* "LOW"  */
    T_OP_ISCONST = 313,            /* "ISCONST"  */
    T_OP_STRCMP = 314,             /* "STRCMP"  */
    T_OP_STRIN = 315,              /* "STRIN"  */
    T_OP_STRRIN = 316,             /* "STRRIN"  */
    T_OP_STRSUB = 317,             /* "STRSUB"  */
    T_OP_STRLEN = 318,             /* "STRLEN"  */
    T_OP_STRCAT = 319,             /* "STRCAT"  */
    T_OP_STRUPR = 320,             /* "STRUPR"  */
    T_OP_STRLWR = 321,             /* "STRLWR"  */
    T_OP_STRRPL = 322,             /* "STRRPL"  */
    T_OP_STRFMT = 323,             /* "STRFMT"  */
    T_OP_CHARLEN = 324,            /* "CHARLEN"  */
    T_OP_CHARSUB = 325,            /* "CHARSUB"  */
    T_LABEL = 326,                 /* "label"  */
    T_ID = 327,                    /* "identifier"  */
    T_LOCAL_ID = 328,              /* "local identifier"  */
    T_ANON = 329,                  /* "anonymous label"  */
    T_POP_EQU = 330,               /* "EQU"  */
    T_POP_EQUAL = 331,             /* "="  */
    T_POP_EQUS = 332,              /* "EQUS"  */
    T_POP_ADDEQ = 333,             /* "+="  */
    T_POP_SUBEQ = 334,             /* "-="  */
    T_POP_MULEQ = 335,             /* "*="  */
    T_POP_DIVEQ = 336,             /* "/="  */
    T_POP_MODEQ = 337,             /* "%="  */
    T_POP_OREQ = 338,              /* "|="  */
    T_POP_XOREQ = 339,             /* "^="  */
    T_POP_ANDEQ = 340,             /* "&="  */
    T_POP_SHLEQ = 341,             /* "<<="  */
    T_POP_SHREQ = 342,             /* ">>="  */
    T_POP_INCLUDE = 343,           /* "INCLUDE"  */
    T_POP_PRINT = 344,             /* "PRINT"  */
    T_POP_PRINTLN = 345,           /* "PRINTLN"  */
    T_POP_IF = 346,                /* "IF"  */
    T_POP_ELIF = 347,              /* "ELIF"  */
    T_POP_ELSE = 348,              /* "ELSE"  */
    T_POP_ENDC = 349,              /* "ENDC"  */
    T_POP_EXPORT = 350,            /* "EXPORT"  */
    T_POP_DB = 351,                /* "DB"  */
    T_POP_DS = 352,                /* "DS"  */
    T_POP_DW = 353,                /* "DW"  */
    T_POP_DL = 354,                /* "DL"  */
    T_POP_SECTION = 355,           /* "SECTION"  */
    T_POP_FRAGMENT = 356,          /* "FRAGMENT"  */
    T_POP_RB = 357,                /* "RB"  */
    T_POP_RW = 358,                /* "RW"  */
    T_POP_MACRO = 359,             /* "MACRO"  */
    T_POP_ENDM = 360,              /* "ENDM"  */
    T_POP_RSRESET = 361,           /* "RSRESET"  */
    T_POP_RSSET = 362,             /* "RSSET"  */
    T_POP_UNION = 363,             /* "UNION"  */
    T_POP_NEXTU = 364,             /* "NEXTU"  */
    T_POP_ENDU = 365,              /* "ENDU"  */
    T_POP_INCBIN = 366,            /* "INCBIN"  */
    T_POP_REPT = 367,              /* "REPT"  */
    T_POP_FOR = 368,               /* "FOR"  */
    T_POP_CHARMAP = 369,           /* "CHARMAP"  */
    T_POP_NEWCHARMAP = 370,        /* "NEWCHARMAP"  */
    T_POP_SETCHARMAP = 371,        /* "SETCHARMAP"  */
    T_POP_PUSHC = 372,             /* "PUSHC"  */
    T_POP_POPC = 373,              /* "POPC"  */
    T_POP_SHIFT = 374,             /* "SHIFT"  */
    T_POP_ENDR = 375,              /* "ENDR"  */
    T_POP_BREAK = 376,             /* "BREAK"  */
    T_POP_LOAD = 377,              /* "LOAD"  */
    T_POP_ENDL = 378,              /* "ENDL"  */
    T_POP_FAIL = 379,              /* "FAIL"  */
    T_POP_WARN = 380,              /* "WARN"  */
    T_POP_FATAL = 381,             /* "FATAL"  */
    T_POP_ASSERT = 382,            /* "ASSERT"  */
    T_POP_STATIC_ASSERT = 383,     /* "STATIC_ASSERT"  */
    T_POP_PURGE = 384,             /* "PURGE"  */
    T_POP_REDEF = 385,             /* "REDEF"  */
    T_POP_POPS = 386,              /* "POPS"  */
    T_POP_PUSHS = 387,             /* "PUSHS"  */
    T_POP_POPO = 388,              /* "POPO"  */
    T_POP_PUSHO = 389,             /* "PUSHO"  */
    T_POP_OPT = 390,               /* "OPT"  */
    T_SECT_ROM0 = 391,             /* "ROM0"  */
    T_SECT_ROMX = 392,             /* "ROMX"  */
    T_SECT_WRAM0 = 393,            /* "WRAM0"  */
    T_SECT_WRAMX = 394,            /* "WRAMX"  */
    T_SECT_HRAM = 395,             /* "HRAM"  */
    T_SECT_VRAM = 396,             /* "VRAM"  */
    T_SECT_SRAM = 397,             /* "SRAM"  */
    T_SECT_OAM = 398,              /* "OAM"  */
    T_Z80_ADC = 399,               /* "adc"  */
    T_Z80_ADD = 400,               /* "add"  */
    T_Z80_AND = 401,               /* "and"  */
    T_Z80_BIT = 402,               /* "bit"  */
    T_Z80_CALL = 403,              /* "call"  */
    T_Z80_CCF = 404,               /* "ccf"  */
    T_Z80_CP = 405,                /* "cp"  */
    T_Z80_CPL = 406,               /* "cpl"  */
    T_Z80_DAA = 407,               /* "daa"  */
    T_Z80_DEC = 408,               /* "dec"  */
    T_Z80_DI = 409,                /* "di"  */
    T_Z80_EI = 410,                /* "ei"  */
    T_Z80_HALT = 411,              /* "halt"  */
    T_Z80_INC = 412,               /* "inc"  */
    T_Z80_JP = 413,                /* "jp"  */
    T_Z80_JR = 414,                /* "jr"  */
    T_Z80_LD = 415,                /* "ld"  */
    T_Z80_LDI = 416,               /* "ldi"  */
    T_Z80_LDD = 417,               /* "ldd"  */
    T_Z80_LDH = 418,               /* "ldh"  */
    T_Z80_NOP = 419,               /* "nop"  */
    T_Z80_OR = 420,                /* "or"  */
    T_Z80_POP = 421,               /* "pop"  */
    T_Z80_PUSH = 422,              /* "push"  */
    T_Z80_RES = 423,               /* "res"  */
    T_Z80_RET = 424,               /* "ret"  */
    T_Z80_RETI = 425,              /* "reti"  */
    T_Z80_RST = 426,               /* "rst"  */
    T_Z80_RL = 427,                /* "rl"  */
    T_Z80_RLA = 428,               /* "rla"  */
    T_Z80_RLC = 429,               /* "rlc"  */
    T_Z80_RLCA = 430,              /* "rlca"  */
    T_Z80_RR = 431,                /* "rr"  */
    T_Z80_RRA = 432,               /* "rra"  */
    T_Z80_RRC = 433,               /* "rrc"  */
    T_Z80_RRCA = 434,              /* "rrca"  */
    T_Z80_SBC = 435,               /* "sbc"  */
    T_Z80_SCF = 436,               /* "scf"  */
    T_Z80_SET = 437,               /* "set"  */
    T_Z80_STOP = 438,              /* "stop"  */
    T_Z80_SLA = 439,               /* "sla"  */
    T_Z80_SRA = 440,               /* "sra"  */
    T_Z80_SRL = 441,               /* "srl"  */
    T_Z80_SUB = 442,               /* "sub"  */
    T_Z80_SWAP = 443,              /* "swap"  */
    T_Z80_XOR = 444,               /* "xor"  */
    T_TOKEN_A = 445,               /* "a"  */
    T_TOKEN_B = 446,               /* "b"  */
    T_TOKEN_C = 447,               /* "c"  */
    T_TOKEN_D = 448,               /* "d"  */
    T_TOKEN_E = 449,               /* "e"  */
    T_TOKEN_H = 450,               /* "h"  */
    T_TOKEN_L = 451,               /* "l"  */
    T_MODE_AF = 452,               /* "af"  */
    T_MODE_BC = 453,               /* "bc"  */
    T_MODE_DE = 454,               /* "de"  */
    T_MODE_SP = 455,               /* "sp"  */
    T_MODE_HL = 456,               /* "hl"  */
    T_MODE_HL_DEC = 457,           /* "hld/hl-"  */
    T_MODE_HL_INC = 458,           /* "hli/hl+"  */
    T_CC_NZ = 459,                 /* "nz"  */
    T_CC_Z = 460,                  /* "z"  */
    T_CC_NC = 461,                 /* "nc"  */
    T_EOB = 462                    /* "end of buffer"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 473 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"

	char symName[MAXSYMLEN + 1];
	char string[MAXSTRLEN + 1];
	struct Expression expr;
	int32_t constValue;
	enum RPNCommand compoundEqual;
	enum SectionModifier sectMod;
	struct SectionSpec sectSpec;
	struct MacroArgs *macroArg;
	enum AssertionType assertType;
	struct DsArgList dsArgs;
	struct {
		int32_t start;
		int32_t stop;
		int32_t step;
	} forArgs;
	struct StrFmtArgList strfmtArgs;
	bool captureTerminated;

#line 291 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/d7545668d604872d/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_D7545668D604872D_PARSER_H_INCLUDED  */

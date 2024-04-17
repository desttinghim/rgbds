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

#ifndef YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_215B8502AE944A74_PARSER_HPP_INCLUDED
# define YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_215B8502AE944A74_PARSER_HPP_INCLUDED
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
    T_DOUBLE_COLON = 263,          /* "::"  */
    T_LBRACK = 264,                /* "["  */
    T_RBRACK = 265,                /* "]"  */
    T_LPAREN = 266,                /* "("  */
    T_RPAREN = 267,                /* ")"  */
    T_NEWLINE = 268,               /* "newline"  */
    T_OP_LOGICNOT = 269,           /* "!"  */
    T_OP_LOGICAND = 270,           /* "&&"  */
    T_OP_LOGICOR = 271,            /* "||"  */
    T_OP_LOGICGT = 272,            /* ">"  */
    T_OP_LOGICLT = 273,            /* "<"  */
    T_OP_LOGICGE = 274,            /* ">="  */
    T_OP_LOGICLE = 275,            /* "<="  */
    T_OP_LOGICNE = 276,            /* "!="  */
    T_OP_LOGICEQU = 277,           /* "=="  */
    T_OP_ADD = 278,                /* "+"  */
    T_OP_SUB = 279,                /* "-"  */
    T_OP_OR = 280,                 /* "|"  */
    T_OP_XOR = 281,                /* "^"  */
    T_OP_AND = 282,                /* "&"  */
    T_OP_SHL = 283,                /* "<<"  */
    T_OP_SHR = 284,                /* ">>"  */
    T_OP_USHR = 285,               /* ">>>"  */
    T_OP_MUL = 286,                /* "*"  */
    T_OP_DIV = 287,                /* "/"  */
    T_OP_MOD = 288,                /* "%"  */
    T_OP_NOT = 289,                /* "~"  */
    NEG = 290,                     /* NEG  */
    T_OP_EXP = 291,                /* "**"  */
    T_OP_DEF = 292,                /* "DEF"  */
    T_OP_BANK = 293,               /* "BANK"  */
    T_OP_ALIGN = 294,              /* "ALIGN"  */
    T_OP_SIZEOF = 295,             /* "SIZEOF"  */
    T_OP_STARTOF = 296,            /* "STARTOF"  */
    T_OP_SIN = 297,                /* "SIN"  */
    T_OP_COS = 298,                /* "COS"  */
    T_OP_TAN = 299,                /* "TAN"  */
    T_OP_ASIN = 300,               /* "ASIN"  */
    T_OP_ACOS = 301,               /* "ACOS"  */
    T_OP_ATAN = 302,               /* "ATAN"  */
    T_OP_ATAN2 = 303,              /* "ATAN2"  */
    T_OP_FDIV = 304,               /* "FDIV"  */
    T_OP_FMUL = 305,               /* "FMUL"  */
    T_OP_FMOD = 306,               /* "FMOD"  */
    T_OP_POW = 307,                /* "POW"  */
    T_OP_LOG = 308,                /* "LOG"  */
    T_OP_ROUND = 309,              /* "ROUND"  */
    T_OP_CEIL = 310,               /* "CEIL"  */
    T_OP_FLOOR = 311,              /* "FLOOR"  */
    T_OP_HIGH = 312,               /* "HIGH"  */
    T_OP_LOW = 313,                /* "LOW"  */
    T_OP_ISCONST = 314,            /* "ISCONST"  */
    T_OP_STRCMP = 315,             /* "STRCMP"  */
    T_OP_STRIN = 316,              /* "STRIN"  */
    T_OP_STRRIN = 317,             /* "STRRIN"  */
    T_OP_STRSUB = 318,             /* "STRSUB"  */
    T_OP_STRLEN = 319,             /* "STRLEN"  */
    T_OP_STRCAT = 320,             /* "STRCAT"  */
    T_OP_STRUPR = 321,             /* "STRUPR"  */
    T_OP_STRLWR = 322,             /* "STRLWR"  */
    T_OP_STRRPL = 323,             /* "STRRPL"  */
    T_OP_STRFMT = 324,             /* "STRFMT"  */
    T_OP_CHARLEN = 325,            /* "CHARLEN"  */
    T_OP_CHARSUB = 326,            /* "CHARSUB"  */
    T_OP_INCHARMAP = 327,          /* "INCHARMAP"  */
    T_LABEL = 328,                 /* "label"  */
    T_ID = 329,                    /* "identifier"  */
    T_LOCAL_ID = 330,              /* "local identifier"  */
    T_ANON = 331,                  /* "anonymous label"  */
    T_POP_EQU = 332,               /* "EQU"  */
    T_POP_EQUAL = 333,             /* "="  */
    T_POP_EQUS = 334,              /* "EQUS"  */
    T_POP_ADDEQ = 335,             /* "+="  */
    T_POP_SUBEQ = 336,             /* "-="  */
    T_POP_MULEQ = 337,             /* "*="  */
    T_POP_DIVEQ = 338,             /* "/="  */
    T_POP_MODEQ = 339,             /* "%="  */
    T_POP_OREQ = 340,              /* "|="  */
    T_POP_XOREQ = 341,             /* "^="  */
    T_POP_ANDEQ = 342,             /* "&="  */
    T_POP_SHLEQ = 343,             /* "<<="  */
    T_POP_SHREQ = 344,             /* ">>="  */
    T_POP_INCLUDE = 345,           /* "INCLUDE"  */
    T_POP_PRINT = 346,             /* "PRINT"  */
    T_POP_PRINTLN = 347,           /* "PRINTLN"  */
    T_POP_IF = 348,                /* "IF"  */
    T_POP_ELIF = 349,              /* "ELIF"  */
    T_POP_ELSE = 350,              /* "ELSE"  */
    T_POP_ENDC = 351,              /* "ENDC"  */
    T_POP_EXPORT = 352,            /* "EXPORT"  */
    T_POP_DB = 353,                /* "DB"  */
    T_POP_DS = 354,                /* "DS"  */
    T_POP_DW = 355,                /* "DW"  */
    T_POP_DL = 356,                /* "DL"  */
    T_POP_SECTION = 357,           /* "SECTION"  */
    T_POP_FRAGMENT = 358,          /* "FRAGMENT"  */
    T_POP_ENDSECTION = 359,        /* "ENDSECTION"  */
    T_POP_RB = 360,                /* "RB"  */
    T_POP_RW = 361,                /* "RW"  */
    T_POP_MACRO = 362,             /* "MACRO"  */
    T_POP_ENDM = 363,              /* "ENDM"  */
    T_POP_RSRESET = 364,           /* "RSRESET"  */
    T_POP_RSSET = 365,             /* "RSSET"  */
    T_POP_UNION = 366,             /* "UNION"  */
    T_POP_NEXTU = 367,             /* "NEXTU"  */
    T_POP_ENDU = 368,              /* "ENDU"  */
    T_POP_INCBIN = 369,            /* "INCBIN"  */
    T_POP_REPT = 370,              /* "REPT"  */
    T_POP_FOR = 371,               /* "FOR"  */
    T_POP_CHARMAP = 372,           /* "CHARMAP"  */
    T_POP_NEWCHARMAP = 373,        /* "NEWCHARMAP"  */
    T_POP_SETCHARMAP = 374,        /* "SETCHARMAP"  */
    T_POP_PUSHC = 375,             /* "PUSHC"  */
    T_POP_POPC = 376,              /* "POPC"  */
    T_POP_SHIFT = 377,             /* "SHIFT"  */
    T_POP_ENDR = 378,              /* "ENDR"  */
    T_POP_BREAK = 379,             /* "BREAK"  */
    T_POP_LOAD = 380,              /* "LOAD"  */
    T_POP_ENDL = 381,              /* "ENDL"  */
    T_POP_FAIL = 382,              /* "FAIL"  */
    T_POP_WARN = 383,              /* "WARN"  */
    T_POP_FATAL = 384,             /* "FATAL"  */
    T_POP_ASSERT = 385,            /* "ASSERT"  */
    T_POP_STATIC_ASSERT = 386,     /* "STATIC_ASSERT"  */
    T_POP_PURGE = 387,             /* "PURGE"  */
    T_POP_REDEF = 388,             /* "REDEF"  */
    T_POP_POPS = 389,              /* "POPS"  */
    T_POP_PUSHS = 390,             /* "PUSHS"  */
    T_POP_POPO = 391,              /* "POPO"  */
    T_POP_PUSHO = 392,             /* "PUSHO"  */
    T_POP_OPT = 393,               /* "OPT"  */
    T_SECT_ROM0 = 394,             /* "ROM0"  */
    T_SECT_ROMX = 395,             /* "ROMX"  */
    T_SECT_WRAM0 = 396,            /* "WRAM0"  */
    T_SECT_WRAMX = 397,            /* "WRAMX"  */
    T_SECT_HRAM = 398,             /* "HRAM"  */
    T_SECT_VRAM = 399,             /* "VRAM"  */
    T_SECT_SRAM = 400,             /* "SRAM"  */
    T_SECT_OAM = 401,              /* "OAM"  */
    T_Z80_ADC = 402,               /* "adc"  */
    T_Z80_ADD = 403,               /* "add"  */
    T_Z80_AND = 404,               /* "and"  */
    T_Z80_BIT = 405,               /* "bit"  */
    T_Z80_CALL = 406,              /* "call"  */
    T_Z80_CCF = 407,               /* "ccf"  */
    T_Z80_CP = 408,                /* "cp"  */
    T_Z80_CPL = 409,               /* "cpl"  */
    T_Z80_DAA = 410,               /* "daa"  */
    T_Z80_DEC = 411,               /* "dec"  */
    T_Z80_DI = 412,                /* "di"  */
    T_Z80_EI = 413,                /* "ei"  */
    T_Z80_HALT = 414,              /* "halt"  */
    T_Z80_INC = 415,               /* "inc"  */
    T_Z80_JP = 416,                /* "jp"  */
    T_Z80_JR = 417,                /* "jr"  */
    T_Z80_LD = 418,                /* "ld"  */
    T_Z80_LDI = 419,               /* "ldi"  */
    T_Z80_LDD = 420,               /* "ldd"  */
    T_Z80_LDH = 421,               /* "ldh"  */
    T_Z80_NOP = 422,               /* "nop"  */
    T_Z80_OR = 423,                /* "or"  */
    T_Z80_POP = 424,               /* "pop"  */
    T_Z80_PUSH = 425,              /* "push"  */
    T_Z80_RES = 426,               /* "res"  */
    T_Z80_RET = 427,               /* "ret"  */
    T_Z80_RETI = 428,              /* "reti"  */
    T_Z80_RST = 429,               /* "rst"  */
    T_Z80_RL = 430,                /* "rl"  */
    T_Z80_RLA = 431,               /* "rla"  */
    T_Z80_RLC = 432,               /* "rlc"  */
    T_Z80_RLCA = 433,              /* "rlca"  */
    T_Z80_RR = 434,                /* "rr"  */
    T_Z80_RRA = 435,               /* "rra"  */
    T_Z80_RRC = 436,               /* "rrc"  */
    T_Z80_RRCA = 437,              /* "rrca"  */
    T_Z80_SBC = 438,               /* "sbc"  */
    T_Z80_SCF = 439,               /* "scf"  */
    T_Z80_SET = 440,               /* "set"  */
    T_Z80_STOP = 441,              /* "stop"  */
    T_Z80_SLA = 442,               /* "sla"  */
    T_Z80_SRA = 443,               /* "sra"  */
    T_Z80_SRL = 444,               /* "srl"  */
    T_Z80_SUB = 445,               /* "sub"  */
    T_Z80_SWAP = 446,              /* "swap"  */
    T_Z80_XOR = 447,               /* "xor"  */
    T_TOKEN_A = 448,               /* "a"  */
    T_TOKEN_B = 449,               /* "b"  */
    T_TOKEN_C = 450,               /* "c"  */
    T_TOKEN_D = 451,               /* "d"  */
    T_TOKEN_E = 452,               /* "e"  */
    T_TOKEN_H = 453,               /* "h"  */
    T_TOKEN_L = 454,               /* "l"  */
    T_MODE_AF = 455,               /* "af"  */
    T_MODE_BC = 456,               /* "bc"  */
    T_MODE_DE = 457,               /* "de"  */
    T_MODE_SP = 458,               /* "sp"  */
    T_MODE_HL = 459,               /* "hl"  */
    T_MODE_HL_DEC = 460,           /* "hld/hl-"  */
    T_MODE_HL_INC = 461,           /* "hli/hl+"  */
    T_CC_NZ = 462,                 /* "nz"  */
    T_CC_Z = 463,                  /* "z"  */
    T_CC_NC = 464,                 /* "nc"  */
    T_EOB = 465                    /* "end of buffer"  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 496 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"

	char symName[MAXSYMLEN + 1];
	char string[MAXSTRLEN + 1];
	struct Expression expr;
	int32_t constValue;
	enum RPNCommand compoundEqual;
	enum SectionModifier sectMod;
	struct SectionSpec sectSpec;
	struct MacroArgs *macroArg;
	enum AssertionType assertType;
	struct AlignmentSpec alignSpec;
	struct DsArgList dsArgs;
	struct PurgeArgList purgeArgs;
	struct {
		int32_t start;
		int32_t stop;
		int32_t step;
	} forArgs;
	struct StrFmtArgList strfmtArgs;
	bool captureTerminated;

#line 296 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_HOME_DESTTINGHIM_CODE_RANDOM_RGBDS_ZIG_CACHE_TMP_215B8502AE944A74_PARSER_HPP_INCLUDED  */

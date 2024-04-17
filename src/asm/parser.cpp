/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 3 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm/charmap.hpp"
#include "asm/fixpoint.hpp"
#include "asm/format.hpp"
#include "asm/fstack.hpp"
#include "asm/lexer.hpp"
#include "asm/macro.hpp"
#include "asm/main.hpp"
#include "asm/opt.hpp"
#include "asm/output.hpp"
#include "asm/rpn.hpp"
#include "asm/section.hpp"
#include "asm/symbol.hpp"
#include "util.hpp"
#include "asm/warning.hpp"

#include "extern/utf8decoder.hpp"

#include "linkdefs.hpp"
#include "platform.hpp" // strncasecmp, strdup

static struct CaptureBody captureBody; // Captures a REPT/FOR or MACRO

static void upperstring(char *dest, char const *src)
{
	while (*src)
		*dest++ = toupper(*src++);
	*dest = '\0';
}

static void lowerstring(char *dest, char const *src)
{
	while (*src)
		*dest++ = tolower(*src++);
	*dest = '\0';
}

static uint32_t str2int2(uint8_t *s, uint32_t length)
{
	if (length > 4)
		warning(WARNING_NUMERIC_STRING_1,
			"Treating string as a number ignores first %" PRIu32 " character%s\n",
			length - 4, length == 5 ? "" : "s");
	else if (length > 1)
		warning(WARNING_NUMERIC_STRING_2,
			"Treating %" PRIu32 "-character string as a number\n", length);

	uint32_t r = 0;

	for (uint32_t i = length < 4 ? 0 : length - 4; i < length; i++) {
		r <<= 8;
		r |= s[i];
	}

	return r;
}

static const char *strrstr(char const *s1, char const *s2)
{
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);

	if (len2 > len1)
		return NULL;

	for (char const *p = s1 + len1 - len2; p >= s1; p--)
		if (!strncmp(p, s2, len2))
			return p;

	return NULL;
}

static void errorInvalidUTF8Byte(uint8_t byte, char const *functionName)
{
	error("%s: Invalid UTF-8 byte 0x%02hhX\n", functionName, byte);
}

static size_t strlenUTF8(char const *s)
{
	size_t len = 0;
	uint32_t state = 0;

	for (uint32_t codep = 0; *s; s++) {
		uint8_t byte = *s;

		switch (decode(&state, &codep, byte)) {
		case 1:
			errorInvalidUTF8Byte(byte, "STRLEN");
			state = 0;
			// fallthrough
		case 0:
			len++;
			break;
		}
	}

	// Check for partial code point.
	if (state != 0)
		error("STRLEN: Incomplete UTF-8 character\n");

	return len;
}

static void strsubUTF8(char *dest, size_t destLen, char const *src, uint32_t pos, uint32_t len)
{
	size_t srcIndex = 0;
	size_t destIndex = 0;
	uint32_t state = 0;
	uint32_t codep = 0;
	uint32_t curLen = 0;
	uint32_t curPos = 1;

	// Advance to starting position in source string.
	while (src[srcIndex] && curPos < pos) {
		switch (decode(&state, &codep, src[srcIndex])) {
		case 1:
			errorInvalidUTF8Byte(src[srcIndex], "STRSUB");
			state = 0;
			// fallthrough
		case 0:
			curPos++;
			break;
		}
		srcIndex++;
	}

	// A position 1 past the end of the string is allowed, but will trigger the
	// "Length too big" warning below if the length is nonzero.
	if (!src[srcIndex] && pos > curPos)
		warning(WARNING_BUILTIN_ARG,
			"STRSUB: Position %" PRIu32 " is past the end of the string\n", pos);

	// Copy from source to destination.
	while (src[srcIndex] && destIndex < destLen - 1 && curLen < len) {
		switch (decode(&state, &codep, src[srcIndex])) {
		case 1:
			errorInvalidUTF8Byte(src[srcIndex], "STRSUB");
			state = 0;
			// fallthrough
		case 0:
			curLen++;
			break;
		}
		dest[destIndex++] = src[srcIndex++];
	}

	if (curLen < len)
		warning(WARNING_BUILTIN_ARG, "STRSUB: Length too big: %" PRIu32 "\n", len);

	// Check for partial code point.
	if (state != 0)
		error("STRSUB: Incomplete UTF-8 character\n");

	dest[destIndex] = '\0';
}

static size_t charlenUTF8(char const *s)
{
	size_t len;

	for (len = 0; charmap_ConvertNext(&s, NULL); len++)
		;

	return len;
}

static void charsubUTF8(char *dest, char const *src, uint32_t pos)
{
	size_t charLen = 1;

	// Advance to starting position in source string.
	for (uint32_t curPos = 1; charLen && curPos < pos; curPos++)
		charLen = charmap_ConvertNext(&src, NULL);

	char const *start = src;

	if (!charmap_ConvertNext(&src, NULL))
		warning(WARNING_BUILTIN_ARG,
			"CHARSUB: Position %" PRIu32 " is past the end of the string\n", pos);

	// Copy from source to destination.
	memcpy(dest, start, src - start);

	dest[src - start] = '\0';
}

static uint32_t adjustNegativePos(int32_t pos, size_t len, char const *functionName)
{
	// STRSUB and CHARSUB adjust negative `pos` arguments the same way,
	// such that position -1 is the last character of a string.
	if (pos < 0)
		pos += len + 1;
	if (pos < 1) {
		warning(WARNING_BUILTIN_ARG, "%s: Position starts at 1\n", functionName);
		pos = 1;
	}
	return (uint32_t)pos;
}

static void strrpl(char *dest, size_t destLen, char const *src, char const *old, char const *rep)
{
	size_t oldLen = strlen(old);
	size_t repLen = strlen(rep);
	size_t i = 0;

	if (!oldLen) {
		warning(WARNING_EMPTY_STRRPL, "STRRPL: Cannot replace an empty string\n");
		strcpy(dest, src);
		return;
	}

	for (char const *next = strstr(src, old); next && *next; next = strstr(src, old)) {
		// Copy anything before the substring to replace
		unsigned int lenBefore = next - src;

		memcpy(dest + i, src, lenBefore < destLen - i ? lenBefore : destLen - i);
		i += next - src;
		if (i >= destLen)
			break;

		// Copy the replacement substring
		memcpy(dest + i, rep, repLen < destLen - i ? repLen : destLen - i);
		i += repLen;
		if (i >= destLen)
			break;

		src = next + oldLen;
	}

	if (i < destLen) {
		size_t srcLen = strlen(src);

		// Copy anything after the last replaced substring
		memcpy(dest + i, src, srcLen < destLen - i ? srcLen : destLen - i);
		i += srcLen;
	}

	if (i >= destLen) {
		warning(WARNING_LONG_STR, "STRRPL: String too long, got truncated\n");
		i = destLen - 1;
	}
	dest[i] = '\0';
}

static void initStrFmtArgList(struct StrFmtArgList *args)
{
	args->nbArgs = 0;
	args->capacity = INITIAL_STRFMT_ARG_SIZE;
	args->args = (struct StrFmtArg *)malloc(args->capacity * sizeof(*args->args));
	if (!args->args)
		fatalerror("Failed to allocate memory for STRFMT arg list: %s\n",
			   strerror(errno));
}

static size_t nextStrFmtArgListIndex(struct StrFmtArgList *args)
{
	if (args->nbArgs == args->capacity) {
		args->capacity = (args->capacity + 1) * 2;
		args->args = (struct StrFmtArg *)realloc(args->args, args->capacity * sizeof(*args->args));
		if (!args->args)
			fatalerror("realloc error while resizing STRFMT arg list: %s\n",
				   strerror(errno));
	}
	return args->nbArgs++;
}

static void freeStrFmtArgList(struct StrFmtArgList *args)
{
	free(args->format);
	for (size_t i = 0; i < args->nbArgs; i++)
		if (!args->args[i].isNumeric)
			free(args->args[i].string);
	free(args->args);
}

static void strfmt(char *dest, size_t destLen, char const *fmt, size_t nbArgs, struct StrFmtArg *args)
{
	size_t a = 0;
	size_t i = 0;

	while (i < destLen) {
		int c = *fmt++;

		if (c == '\0') {
			break;
		} else if (c != '%') {
			dest[i++] = c;
			continue;
		}

		c = *fmt++;

		if (c == '%') {
			dest[i++] = c;
			continue;
		}

		struct FormatSpec spec = fmt_NewSpec();

		while (c != '\0') {
			fmt_UseCharacter(&spec, c);
			if (fmt_IsFinished(&spec))
				break;
			c = *fmt++;
		}

		if (fmt_IsEmpty(&spec)) {
			error("STRFMT: Illegal '%%' at end of format string\n");
			dest[i++] = '%';
			break;
		} else if (!fmt_IsValid(&spec)) {
			error("STRFMT: Invalid format spec for argument %zu\n", a + 1);
			dest[i++] = '%';
			a++;
			continue;
		} else if (a >= nbArgs) {
			// Will warn after formatting is done.
			dest[i++] = '%';
			a++;
			continue;
		}

		struct StrFmtArg *arg = &args[a++];
		static char buf[MAXSTRLEN + 1];

		if (arg->isNumeric)
			fmt_PrintNumber(buf, sizeof(buf), &spec, arg->number);
		else
			fmt_PrintString(buf, sizeof(buf), &spec, arg->string);

		i += snprintf(&dest[i], destLen - i, "%s", buf);
	}

	if (a < nbArgs)
		error("STRFMT: %zu unformatted argument(s)\n", nbArgs - a);
	else if (a > nbArgs)
		error("STRFMT: Not enough arguments for format spec, got: %zu, need: %zu\n", nbArgs, a);

	if (i > destLen - 1) {
		warning(WARNING_LONG_STR, "STRFMT: String too long, got truncated\n");
		i = destLen - 1;
	}
	dest[i] = '\0';
}

static void compoundAssignment(const char *symName, enum RPNCommand op, int32_t constValue) {
	struct Expression oldExpr, constExpr, newExpr;
	int32_t newValue;

	rpn_Symbol(&oldExpr, symName);
	rpn_Number(&constExpr, constValue);
	rpn_BinaryOp(op, &newExpr, &oldExpr, &constExpr);
	newValue = rpn_GetConstVal(&newExpr);
	sym_AddVar(symName, newValue);
}

static void initDsArgList(struct DsArgList *args)
{
	args->nbArgs = 0;
	args->capacity = INITIAL_DS_ARG_SIZE;
	args->args = (struct Expression *)malloc(args->capacity * sizeof(*args->args));
	if (!args->args)
		fatalerror("Failed to allocate memory for ds arg list: %s\n",
			   strerror(errno));
}

static void appendDsArgList(struct DsArgList *args, const struct Expression *expr)
{
	if (args->nbArgs == args->capacity) {
		args->capacity = (args->capacity + 1) * 2;
		args->args = (struct Expression *)realloc(args->args, args->capacity * sizeof(*args->args));
		if (!args->args)
			fatalerror("realloc error while resizing ds arg list: %s\n",
				   strerror(errno));
	}
	args->args[args->nbArgs++] = *expr;
}

static void freeDsArgList(struct DsArgList *args)
{
	free(args->args);
}

static void initPurgeArgList(struct PurgeArgList *args)
{
	args->nbArgs = 0;
	args->capacity = INITIAL_PURGE_ARG_SIZE;
	args->args = (char **)malloc(args->capacity * sizeof(*args->args));
	if (!args->args)
		fatalerror("Failed to allocate memory for purge arg list: %s\n",
			   strerror(errno));
}

static void appendPurgeArgList(struct PurgeArgList *args, char *arg)
{
	if (args->nbArgs == args->capacity) {
		args->capacity = (args->capacity + 1) * 2;
		args->args = (char **)realloc(args->args, args->capacity * sizeof(*args->args));
		if (!args->args)
			fatalerror("realloc error while resizing purge arg list: %s\n",
				   strerror(errno));
	}
	args->args[args->nbArgs++] = arg;
}

static void freePurgeArgList(struct PurgeArgList *args)
{
	for (size_t i = 0; i < args->nbArgs; i++)
		free(args->args[i]);
	free(args->args);
}

static void failAssert(enum AssertionType type)
{
	switch (type) {
		case ASSERT_FATAL:
			fatalerror("Assertion failed\n");
		case ASSERT_ERROR:
			error("Assertion failed\n");
			break;
		case ASSERT_WARN:
			warning(WARNING_ASSERT, "Assertion failed\n");
			break;
	}
}

static void failAssertMsg(enum AssertionType type, char const *msg)
{
	switch (type) {
		case ASSERT_FATAL:
			fatalerror("Assertion failed: %s\n", msg);
		case ASSERT_ERROR:
			error("Assertion failed: %s\n", msg);
			break;
		case ASSERT_WARN:
			warning(WARNING_ASSERT, "Assertion failed: %s\n", msg);
			break;
	}
}

void yyerror(char const *str)
{
	error("%s\n", str);
}

// The CPU encodes instructions in a logical way, so most instructions actually follow patterns.
// These enums thus help with bit twiddling to compute opcodes
enum {
	REG_B = 0,
	REG_C,
	REG_D,
	REG_E,
	REG_H,
	REG_L,
	REG_HL_IND,
	REG_A
};

enum {
	REG_BC_IND = 0,
	REG_DE_IND,
	REG_HL_INDINC,
	REG_HL_INDDEC,
};

enum {
	REG_BC = 0,
	REG_DE = 1,
	REG_HL = 2,
	// LD/INC/ADD/DEC allow SP, PUSH/POP allow AF
	REG_SP = 3,
	REG_AF = 3
};

enum {
	CC_NZ = 0,
	CC_Z,
	CC_NC,
	CC_C
};


#line 563 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_T_NUMBER = 3,                   /* "number"  */
  YYSYMBOL_T_STRING = 4,                   /* "string"  */
  YYSYMBOL_T_PERIOD = 5,                   /* "."  */
  YYSYMBOL_T_COMMA = 6,                    /* ","  */
  YYSYMBOL_T_COLON = 7,                    /* ":"  */
  YYSYMBOL_T_DOUBLE_COLON = 8,             /* "::"  */
  YYSYMBOL_T_LBRACK = 9,                   /* "["  */
  YYSYMBOL_T_RBRACK = 10,                  /* "]"  */
  YYSYMBOL_T_LPAREN = 11,                  /* "("  */
  YYSYMBOL_T_RPAREN = 12,                  /* ")"  */
  YYSYMBOL_T_NEWLINE = 13,                 /* "newline"  */
  YYSYMBOL_T_OP_LOGICNOT = 14,             /* "!"  */
  YYSYMBOL_T_OP_LOGICAND = 15,             /* "&&"  */
  YYSYMBOL_T_OP_LOGICOR = 16,              /* "||"  */
  YYSYMBOL_T_OP_LOGICGT = 17,              /* ">"  */
  YYSYMBOL_T_OP_LOGICLT = 18,              /* "<"  */
  YYSYMBOL_T_OP_LOGICGE = 19,              /* ">="  */
  YYSYMBOL_T_OP_LOGICLE = 20,              /* "<="  */
  YYSYMBOL_T_OP_LOGICNE = 21,              /* "!="  */
  YYSYMBOL_T_OP_LOGICEQU = 22,             /* "=="  */
  YYSYMBOL_T_OP_ADD = 23,                  /* "+"  */
  YYSYMBOL_T_OP_SUB = 24,                  /* "-"  */
  YYSYMBOL_T_OP_OR = 25,                   /* "|"  */
  YYSYMBOL_T_OP_XOR = 26,                  /* "^"  */
  YYSYMBOL_T_OP_AND = 27,                  /* "&"  */
  YYSYMBOL_T_OP_SHL = 28,                  /* "<<"  */
  YYSYMBOL_T_OP_SHR = 29,                  /* ">>"  */
  YYSYMBOL_T_OP_USHR = 30,                 /* ">>>"  */
  YYSYMBOL_T_OP_MUL = 31,                  /* "*"  */
  YYSYMBOL_T_OP_DIV = 32,                  /* "/"  */
  YYSYMBOL_T_OP_MOD = 33,                  /* "%"  */
  YYSYMBOL_T_OP_NOT = 34,                  /* "~"  */
  YYSYMBOL_NEG = 35,                       /* NEG  */
  YYSYMBOL_T_OP_EXP = 36,                  /* "**"  */
  YYSYMBOL_T_OP_DEF = 37,                  /* "DEF"  */
  YYSYMBOL_T_OP_BANK = 38,                 /* "BANK"  */
  YYSYMBOL_T_OP_ALIGN = 39,                /* "ALIGN"  */
  YYSYMBOL_T_OP_SIZEOF = 40,               /* "SIZEOF"  */
  YYSYMBOL_T_OP_STARTOF = 41,              /* "STARTOF"  */
  YYSYMBOL_T_OP_SIN = 42,                  /* "SIN"  */
  YYSYMBOL_T_OP_COS = 43,                  /* "COS"  */
  YYSYMBOL_T_OP_TAN = 44,                  /* "TAN"  */
  YYSYMBOL_T_OP_ASIN = 45,                 /* "ASIN"  */
  YYSYMBOL_T_OP_ACOS = 46,                 /* "ACOS"  */
  YYSYMBOL_T_OP_ATAN = 47,                 /* "ATAN"  */
  YYSYMBOL_T_OP_ATAN2 = 48,                /* "ATAN2"  */
  YYSYMBOL_T_OP_FDIV = 49,                 /* "FDIV"  */
  YYSYMBOL_T_OP_FMUL = 50,                 /* "FMUL"  */
  YYSYMBOL_T_OP_FMOD = 51,                 /* "FMOD"  */
  YYSYMBOL_T_OP_POW = 52,                  /* "POW"  */
  YYSYMBOL_T_OP_LOG = 53,                  /* "LOG"  */
  YYSYMBOL_T_OP_ROUND = 54,                /* "ROUND"  */
  YYSYMBOL_T_OP_CEIL = 55,                 /* "CEIL"  */
  YYSYMBOL_T_OP_FLOOR = 56,                /* "FLOOR"  */
  YYSYMBOL_T_OP_HIGH = 57,                 /* "HIGH"  */
  YYSYMBOL_T_OP_LOW = 58,                  /* "LOW"  */
  YYSYMBOL_T_OP_ISCONST = 59,              /* "ISCONST"  */
  YYSYMBOL_T_OP_STRCMP = 60,               /* "STRCMP"  */
  YYSYMBOL_T_OP_STRIN = 61,                /* "STRIN"  */
  YYSYMBOL_T_OP_STRRIN = 62,               /* "STRRIN"  */
  YYSYMBOL_T_OP_STRSUB = 63,               /* "STRSUB"  */
  YYSYMBOL_T_OP_STRLEN = 64,               /* "STRLEN"  */
  YYSYMBOL_T_OP_STRCAT = 65,               /* "STRCAT"  */
  YYSYMBOL_T_OP_STRUPR = 66,               /* "STRUPR"  */
  YYSYMBOL_T_OP_STRLWR = 67,               /* "STRLWR"  */
  YYSYMBOL_T_OP_STRRPL = 68,               /* "STRRPL"  */
  YYSYMBOL_T_OP_STRFMT = 69,               /* "STRFMT"  */
  YYSYMBOL_T_OP_CHARLEN = 70,              /* "CHARLEN"  */
  YYSYMBOL_T_OP_CHARSUB = 71,              /* "CHARSUB"  */
  YYSYMBOL_T_OP_INCHARMAP = 72,            /* "INCHARMAP"  */
  YYSYMBOL_T_LABEL = 73,                   /* "label"  */
  YYSYMBOL_T_ID = 74,                      /* "identifier"  */
  YYSYMBOL_T_LOCAL_ID = 75,                /* "local identifier"  */
  YYSYMBOL_T_ANON = 76,                    /* "anonymous label"  */
  YYSYMBOL_T_POP_EQU = 77,                 /* "EQU"  */
  YYSYMBOL_T_POP_EQUAL = 78,               /* "="  */
  YYSYMBOL_T_POP_EQUS = 79,                /* "EQUS"  */
  YYSYMBOL_T_POP_ADDEQ = 80,               /* "+="  */
  YYSYMBOL_T_POP_SUBEQ = 81,               /* "-="  */
  YYSYMBOL_T_POP_MULEQ = 82,               /* "*="  */
  YYSYMBOL_T_POP_DIVEQ = 83,               /* "/="  */
  YYSYMBOL_T_POP_MODEQ = 84,               /* "%="  */
  YYSYMBOL_T_POP_OREQ = 85,                /* "|="  */
  YYSYMBOL_T_POP_XOREQ = 86,               /* "^="  */
  YYSYMBOL_T_POP_ANDEQ = 87,               /* "&="  */
  YYSYMBOL_T_POP_SHLEQ = 88,               /* "<<="  */
  YYSYMBOL_T_POP_SHREQ = 89,               /* ">>="  */
  YYSYMBOL_T_POP_INCLUDE = 90,             /* "INCLUDE"  */
  YYSYMBOL_T_POP_PRINT = 91,               /* "PRINT"  */
  YYSYMBOL_T_POP_PRINTLN = 92,             /* "PRINTLN"  */
  YYSYMBOL_T_POP_IF = 93,                  /* "IF"  */
  YYSYMBOL_T_POP_ELIF = 94,                /* "ELIF"  */
  YYSYMBOL_T_POP_ELSE = 95,                /* "ELSE"  */
  YYSYMBOL_T_POP_ENDC = 96,                /* "ENDC"  */
  YYSYMBOL_T_POP_EXPORT = 97,              /* "EXPORT"  */
  YYSYMBOL_T_POP_DB = 98,                  /* "DB"  */
  YYSYMBOL_T_POP_DS = 99,                  /* "DS"  */
  YYSYMBOL_T_POP_DW = 100,                 /* "DW"  */
  YYSYMBOL_T_POP_DL = 101,                 /* "DL"  */
  YYSYMBOL_T_POP_SECTION = 102,            /* "SECTION"  */
  YYSYMBOL_T_POP_FRAGMENT = 103,           /* "FRAGMENT"  */
  YYSYMBOL_T_POP_ENDSECTION = 104,         /* "ENDSECTION"  */
  YYSYMBOL_T_POP_RB = 105,                 /* "RB"  */
  YYSYMBOL_T_POP_RW = 106,                 /* "RW"  */
  YYSYMBOL_T_POP_MACRO = 107,              /* "MACRO"  */
  YYSYMBOL_T_POP_ENDM = 108,               /* "ENDM"  */
  YYSYMBOL_T_POP_RSRESET = 109,            /* "RSRESET"  */
  YYSYMBOL_T_POP_RSSET = 110,              /* "RSSET"  */
  YYSYMBOL_T_POP_UNION = 111,              /* "UNION"  */
  YYSYMBOL_T_POP_NEXTU = 112,              /* "NEXTU"  */
  YYSYMBOL_T_POP_ENDU = 113,               /* "ENDU"  */
  YYSYMBOL_T_POP_INCBIN = 114,             /* "INCBIN"  */
  YYSYMBOL_T_POP_REPT = 115,               /* "REPT"  */
  YYSYMBOL_T_POP_FOR = 116,                /* "FOR"  */
  YYSYMBOL_T_POP_CHARMAP = 117,            /* "CHARMAP"  */
  YYSYMBOL_T_POP_NEWCHARMAP = 118,         /* "NEWCHARMAP"  */
  YYSYMBOL_T_POP_SETCHARMAP = 119,         /* "SETCHARMAP"  */
  YYSYMBOL_T_POP_PUSHC = 120,              /* "PUSHC"  */
  YYSYMBOL_T_POP_POPC = 121,               /* "POPC"  */
  YYSYMBOL_T_POP_SHIFT = 122,              /* "SHIFT"  */
  YYSYMBOL_T_POP_ENDR = 123,               /* "ENDR"  */
  YYSYMBOL_T_POP_BREAK = 124,              /* "BREAK"  */
  YYSYMBOL_T_POP_LOAD = 125,               /* "LOAD"  */
  YYSYMBOL_T_POP_ENDL = 126,               /* "ENDL"  */
  YYSYMBOL_T_POP_FAIL = 127,               /* "FAIL"  */
  YYSYMBOL_T_POP_WARN = 128,               /* "WARN"  */
  YYSYMBOL_T_POP_FATAL = 129,              /* "FATAL"  */
  YYSYMBOL_T_POP_ASSERT = 130,             /* "ASSERT"  */
  YYSYMBOL_T_POP_STATIC_ASSERT = 131,      /* "STATIC_ASSERT"  */
  YYSYMBOL_T_POP_PURGE = 132,              /* "PURGE"  */
  YYSYMBOL_T_POP_REDEF = 133,              /* "REDEF"  */
  YYSYMBOL_T_POP_POPS = 134,               /* "POPS"  */
  YYSYMBOL_T_POP_PUSHS = 135,              /* "PUSHS"  */
  YYSYMBOL_T_POP_POPO = 136,               /* "POPO"  */
  YYSYMBOL_T_POP_PUSHO = 137,              /* "PUSHO"  */
  YYSYMBOL_T_POP_OPT = 138,                /* "OPT"  */
  YYSYMBOL_T_SECT_ROM0 = 139,              /* "ROM0"  */
  YYSYMBOL_T_SECT_ROMX = 140,              /* "ROMX"  */
  YYSYMBOL_T_SECT_WRAM0 = 141,             /* "WRAM0"  */
  YYSYMBOL_T_SECT_WRAMX = 142,             /* "WRAMX"  */
  YYSYMBOL_T_SECT_HRAM = 143,              /* "HRAM"  */
  YYSYMBOL_T_SECT_VRAM = 144,              /* "VRAM"  */
  YYSYMBOL_T_SECT_SRAM = 145,              /* "SRAM"  */
  YYSYMBOL_T_SECT_OAM = 146,               /* "OAM"  */
  YYSYMBOL_T_Z80_ADC = 147,                /* "adc"  */
  YYSYMBOL_T_Z80_ADD = 148,                /* "add"  */
  YYSYMBOL_T_Z80_AND = 149,                /* "and"  */
  YYSYMBOL_T_Z80_BIT = 150,                /* "bit"  */
  YYSYMBOL_T_Z80_CALL = 151,               /* "call"  */
  YYSYMBOL_T_Z80_CCF = 152,                /* "ccf"  */
  YYSYMBOL_T_Z80_CP = 153,                 /* "cp"  */
  YYSYMBOL_T_Z80_CPL = 154,                /* "cpl"  */
  YYSYMBOL_T_Z80_DAA = 155,                /* "daa"  */
  YYSYMBOL_T_Z80_DEC = 156,                /* "dec"  */
  YYSYMBOL_T_Z80_DI = 157,                 /* "di"  */
  YYSYMBOL_T_Z80_EI = 158,                 /* "ei"  */
  YYSYMBOL_T_Z80_HALT = 159,               /* "halt"  */
  YYSYMBOL_T_Z80_INC = 160,                /* "inc"  */
  YYSYMBOL_T_Z80_JP = 161,                 /* "jp"  */
  YYSYMBOL_T_Z80_JR = 162,                 /* "jr"  */
  YYSYMBOL_T_Z80_LD = 163,                 /* "ld"  */
  YYSYMBOL_T_Z80_LDI = 164,                /* "ldi"  */
  YYSYMBOL_T_Z80_LDD = 165,                /* "ldd"  */
  YYSYMBOL_T_Z80_LDH = 166,                /* "ldh"  */
  YYSYMBOL_T_Z80_NOP = 167,                /* "nop"  */
  YYSYMBOL_T_Z80_OR = 168,                 /* "or"  */
  YYSYMBOL_T_Z80_POP = 169,                /* "pop"  */
  YYSYMBOL_T_Z80_PUSH = 170,               /* "push"  */
  YYSYMBOL_T_Z80_RES = 171,                /* "res"  */
  YYSYMBOL_T_Z80_RET = 172,                /* "ret"  */
  YYSYMBOL_T_Z80_RETI = 173,               /* "reti"  */
  YYSYMBOL_T_Z80_RST = 174,                /* "rst"  */
  YYSYMBOL_T_Z80_RL = 175,                 /* "rl"  */
  YYSYMBOL_T_Z80_RLA = 176,                /* "rla"  */
  YYSYMBOL_T_Z80_RLC = 177,                /* "rlc"  */
  YYSYMBOL_T_Z80_RLCA = 178,               /* "rlca"  */
  YYSYMBOL_T_Z80_RR = 179,                 /* "rr"  */
  YYSYMBOL_T_Z80_RRA = 180,                /* "rra"  */
  YYSYMBOL_T_Z80_RRC = 181,                /* "rrc"  */
  YYSYMBOL_T_Z80_RRCA = 182,               /* "rrca"  */
  YYSYMBOL_T_Z80_SBC = 183,                /* "sbc"  */
  YYSYMBOL_T_Z80_SCF = 184,                /* "scf"  */
  YYSYMBOL_T_Z80_SET = 185,                /* "set"  */
  YYSYMBOL_T_Z80_STOP = 186,               /* "stop"  */
  YYSYMBOL_T_Z80_SLA = 187,                /* "sla"  */
  YYSYMBOL_T_Z80_SRA = 188,                /* "sra"  */
  YYSYMBOL_T_Z80_SRL = 189,                /* "srl"  */
  YYSYMBOL_T_Z80_SUB = 190,                /* "sub"  */
  YYSYMBOL_T_Z80_SWAP = 191,               /* "swap"  */
  YYSYMBOL_T_Z80_XOR = 192,                /* "xor"  */
  YYSYMBOL_T_TOKEN_A = 193,                /* "a"  */
  YYSYMBOL_T_TOKEN_B = 194,                /* "b"  */
  YYSYMBOL_T_TOKEN_C = 195,                /* "c"  */
  YYSYMBOL_T_TOKEN_D = 196,                /* "d"  */
  YYSYMBOL_T_TOKEN_E = 197,                /* "e"  */
  YYSYMBOL_T_TOKEN_H = 198,                /* "h"  */
  YYSYMBOL_T_TOKEN_L = 199,                /* "l"  */
  YYSYMBOL_T_MODE_AF = 200,                /* "af"  */
  YYSYMBOL_T_MODE_BC = 201,                /* "bc"  */
  YYSYMBOL_T_MODE_DE = 202,                /* "de"  */
  YYSYMBOL_T_MODE_SP = 203,                /* "sp"  */
  YYSYMBOL_T_MODE_HL = 204,                /* "hl"  */
  YYSYMBOL_T_MODE_HL_DEC = 205,            /* "hld/hl-"  */
  YYSYMBOL_T_MODE_HL_INC = 206,            /* "hli/hl+"  */
  YYSYMBOL_T_CC_NZ = 207,                  /* "nz"  */
  YYSYMBOL_T_CC_Z = 208,                   /* "z"  */
  YYSYMBOL_T_CC_NC = 209,                  /* "nc"  */
  YYSYMBOL_T_EOB = 210,                    /* "end of buffer"  */
  YYSYMBOL_YYACCEPT = 211,                 /* $accept  */
  YYSYMBOL_asmfile = 212,                  /* asmfile  */
  YYSYMBOL_lines = 213,                    /* lines  */
  YYSYMBOL_endofline = 214,                /* endofline  */
  YYSYMBOL_opt_diff_mark = 215,            /* opt_diff_mark  */
  YYSYMBOL_line = 216,                     /* line  */
  YYSYMBOL_217_1 = 217,                    /* $@1  */
  YYSYMBOL_218_2 = 218,                    /* $@2  */
  YYSYMBOL_line_directive = 219,           /* line_directive  */
  YYSYMBOL_if = 220,                       /* if  */
  YYSYMBOL_elif = 221,                     /* elif  */
  YYSYMBOL_else = 222,                     /* else  */
  YYSYMBOL_plain_directive = 223,          /* plain_directive  */
  YYSYMBOL_endc = 224,                     /* endc  */
  YYSYMBOL_def_id = 225,                   /* def_id  */
  YYSYMBOL_226_3 = 226,                    /* $@3  */
  YYSYMBOL_redef_id = 227,                 /* redef_id  */
  YYSYMBOL_228_4 = 228,                    /* $@4  */
  YYSYMBOL_scoped_id = 229,                /* scoped_id  */
  YYSYMBOL_scoped_anon_id = 230,           /* scoped_anon_id  */
  YYSYMBOL_label = 231,                    /* label  */
  YYSYMBOL_macro = 232,                    /* macro  */
  YYSYMBOL_233_5 = 233,                    /* $@5  */
  YYSYMBOL_macroargs = 234,                /* macroargs  */
  YYSYMBOL_assignment_directive = 235,     /* assignment_directive  */
  YYSYMBOL_directive = 236,                /* directive  */
  YYSYMBOL_trailing_comma = 237,           /* trailing_comma  */
  YYSYMBOL_compoundeq = 238,               /* compoundeq  */
  YYSYMBOL_equ = 239,                      /* equ  */
  YYSYMBOL_assignment = 240,               /* assignment  */
  YYSYMBOL_equs = 241,                     /* equs  */
  YYSYMBOL_rb = 242,                       /* rb  */
  YYSYMBOL_rw = 243,                       /* rw  */
  YYSYMBOL_rl = 244,                       /* rl  */
  YYSYMBOL_align = 245,                    /* align  */
  YYSYMBOL_align_spec = 246,               /* align_spec  */
  YYSYMBOL_opt = 247,                      /* opt  */
  YYSYMBOL_248_6 = 248,                    /* $@6  */
  YYSYMBOL_opt_list = 249,                 /* opt_list  */
  YYSYMBOL_opt_list_entry = 250,           /* opt_list_entry  */
  YYSYMBOL_popo = 251,                     /* popo  */
  YYSYMBOL_pusho = 252,                    /* pusho  */
  YYSYMBOL_pops = 253,                     /* pops  */
  YYSYMBOL_pushs = 254,                    /* pushs  */
  YYSYMBOL_endsection = 255,               /* endsection  */
  YYSYMBOL_fail = 256,                     /* fail  */
  YYSYMBOL_warn = 257,                     /* warn  */
  YYSYMBOL_assert_type = 258,              /* assert_type  */
  YYSYMBOL_assert = 259,                   /* assert  */
  YYSYMBOL_shift = 260,                    /* shift  */
  YYSYMBOL_load = 261,                     /* load  */
  YYSYMBOL_rept = 262,                     /* rept  */
  YYSYMBOL_263_7 = 263,                    /* @7  */
  YYSYMBOL_for = 264,                      /* for  */
  YYSYMBOL_265_8 = 265,                    /* $@8  */
  YYSYMBOL_266_9 = 266,                    /* $@9  */
  YYSYMBOL_267_10 = 267,                   /* @10  */
  YYSYMBOL_for_args = 268,                 /* for_args  */
  YYSYMBOL_break = 269,                    /* break  */
  YYSYMBOL_macrodef = 270,                 /* macrodef  */
  YYSYMBOL_271_11 = 271,                   /* $@11  */
  YYSYMBOL_272_12 = 272,                   /* $@12  */
  YYSYMBOL_273_13 = 273,                   /* @13  */
  YYSYMBOL_rsset = 274,                    /* rsset  */
  YYSYMBOL_rsreset = 275,                  /* rsreset  */
  YYSYMBOL_rs_uconst = 276,                /* rs_uconst  */
  YYSYMBOL_union = 277,                    /* union  */
  YYSYMBOL_nextu = 278,                    /* nextu  */
  YYSYMBOL_endu = 279,                     /* endu  */
  YYSYMBOL_ds = 280,                       /* ds  */
  YYSYMBOL_ds_args = 281,                  /* ds_args  */
  YYSYMBOL_db = 282,                       /* db  */
  YYSYMBOL_dw = 283,                       /* dw  */
  YYSYMBOL_dl = 284,                       /* dl  */
  YYSYMBOL_def_equ = 285,                  /* def_equ  */
  YYSYMBOL_redef_equ = 286,                /* redef_equ  */
  YYSYMBOL_def_set = 287,                  /* def_set  */
  YYSYMBOL_def_rb = 288,                   /* def_rb  */
  YYSYMBOL_def_rw = 289,                   /* def_rw  */
  YYSYMBOL_def_rl = 290,                   /* def_rl  */
  YYSYMBOL_def_equs = 291,                 /* def_equs  */
  YYSYMBOL_redef_equs = 292,               /* redef_equs  */
  YYSYMBOL_purge = 293,                    /* purge  */
  YYSYMBOL_294_14 = 294,                   /* $@14  */
  YYSYMBOL_purge_args = 295,               /* purge_args  */
  YYSYMBOL_export = 296,                   /* export  */
  YYSYMBOL_export_list = 297,              /* export_list  */
  YYSYMBOL_export_list_entry = 298,        /* export_list_entry  */
  YYSYMBOL_include = 299,                  /* include  */
  YYSYMBOL_incbin = 300,                   /* incbin  */
  YYSYMBOL_charmap = 301,                  /* charmap  */
  YYSYMBOL_newcharmap = 302,               /* newcharmap  */
  YYSYMBOL_setcharmap = 303,               /* setcharmap  */
  YYSYMBOL_pushc = 304,                    /* pushc  */
  YYSYMBOL_popc = 305,                     /* popc  */
  YYSYMBOL_print = 306,                    /* print  */
  YYSYMBOL_println = 307,                  /* println  */
  YYSYMBOL_print_exprs = 308,              /* print_exprs  */
  YYSYMBOL_print_expr = 309,               /* print_expr  */
  YYSYMBOL_const_3bit = 310,               /* const_3bit  */
  YYSYMBOL_constlist_8bit = 311,           /* constlist_8bit  */
  YYSYMBOL_constlist_8bit_entry = 312,     /* constlist_8bit_entry  */
  YYSYMBOL_constlist_16bit = 313,          /* constlist_16bit  */
  YYSYMBOL_constlist_16bit_entry = 314,    /* constlist_16bit_entry  */
  YYSYMBOL_constlist_32bit = 315,          /* constlist_32bit  */
  YYSYMBOL_constlist_32bit_entry = 316,    /* constlist_32bit_entry  */
  YYSYMBOL_reloc_8bit = 317,               /* reloc_8bit  */
  YYSYMBOL_reloc_8bit_no_str = 318,        /* reloc_8bit_no_str  */
  YYSYMBOL_reloc_8bit_offset = 319,        /* reloc_8bit_offset  */
  YYSYMBOL_reloc_16bit = 320,              /* reloc_16bit  */
  YYSYMBOL_reloc_16bit_no_str = 321,       /* reloc_16bit_no_str  */
  YYSYMBOL_relocexpr = 322,                /* relocexpr  */
  YYSYMBOL_relocexpr_no_str = 323,         /* relocexpr_no_str  */
  YYSYMBOL_324_15 = 324,                   /* $@15  */
  YYSYMBOL_uconst = 325,                   /* uconst  */
  YYSYMBOL_const = 326,                    /* const  */
  YYSYMBOL_const_no_str = 327,             /* const_no_str  */
  YYSYMBOL_const_8bit = 328,               /* const_8bit  */
  YYSYMBOL_opt_q_arg = 329,                /* opt_q_arg  */
  YYSYMBOL_string = 330,                   /* string  */
  YYSYMBOL_strcat_args = 331,              /* strcat_args  */
  YYSYMBOL_strfmt_args = 332,              /* strfmt_args  */
  YYSYMBOL_strfmt_va_args = 333,           /* strfmt_va_args  */
  YYSYMBOL_section = 334,                  /* section  */
  YYSYMBOL_sectmod = 335,                  /* sectmod  */
  YYSYMBOL_sectiontype = 336,              /* sectiontype  */
  YYSYMBOL_sectorg = 337,                  /* sectorg  */
  YYSYMBOL_sectattrs = 338,                /* sectattrs  */
  YYSYMBOL_cpu_commands = 339,             /* cpu_commands  */
  YYSYMBOL_cpu_command = 340,              /* cpu_command  */
  YYSYMBOL_z80_adc = 341,                  /* z80_adc  */
  YYSYMBOL_z80_add = 342,                  /* z80_add  */
  YYSYMBOL_z80_and = 343,                  /* z80_and  */
  YYSYMBOL_z80_bit = 344,                  /* z80_bit  */
  YYSYMBOL_z80_call = 345,                 /* z80_call  */
  YYSYMBOL_z80_ccf = 346,                  /* z80_ccf  */
  YYSYMBOL_z80_cp = 347,                   /* z80_cp  */
  YYSYMBOL_z80_cpl = 348,                  /* z80_cpl  */
  YYSYMBOL_z80_daa = 349,                  /* z80_daa  */
  YYSYMBOL_z80_dec = 350,                  /* z80_dec  */
  YYSYMBOL_z80_di = 351,                   /* z80_di  */
  YYSYMBOL_z80_ei = 352,                   /* z80_ei  */
  YYSYMBOL_z80_halt = 353,                 /* z80_halt  */
  YYSYMBOL_z80_inc = 354,                  /* z80_inc  */
  YYSYMBOL_z80_jp = 355,                   /* z80_jp  */
  YYSYMBOL_z80_jr = 356,                   /* z80_jr  */
  YYSYMBOL_z80_ldi = 357,                  /* z80_ldi  */
  YYSYMBOL_z80_ldd = 358,                  /* z80_ldd  */
  YYSYMBOL_z80_ldio = 359,                 /* z80_ldio  */
  YYSYMBOL_c_ind = 360,                    /* c_ind  */
  YYSYMBOL_z80_ld = 361,                   /* z80_ld  */
  YYSYMBOL_z80_ld_hl = 362,                /* z80_ld_hl  */
  YYSYMBOL_z80_ld_sp = 363,                /* z80_ld_sp  */
  YYSYMBOL_z80_ld_mem = 364,               /* z80_ld_mem  */
  YYSYMBOL_z80_ld_cind = 365,              /* z80_ld_cind  */
  YYSYMBOL_z80_ld_rr = 366,                /* z80_ld_rr  */
  YYSYMBOL_z80_ld_r = 367,                 /* z80_ld_r  */
  YYSYMBOL_z80_ld_a = 368,                 /* z80_ld_a  */
  YYSYMBOL_z80_ld_ss = 369,                /* z80_ld_ss  */
  YYSYMBOL_z80_nop = 370,                  /* z80_nop  */
  YYSYMBOL_z80_or = 371,                   /* z80_or  */
  YYSYMBOL_z80_pop = 372,                  /* z80_pop  */
  YYSYMBOL_z80_push = 373,                 /* z80_push  */
  YYSYMBOL_z80_res = 374,                  /* z80_res  */
  YYSYMBOL_z80_ret = 375,                  /* z80_ret  */
  YYSYMBOL_z80_reti = 376,                 /* z80_reti  */
  YYSYMBOL_z80_rl = 377,                   /* z80_rl  */
  YYSYMBOL_z80_rla = 378,                  /* z80_rla  */
  YYSYMBOL_z80_rlc = 379,                  /* z80_rlc  */
  YYSYMBOL_z80_rlca = 380,                 /* z80_rlca  */
  YYSYMBOL_z80_rr = 381,                   /* z80_rr  */
  YYSYMBOL_z80_rra = 382,                  /* z80_rra  */
  YYSYMBOL_z80_rrc = 383,                  /* z80_rrc  */
  YYSYMBOL_z80_rrca = 384,                 /* z80_rrca  */
  YYSYMBOL_z80_rst = 385,                  /* z80_rst  */
  YYSYMBOL_z80_sbc = 386,                  /* z80_sbc  */
  YYSYMBOL_z80_scf = 387,                  /* z80_scf  */
  YYSYMBOL_z80_set = 388,                  /* z80_set  */
  YYSYMBOL_z80_sla = 389,                  /* z80_sla  */
  YYSYMBOL_z80_sra = 390,                  /* z80_sra  */
  YYSYMBOL_z80_srl = 391,                  /* z80_srl  */
  YYSYMBOL_z80_stop = 392,                 /* z80_stop  */
  YYSYMBOL_z80_sub = 393,                  /* z80_sub  */
  YYSYMBOL_z80_swap = 394,                 /* z80_swap  */
  YYSYMBOL_z80_xor = 395,                  /* z80_xor  */
  YYSYMBOL_op_mem_ind = 396,               /* op_mem_ind  */
  YYSYMBOL_op_a_r = 397,                   /* op_a_r  */
  YYSYMBOL_op_a_n = 398,                   /* op_a_n  */
  YYSYMBOL_T_MODE_A = 399,                 /* T_MODE_A  */
  YYSYMBOL_T_MODE_B = 400,                 /* T_MODE_B  */
  YYSYMBOL_T_MODE_C = 401,                 /* T_MODE_C  */
  YYSYMBOL_T_MODE_D = 402,                 /* T_MODE_D  */
  YYSYMBOL_T_MODE_E = 403,                 /* T_MODE_E  */
  YYSYMBOL_T_MODE_H = 404,                 /* T_MODE_H  */
  YYSYMBOL_T_MODE_L = 405,                 /* T_MODE_L  */
  YYSYMBOL_ccode_expr = 406,               /* ccode_expr  */
  YYSYMBOL_ccode = 407,                    /* ccode  */
  YYSYMBOL_reg_r = 408,                    /* reg_r  */
  YYSYMBOL_reg_tt = 409,                   /* reg_tt  */
  YYSYMBOL_reg_ss = 410,                   /* reg_ss  */
  YYSYMBOL_reg_rr = 411,                   /* reg_rr  */
  YYSYMBOL_hl_ind_inc = 412,               /* hl_ind_inc  */
  YYSYMBOL_hl_ind_dec = 413                /* hl_ind_dec  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2631

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  211
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  203
/* YYNRULES -- Number of rules.  */
#define YYNRULES  524
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  946

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   465


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   718,   718,   721,   722,   725,   725,   728,   729,   732,
     737,   738,   740,   740,   748,   748,   766,   767,   768,   769,
     770,   771,   773,   774,   777,   787,   804,   820,   821,   822,
     823,   824,   827,   832,   832,   840,   840,   850,   850,   850,
     851,   851,   853,   854,   857,   860,   863,   866,   870,   876,
     876,   884,   887,   893,   894,   895,   896,   897,   898,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   925,   926,   927,   928,   929,   930,   931,
     932,   933,   934,   935,   936,   937,   938,   939,   940,   941,
     944,   944,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   956,   959,   965,   969,   991,   997,  1004,  1011,  1018,
    1021,  1030,  1046,  1046,  1052,  1053,  1056,  1059,  1062,  1065,
    1068,  1071,  1074,  1077,  1080,  1081,  1082,  1083,  1086,  1097,
    1108,  1112,  1118,  1119,  1122,  1125,  1128,  1128,  1137,  1139,
    1141,  1137,  1149,  1154,  1159,  1166,  1172,  1174,  1176,  1172,
    1185,  1188,  1191,  1192,  1195,  1198,  1201,  1204,  1205,  1209,
    1215,  1224,  1228,  1234,  1235,  1238,  1239,  1242,  1243,  1246,
    1249,  1252,  1253,  1254,  1255,  1258,  1264,  1270,  1276,  1279,
    1282,  1282,  1292,  1296,  1302,  1305,  1306,  1309,  1312,  1319,
    1324,  1329,  1336,  1341,  1342,  1345,  1348,  1351,  1354,  1357,
    1361,  1367,  1368,  1371,  1372,  1375,  1387,  1388,  1391,  1394,
    1403,  1404,  1407,  1410,  1419,  1420,  1423,  1426,  1436,  1442,
    1448,  1452,  1458,  1464,  1471,  1472,  1483,  1484,  1485,  1488,
    1491,  1494,  1497,  1500,  1503,  1506,  1509,  1512,  1515,  1518,
    1521,  1524,  1527,  1530,  1533,  1536,  1539,  1542,  1545,  1548,
    1549,  1550,  1551,  1552,  1553,  1554,  1558,  1559,  1560,  1561,
    1564,  1567,  1567,  1574,  1577,  1580,  1583,  1586,  1589,  1592,
    1595,  1598,  1601,  1604,  1607,  1610,  1613,  1616,  1619,  1622,
    1627,  1632,  1635,  1638,  1641,  1644,  1651,  1654,  1657,  1660,
    1661,  1671,  1672,  1678,  1684,  1690,  1693,  1696,  1699,  1702,
    1705,  1709,  1724,  1725,  1736,  1744,  1747,  1754,  1763,  1768,
    1769,  1770,  1773,  1774,  1775,  1776,  1777,  1778,  1779,  1780,
    1783,  1784,  1794,  1799,  1803,  1809,  1810,  1813,  1814,  1815,
    1816,  1817,  1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,
    1826,  1827,  1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,
    1836,  1837,  1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,
    1846,  1847,  1848,  1849,  1850,  1851,  1852,  1853,  1854,  1855,
    1856,  1857,  1858,  1861,  1865,  1868,  1872,  1873,  1874,  1881,
    1885,  1888,  1894,  1898,  1904,  1907,  1911,  1914,  1917,  1920,
    1921,  1924,  1927,  1930,  1943,  1944,  1947,  1951,  1955,  1960,
    1964,  1970,  1973,  1978,  1981,  1986,  1992,  1998,  2001,  2006,
    2007,  2013,  2014,  2015,  2016,  2017,  2018,  2019,  2020,  2023,
    2027,  2033,  2034,  2040,  2044,  2062,  2067,  2072,  2076,  2084,
    2090,  2096,  2119,  2123,  2131,  2134,  2138,  2141,  2144,  2147,
    2153,  2154,  2157,  2160,  2166,  2169,  2175,  2178,  2184,  2187,
    2193,  2196,  2206,  2210,  2213,  2216,  2222,  2228,  2234,  2240,
    2244,  2250,  2254,  2257,  2263,  2267,  2270,  2273,  2274,  2277,
    2278,  2281,  2282,  2285,  2286,  2289,  2290,  2293,  2294,  2297,
    2298,  2301,  2302,  2305,  2306,  2309,  2310,  2315,  2316,  2317,
    2318,  2321,  2322,  2323,  2324,  2325,  2326,  2327,  2328,  2331,
    2332,  2333,  2334,  2337,  2338,  2339,  2340,  2343,  2344,  2345,
    2346,  2349,  2350,  2353,  2354
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "\"number\"",
  "\"string\"", "\".\"", "\",\"", "\":\"", "\"::\"", "\"[\"", "\"]\"",
  "\"(\"", "\")\"", "\"newline\"", "\"!\"", "\"&&\"", "\"||\"", "\">\"",
  "\"<\"", "\">=\"", "\"<=\"", "\"!=\"", "\"==\"", "\"+\"", "\"-\"",
  "\"|\"", "\"^\"", "\"&\"", "\"<<\"", "\">>\"", "\">>>\"", "\"*\"",
  "\"/\"", "\"%\"", "\"~\"", "NEG", "\"**\"", "\"DEF\"", "\"BANK\"",
  "\"ALIGN\"", "\"SIZEOF\"", "\"STARTOF\"", "\"SIN\"", "\"COS\"",
  "\"TAN\"", "\"ASIN\"", "\"ACOS\"", "\"ATAN\"", "\"ATAN2\"", "\"FDIV\"",
  "\"FMUL\"", "\"FMOD\"", "\"POW\"", "\"LOG\"", "\"ROUND\"", "\"CEIL\"",
  "\"FLOOR\"", "\"HIGH\"", "\"LOW\"", "\"ISCONST\"", "\"STRCMP\"",
  "\"STRIN\"", "\"STRRIN\"", "\"STRSUB\"", "\"STRLEN\"", "\"STRCAT\"",
  "\"STRUPR\"", "\"STRLWR\"", "\"STRRPL\"", "\"STRFMT\"", "\"CHARLEN\"",
  "\"CHARSUB\"", "\"INCHARMAP\"", "\"label\"", "\"identifier\"",
  "\"local identifier\"", "\"anonymous label\"", "\"EQU\"", "\"=\"",
  "\"EQUS\"", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"", "\"%=\"", "\"|=\"",
  "\"^=\"", "\"&=\"", "\"<<=\"", "\">>=\"", "\"INCLUDE\"", "\"PRINT\"",
  "\"PRINTLN\"", "\"IF\"", "\"ELIF\"", "\"ELSE\"", "\"ENDC\"",
  "\"EXPORT\"", "\"DB\"", "\"DS\"", "\"DW\"", "\"DL\"", "\"SECTION\"",
  "\"FRAGMENT\"", "\"ENDSECTION\"", "\"RB\"", "\"RW\"", "\"MACRO\"",
  "\"ENDM\"", "\"RSRESET\"", "\"RSSET\"", "\"UNION\"", "\"NEXTU\"",
  "\"ENDU\"", "\"INCBIN\"", "\"REPT\"", "\"FOR\"", "\"CHARMAP\"",
  "\"NEWCHARMAP\"", "\"SETCHARMAP\"", "\"PUSHC\"", "\"POPC\"", "\"SHIFT\"",
  "\"ENDR\"", "\"BREAK\"", "\"LOAD\"", "\"ENDL\"", "\"FAIL\"", "\"WARN\"",
  "\"FATAL\"", "\"ASSERT\"", "\"STATIC_ASSERT\"", "\"PURGE\"", "\"REDEF\"",
  "\"POPS\"", "\"PUSHS\"", "\"POPO\"", "\"PUSHO\"", "\"OPT\"", "\"ROM0\"",
  "\"ROMX\"", "\"WRAM0\"", "\"WRAMX\"", "\"HRAM\"", "\"VRAM\"", "\"SRAM\"",
  "\"OAM\"", "\"adc\"", "\"add\"", "\"and\"", "\"bit\"", "\"call\"",
  "\"ccf\"", "\"cp\"", "\"cpl\"", "\"daa\"", "\"dec\"", "\"di\"", "\"ei\"",
  "\"halt\"", "\"inc\"", "\"jp\"", "\"jr\"", "\"ld\"", "\"ldi\"",
  "\"ldd\"", "\"ldh\"", "\"nop\"", "\"or\"", "\"pop\"", "\"push\"",
  "\"res\"", "\"ret\"", "\"reti\"", "\"rst\"", "\"rl\"", "\"rla\"",
  "\"rlc\"", "\"rlca\"", "\"rr\"", "\"rra\"", "\"rrc\"", "\"rrca\"",
  "\"sbc\"", "\"scf\"", "\"set\"", "\"stop\"", "\"sla\"", "\"sra\"",
  "\"srl\"", "\"sub\"", "\"swap\"", "\"xor\"", "\"a\"", "\"b\"", "\"c\"",
  "\"d\"", "\"e\"", "\"h\"", "\"l\"", "\"af\"", "\"bc\"", "\"de\"",
  "\"sp\"", "\"hl\"", "\"hld/hl-\"", "\"hli/hl+\"", "\"nz\"", "\"z\"",
  "\"nc\"", "\"end of buffer\"", "$accept", "asmfile", "lines",
  "endofline", "opt_diff_mark", "line", "$@1", "$@2", "line_directive",
  "if", "elif", "else", "plain_directive", "endc", "def_id", "$@3",
  "redef_id", "$@4", "scoped_id", "scoped_anon_id", "label", "macro",
  "$@5", "macroargs", "assignment_directive", "directive",
  "trailing_comma", "compoundeq", "equ", "assignment", "equs", "rb", "rw",
  "rl", "align", "align_spec", "opt", "$@6", "opt_list", "opt_list_entry",
  "popo", "pusho", "pops", "pushs", "endsection", "fail", "warn",
  "assert_type", "assert", "shift", "load", "rept", "@7", "for", "$@8",
  "$@9", "@10", "for_args", "break", "macrodef", "$@11", "$@12", "@13",
  "rsset", "rsreset", "rs_uconst", "union", "nextu", "endu", "ds",
  "ds_args", "db", "dw", "dl", "def_equ", "redef_equ", "def_set", "def_rb",
  "def_rw", "def_rl", "def_equs", "redef_equs", "purge", "$@14",
  "purge_args", "export", "export_list", "export_list_entry", "include",
  "incbin", "charmap", "newcharmap", "setcharmap", "pushc", "popc",
  "print", "println", "print_exprs", "print_expr", "const_3bit",
  "constlist_8bit", "constlist_8bit_entry", "constlist_16bit",
  "constlist_16bit_entry", "constlist_32bit", "constlist_32bit_entry",
  "reloc_8bit", "reloc_8bit_no_str", "reloc_8bit_offset", "reloc_16bit",
  "reloc_16bit_no_str", "relocexpr", "relocexpr_no_str", "$@15", "uconst",
  "const", "const_no_str", "const_8bit", "opt_q_arg", "string",
  "strcat_args", "strfmt_args", "strfmt_va_args", "section", "sectmod",
  "sectiontype", "sectorg", "sectattrs", "cpu_commands", "cpu_command",
  "z80_adc", "z80_add", "z80_and", "z80_bit", "z80_call", "z80_ccf",
  "z80_cp", "z80_cpl", "z80_daa", "z80_dec", "z80_di", "z80_ei",
  "z80_halt", "z80_inc", "z80_jp", "z80_jr", "z80_ldi", "z80_ldd",
  "z80_ldio", "c_ind", "z80_ld", "z80_ld_hl", "z80_ld_sp", "z80_ld_mem",
  "z80_ld_cind", "z80_ld_rr", "z80_ld_r", "z80_ld_a", "z80_ld_ss",
  "z80_nop", "z80_or", "z80_pop", "z80_push", "z80_res", "z80_ret",
  "z80_reti", "z80_rl", "z80_rla", "z80_rlc", "z80_rlca", "z80_rr",
  "z80_rra", "z80_rrc", "z80_rrca", "z80_rst", "z80_sbc", "z80_scf",
  "z80_set", "z80_sla", "z80_sra", "z80_srl", "z80_stop", "z80_sub",
  "z80_swap", "z80_xor", "op_mem_ind", "op_a_r", "op_a_n", "T_MODE_A",
  "T_MODE_B", "T_MODE_C", "T_MODE_D", "T_MODE_E", "T_MODE_H", "T_MODE_L",
  "ccode_expr", "ccode", "reg_r", "reg_tt", "reg_ss", "reg_rr",
  "hl_ind_inc", "hl_ind_dec", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-631)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-318)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -631,    43,    45,  -631,  -631,  -631,  1114,  -631,  -631,   526,
     106,  2483,  2483,     6,  -631,  2483,  -631,  -631,  -631,  -631,
    -631,  -631,     2,  2219,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,     2,  -631,  -631,  -631,
    2483,  2483,   332,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  2483,  2483,  2483,  2483,  -631,  -631,  -631,
    -631,  2483,  2483,  2483,  2483,  2483,  -631,    65,    92,    97,
     126,   134,   146,   148,   160,   165,   166,   168,   174,   176,
     184,   186,   197,   214,   217,   219,   223,   224,   233,   235,
     236,   238,   239,   247,   249,   252,   254,   256,   258,   261,
     266,  -631,  -631,  -631,  -631,   271,  -631,  -631,  1113,  -631,
     283,  -631,   288,  -631,    18,   289,  -631,    31,  -631,  -631,
    -631,  -631,  2483,  -631,   332,  2483,  2483,  -631,   130,  2483,
    2409,  2483,  2483,   -17,  -631,  -631,  2483,  -631,  -631,  -631,
     332,   332,   229,   230,  -631,  -631,  2483,     2,   -17,  -631,
     332,   332,    34,    34,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  1993,  1483,  1993,  2483,  1304,  -631,  1993,  -631,  -631,
      91,  -631,  -631,  -631,    91,   973,  1304,   149,    71,    75,
      81,  -631,  1993,     9,     9,  2483,     7,  -631,  2483,    44,
    -631,    44,  -631,    44,  -631,    44,  -631,  1993,  -631,  2483,
    2483,    44,    44,    44,  1993,    44,  1993,  -631,   331,   776,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,   297,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,     2,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,   813,   270,
     270,   270,   270,   286,   125,   660,   660,  2483,  2483,  2483,
    2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,
    2483,  2483,  2483,  2483,  2483,   332,   332,   332,   332,   332,
     115,   332,   332,   332,   332,   332,   332,   332,    77,  2483,
    2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,
    2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  2483,  -631,
    -631,  -631,  -631,  -631,   240,  -631,   301,  -631,     2,   303,
    -631,  1113,    38,  -631,    14,   303,  -631,   304,  -631,   306,
    -631,  -631,    19,    23,   307,   309,   326,  -631,  -631,    49,
      52,   328,  -631,    60,    61,  -631,  -631,   332,  -631,   329,
     343,   348,  -631,  -631,  -631,   332,  -631,  -631,   349,   350,
     352,  2483,  2483,   130,   263,   334,   109,   362,   363,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  1113,  -631,  -631,
     353,  -631,  -631,  -631,  -631,  -631,  -631,  -631,   369,   370,
    -631,  -631,  -631,  -631,   371,  -631,  1304,  -631,  -631,  -631,
    -631,  -631,  1113,   377,  -631,  -631,  -631,   373,   374,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
     380,  -631,   381,  1378,   382,   390,   396,   398,   399,   400,
     415,   425,  -631,  -631,   231,   422,   432,   237,   433,  2170,
     438,   439,   445,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,   446,     7,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,   447,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  2483,  2483,   332,  2483,  2483,  2483,  2483,  2483,  2483,
     332,  2483,  2439,  -631,  -631,    77,   442,   463,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,   464,   469,   471,   473,
     480,   480,   480,   480,   480,   480,   481,   482,   483,   484,
     485,   488,   480,   480,   480,   912,  1038,  1061,   489,   491,
     492,   494,   490,  -631,  -631,    35,   495,   496,   497,  -631,
     499,   500,   498,   503,   504,  1794,  1625,   746,   746,   746,
     746,   746,   746,   612,   612,   190,   190,   190,   136,   136,
     136,   270,   270,   270,  -631,   506,     2,   519,  -631,  2483,
     505,  -631,  2483,  -631,  -631,   130,  -631,  2483,  -631,  2483,
    2483,  2483,  -631,  2483,  -631,   520,  2483,  2483,   454,   524,
    -631,  -631,  -631,   441,   530,  -631,   534,  -631,  -631,   334,
    -631,   536,  1563,  1660,  1993,  2483,    51,    44,  -631,  2483,
      79,   -84,  2483,  2483,   537,   544,   545,    88,   547,   549,
     553,  1440,   554,  2483,  2483,  1734,  1829,   -29,   -33,  2068,
     -29,   556,   347,   558,   560,   559,   -29,   -29,   562,    44,
      44,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,   565,  -631,  -631,  -631,  -631,  -631,  -631,
    2483,   566,   569,   570,   571,   572,   575,  2483,  2483,  2483,
    2483,  2483,  2483,   576,   577,   582,  -631,  -631,  -631,   332,
     332,   332,  2483,  -631,   332,  -631,  -631,  -631,   332,   590,
    -631,  -631,  2483,  -631,  -631,  -631,  -631,  2483,  -631,  -631,
    -631,  -631,  -631,   587,   593,  -631,  -631,  -631,   284,   594,
    -631,  -631,  -631,   284,   332,   332,   130,  -631,  -631,  -631,
     589,   604,   605,   606,   608,   609,   610,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  1903,  -631,  -631,   592,   613,
    -631,  -631,  -631,  2170,  -631,  -631,  -631,  -631,  -631,   107,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
     619,   368,   620,   423,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,   480,   480,
     480,   480,   480,   480,  -631,  -631,  -631,   616,   617,   618,
      87,  -631,   627,  2483,   622,     2,   633,   629,   643,  2483,
    -631,   641,  2483,   641,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,   644,  2483,  2483,  -631,
     -29,   645,   -29,   647,   650,   651,   653,   654,   655,   656,
    -631,  -631,  -631,  2483,  -631,   332,  -631,    98,  -631,  -631,
    -631,  2483,  2483,  -631,  -631,  2483,  -631,  -631,  -631,  -631,
    1113,  1113,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,   657,   658,     2,   665,   593,   662,   667,   667,
    -631,  -631,  -631,  2483,  -631,  -631,   104,  -631,   666,   687,
    2483,  2483,   664,   688,  -631,  -631
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       3,     0,     7,     1,     8,     9,     0,    12,    43,     0,
      44,     0,     0,     0,   156,     0,   148,     4,    11,    21,
      22,    23,     0,    27,    31,    53,    54,    58,    55,    56,
      57,    17,    18,    19,    16,    20,     0,    14,    46,    48,
       0,     0,     0,   102,   103,   104,   105,   106,   108,   107,
     109,   110,   111,   162,   162,   162,     0,    45,    47,   237,
     301,     0,     0,     0,     0,     0,   271,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,    37,    38,    41,     0,    40,   236,   296,   234,
       0,   235,     0,    26,     0,     0,   295,     0,     5,     6,
      10,    33,     0,    49,     0,     0,   209,    32,     0,   173,
       0,   175,   177,   319,   131,   161,     0,   164,   165,   166,
       0,     0,     0,     0,   206,   207,   142,     0,   319,   145,
       0,     0,   134,   134,   190,    35,   129,   130,   127,   128,
     122,     0,     0,     0,     0,     0,   394,     0,   397,   398,
       0,   401,   402,   403,     0,     0,     0,     0,     0,     0,
       0,   444,     0,     0,     0,     0,   450,   452,     0,     0,
     454,     0,   456,     0,   458,     0,   460,     0,   464,     0,
     469,     0,     0,     0,     0,     0,     0,    59,     0,     0,
      29,    30,    99,    98,    96,    97,    93,    94,    95,    81,
      82,    83,    80,    79,    69,    68,    70,    71,    72,    66,
      63,    64,    65,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    62,    73,    74,    75,    76,    77,    78,    60,
      61,    67,    28,   335,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     355,   354,   356,   353,   425,   426,   421,   422,   423,   427,
     428,   424,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,    13,     0,
     112,   113,   115,   116,   163,   117,   118,   114,     0,   238,
     259,   260,   261,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,   157,   146,   149,     0,   119,   120,    51,     0,   100,
     211,     0,   234,   213,   235,   100,   197,   100,   195,   100,
     216,   218,   234,   235,     0,   167,   100,   220,   222,   234,
     235,   100,   224,   234,   235,   321,   320,     0,   160,   199,
       0,   203,   205,   143,   155,     0,   132,   133,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   481,
     483,   485,   487,   489,   491,   493,   479,   228,   384,   383,
     508,   501,   502,   503,   504,   505,   506,   477,     0,     0,
     386,   385,   390,   389,     0,   215,     0,   500,   497,   498,
     499,   392,   232,     0,   495,   396,   395,     0,     0,   513,
     514,   516,   515,   508,   399,   400,   404,   405,   408,   406,
       0,   409,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   519,   520,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   446,   445,   512,   509,   510,   511,   447,
     448,     0,     0,   451,   461,   453,   455,   457,   459,   463,
     462,     0,   470,   466,   467,   468,   472,   471,   473,   475,
     474,     0,     0,     0,   162,   162,   162,     0,     0,     0,
       0,     0,     0,    15,   294,     0,     0,     0,   325,   324,
     322,   327,   326,   323,   328,   329,     0,     0,     0,     0,
     299,   299,   299,   299,   299,   299,     0,     0,     0,     0,
       0,     0,   299,   299,   299,     0,     0,     0,     0,     0,
       0,     0,     0,   305,   312,     0,     0,     0,     0,   315,
       0,     0,     0,     0,     0,   240,   239,   242,   243,   244,
     245,   246,   241,   247,   248,   250,   249,   251,   252,   253,
     254,   255,   256,   257,   258,     0,     0,     0,    34,     0,
      50,   198,   101,   208,   210,   101,   194,   101,   174,     0,
       0,   101,   176,   101,   178,     0,     0,     0,     0,     0,
     136,   135,   137,   138,   140,   192,   100,    36,   126,   123,
     124,     0,     0,     0,     0,     0,     0,     0,   496,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   232,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   179,   181,   188,   185,   186,   187,   183,   180,   182,
     189,   184,   336,     0,   265,   266,   267,   269,   268,   270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   262,   263,   264,     0,
       0,     0,     0,   291,     0,   306,   307,   308,     0,   314,
     310,   292,     0,   293,   311,   158,   147,     0,   121,    52,
     212,   196,   217,     0,   100,   171,   221,   225,     0,   200,
     298,   202,   204,     0,     0,     0,   101,   191,   125,   507,
       0,     0,     0,     0,     0,     0,     0,   480,   478,   388,
     387,   391,   393,   407,   410,     0,   517,   518,     0,     0,
     523,   521,   476,     0,   419,   442,   443,   431,   432,     0,
     430,   435,   433,   434,   437,   439,   441,   438,   440,   436,
       0,     0,     0,     0,   418,   416,   417,   415,   449,   465,
     272,   300,   281,   282,   283,   284,   285,   286,   299,   299,
     299,   299,   299,   299,   273,   274,   275,     0,     0,     0,
       0,   313,     0,     0,     0,     0,     0,   152,   100,   101,
     168,   330,     0,   330,   139,   141,   193,   482,   484,   488,
     492,   486,   490,   494,   522,   524,     0,     0,     0,   429,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     288,   289,   290,     0,   303,     0,   316,   235,   304,   159,
     150,     0,   101,   169,   172,     0,   332,   201,   332,   420,
     230,   231,   411,   412,   413,   414,   287,   276,   277,   278,
     279,   280,     0,     0,     0,   153,   100,     0,   318,   144,
     302,   309,   151,     0,   170,   331,     0,   154,     0,     0,
       0,     0,     0,     0,   334,   333
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -631,  -631,  -631,   -31,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -125,  -310,
    -631,  -631,  -631,  -631,  -631,  -631,  -357,   -35,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -630,  -631,  -631,  -631,    -6,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,   546,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,    16,  -631,  -631,  -631,  -631,
    -205,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,    67,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,   574,    72,  -121,
    -631,    66,  -631,    64,  -631,    63,   -91,  -631,  -631,   -93,
    -631,   317,  -119,  -631,   -15,   -10,  -146,  -631,  -449,    15,
    -631,  -631,  -631,  -631,   561,  -318,  -155,  -197,   161,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -169,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,  -631,
    -631,  -631,  -631,  -631,  -631,  -154,    69,    94,  -145,  -631,
    -470,  -631,  -631,  -631,  -631,   -80,  -631,  -114,   528,  -170,
      25,  -631,  -631
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,   120,     6,    17,    36,   309,    18,    19,
      20,    21,    22,   207,   208,   384,   209,   434,   106,   107,
      23,   210,   387,   630,    24,   211,   633,    56,    25,    26,
      27,    28,    29,    30,   212,   385,   213,   435,   659,   660,
     214,   215,   216,   217,   218,   219,   220,   431,   221,   222,
     223,    31,   626,    32,   117,   627,   924,   856,    33,    34,
     114,   625,   855,   224,   225,   313,   226,   227,   228,   229,
     764,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   433,   656,   242,   397,   398,    35,   243,
     244,   245,   246,   247,   248,   249,   250,   389,   390,   464,
     399,   400,   406,   407,   411,   412,   446,   401,   879,   680,
     408,   108,   109,   323,   314,   116,   393,   771,   721,   111,
     595,   600,   749,   251,   417,   567,   906,   928,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   498,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   499,   448,   449,   483,   451,
     452,   453,   454,   455,   456,   668,   474,   457,   519,   485,
     501,   502,   503
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     115,   110,   112,   396,   487,   308,   392,   392,   569,   763,
     402,   510,   409,   413,   556,   118,   450,   450,   450,   113,
    -214,   522,   450,   682,   505,  -229,   511,  -214,   505,  -219,
     310,   311,  -229,   506,   508,   512,  -219,   450,   634,   682,
     636,   744,   638,     3,  -297,    -2,   317,   745,   604,   642,
    -297,  -297,   450,   436,   644,  -233,   484,   312,  -223,   450,
     486,   450,  -233,   500,   521,  -223,  -226,  -227,     4,     5,
     315,   316,   471,  -226,  -227,   525,   324,   526,   531,   527,
     504,   528,   489,   491,   507,   473,   415,   533,   534,   535,
     509,   538,   381,   893,   416,   490,   492,   524,   779,   894,
     436,   477,   478,   325,  -317,   383,   523,   386,   326,   532,
    -317,   798,   799,    57,    58,   405,   424,   784,   785,    60,
     786,   418,   722,   723,   724,   725,   726,   593,   505,    60,
     877,   878,   505,   733,   734,   735,   423,   327,   505,   388,
     394,   394,   938,   939,   403,   328,   410,   414,   477,   478,
     101,   102,   103,   104,   465,   419,   420,   329,   493,   330,
     439,   428,   429,   430,   439,   426,   427,   375,   376,   377,
     812,   331,   378,   547,   551,   465,   332,   333,    91,   334,
      93,    94,    95,    96,    97,   335,    99,   336,    91,   465,
      93,    94,    95,    96,    97,   337,    99,   338,   101,   102,
     103,   104,   467,   101,   102,   103,   477,   478,   339,   515,
     516,   517,   119,   518,   468,   469,   470,   105,   372,   373,
     374,   375,   376,   377,  -214,   340,   378,   105,   341,  -229,
     342,   460,   462,  -219,   343,   344,   475,   439,   440,   441,
     442,   443,   444,   445,   345,   713,   346,   347,  -297,   348,
     349,   513,   479,   480,   481,   482,   461,   463,   350,  -233,
     351,   476,  -223,   352,   439,   353,   529,   354,   439,   355,
    -226,  -227,   356,   536,   439,   539,   514,   357,   553,   780,
     781,   782,   358,   783,   439,   440,   441,   442,   443,   444,
     445,   530,   479,   480,   481,   482,   379,   555,   537,   777,
     540,   380,   382,   421,   422,   552,   378,   629,   655,   632,
     635,   943,   637,   661,   628,   640,   639,   570,   571,   572,
     573,   574,   575,   576,   577,   578,   579,   580,   581,   582,
     583,   584,   641,   876,   643,   646,    60,   657,   658,   557,
     566,   568,   439,   440,   441,   442,   443,   444,   445,   647,
     494,   495,   496,   497,   648,   650,   651,   631,   652,   664,
     588,   589,   590,   591,   592,   594,   596,   597,   598,   599,
     601,   602,   603,   662,   663,   665,   666,   667,   318,   319,
     320,   321,   322,   669,   670,   671,   672,   673,   683,   884,
     885,   886,   887,   888,   889,    91,   684,    93,    94,    95,
      96,    97,   685,    99,   686,   687,   688,   860,   541,   542,
     543,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,   689,   654,   558,   559,   560,   561,   562,   563,   564,
     565,   690,   645,   692,   105,   691,   544,   545,   693,   695,
     649,   694,   391,   391,   696,   697,   391,   774,   391,   391,
     861,   698,   699,   700,   714,   863,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   715,   716,   378,   447,   447,
     447,   717,   472,   718,   447,   719,   720,   727,   728,   729,
     730,   731,   472,   472,   732,   739,   790,   740,   741,   447,
     742,   903,   743,   748,   752,   447,   546,   746,   747,   759,
     396,   750,   751,   392,   447,   753,   754,   447,   402,   755,
     815,   447,   409,   447,   413,   757,   768,    37,   772,   826,
     773,   701,   702,    38,    39,   816,   775,   707,   708,   709,
     776,   711,   811,   813,   827,   819,   779,   780,   795,   765,
     788,   824,   825,   791,   796,   797,   770,   800,   703,   801,
     704,   705,   706,   802,   804,   710,   820,   821,   823,   934,
     822,   509,   881,   787,   789,   817,   792,   830,   832,   793,
     794,   833,   834,   835,   836,   828,   829,   837,   844,   845,
     805,   806,   808,   810,   846,   756,   853,   858,   814,   859,
     862,   867,   874,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,   868,   869,   870,   758,
     871,   872,   873,   875,   386,   880,   882,   883,   890,   891,
     892,    53,    54,   895,   898,   901,   769,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   900,   394,   378,   902,
     905,   866,   403,   778,   909,   913,   410,   915,   414,   585,
     586,   587,   916,   917,    60,   918,   919,   920,   921,   930,
     931,   933,   935,   936,   944,   940,   605,   606,   607,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   617,   618,
     619,   620,   621,   622,   623,   624,   941,   926,   945,   432,
     395,    55,   761,   762,   760,   766,   767,   896,   908,   425,
     831,   929,   520,   712,   818,     0,     0,   838,   839,   840,
     841,   842,   843,    91,     0,    93,    94,    95,    96,    97,
       0,    99,   850,     0,   392,   912,     0,   914,     0,     0,
       0,     0,   854,     0,     0,     0,     0,   857,   653,     0,
       0,     0,     0,     0,   847,   848,   849,     0,     0,   851,
       0,     0,   105,   852,     0,     0,     0,     0,   904,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
       0,     0,   378,   319,     0,     0,     0,     0,     0,   864,
     865,     0,     0,     0,     0,     0,     0,     0,     0,   558,
     559,   560,   561,   562,   563,   564,   565,     0,     0,     0,
     681,   765,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   899,   554,   681,     0,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,     0,   378,
       0,     0,   907,   548,   549,   550,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,     0,     0,   897,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   922,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     927,   925,     0,   932,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     923,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   937,   736,   942,   386,   359,   360,   361,
     362,   363,   364,   365,   366,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,     0,     0,   378,   391,
       0,     0,     0,     0,   391,     0,     0,   447,   391,     0,
     391,     0,     0,     0,   447,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,     0,   585,
     586,   447,   447,     0,    61,     0,   472,   466,     0,   472,
     472,     0,     0,     0,     0,     0,    63,    64,     0,     0,
     472,   472,   472,   472,     0,     0,   447,    65,     0,     0,
      66,    67,     0,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     737,     0,     0,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,     0,   738,   378,   105,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,     0,     0,   378,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   586,     0,     0,     7,     0,     0,     0,     0,
     613,     8,     0,     0,     0,     0,     0,   -42,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,     0,     0,   378,
       0,   -42,     0,   -42,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   467,     0,
     391,     0,     0,     0,     0,     0,   447,   488,     0,     0,
     468,   469,   470,     0,     0,     0,     0,     9,   -42,    10,
       0,     0,     0,     0,   910,   911,     0,     0,     0,     0,
       0,     0,     0,     0,   -42,   -42,   -42,    11,    12,    13,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,     0,   -42,   447,
       0,    14,     0,   -42,   -42,   -42,   -42,   -42,   -42,    15,
      16,   -42,   -42,   -42,   -42,   -42,   -42,     0,   -42,   -42,
     -42,   -42,   -42,     0,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,     0,     0,     0,     0,     0,     0,     0,
       0,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,    59,    60,     0,
       0,     0,     0,     0,     0,    61,     0,     0,   466,     0,
       0,     0,     0,     0,   -42,     0,     0,    63,    64,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    65,     0,
       0,    66,    67,     0,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    59,    60,     0,     0,     0,     0,     0,     0,    61,
       0,     0,    62,     0,     0,     0,     0,     0,     0,     0,
       0,    63,    64,     0,     0,     0,   105,     0,     0,     0,
       0,     0,    65,     0,     0,    66,    67,     0,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,   674,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   359,   360,   361,   362,   363,
     364,   365,   366,   803,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,     0,     0,   378,     0,     0,     0,
     105,     0,     0,     0,     0,     0,    59,    60,     0,     0,
       0,     0,   436,     0,    61,     0,     0,    62,     0,   467,
       0,     0,     0,     0,     0,     0,    63,    64,     0,     0,
       0,   468,   469,   470,     0,     0,     0,    65,     0,     0,
      66,    67,     0,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
     437,   438,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,    59,    60,     0,     0,
       0,     0,     0,   441,    61,     0,     0,    62,     0,   675,
     676,     0,   677,   678,   679,   105,    63,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,     0,     0,
      66,    67,     0,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     359,     0,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,     0,
       0,   378,     0,    59,    60,   105,     0,     0,     0,     0,
       0,    61,     0,     0,    62,     0,   439,   440,   441,   442,
     443,   444,   445,    63,    64,     0,   458,   459,     0,     0,
       0,     0,     0,     0,    65,     0,     0,    66,    67,     0,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,    59,    60,     0,
       0,     0,     0,     0,     0,    61,     0,     0,    62,     0,
       0,     0,     0,     0,     0,     0,     0,    63,    64,     0,
       0,     0,   105,   780,   781,   782,     0,   783,    65,     0,
       0,    66,    67,     0,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   361,   362,   363,   364,   365,   366,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,     0,     0,
     378,     0,    59,    60,     0,     0,   105,     0,     0,     0,
      61,     0,     0,    62,     0,     0,     0,     0,     0,     0,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,   784,   785,    65,   786,     0,    66,    67,     0,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    59,    60,     0,     0,
       0,     0,     0,     0,    61,     0,     0,    62,     0,     0,
       0,     0,     0,     0,     0,     0,    63,    64,     0,     0,
       0,   105,     0,     0,     0,     0,     0,    65,   807,     0,
      66,    67,     0,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    59,    60,     0,     0,
       0,     0,   436,     0,    61,   105,     0,    62,     0,     0,
       0,     0,     0,     0,     0,     0,    63,    64,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,     0,     0,
      66,    67,   809,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
     437,   438,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,    59,    60,     0,     0,     0,     0,   493,     0,    61,
       0,     0,    62,     0,     0,     0,     0,     0,     0,     0,
       0,    63,    64,     0,     0,   105,     0,     0,     0,     0,
       0,     0,    65,     0,   784,    66,    67,     0,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,   437,   438,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,    61,     0,     0,    62,     0,   439,   440,   441,   442,
     443,   444,   445,    63,    64,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,    66,    67,     0,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,   674,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   121,     0,   122,     0,
       0,   439,   440,   441,   442,   443,   444,   445,     0,     0,
       0,     0,   105,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   123,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   124,
     125,   126,     0,     0,     0,   127,   128,   129,   130,   131,
     132,   133,     0,   134,     0,     0,     0,     0,   135,   136,
     137,   138,   139,   140,     0,     0,   141,   142,   143,   144,
     145,   146,     0,   147,   148,   149,   150,   151,     0,   152,
     153,   154,   155,   156,   157,   158,   159,   160,     0,     0,
       0,     0,     0,     0,     0,   441,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,    59,    60,     0,     0,     0,     0,     0,     0,
      61,     0,     0,    62,     0,     0,     0,     0,     0,     0,
       0,     0,    63,    64,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    65,     0,     0,    66,    67,   404,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    59,    60,     0,     0,
       0,     0,     0,     0,    61,     0,     0,    62,     0,     0,
       0,     0,     0,     0,     0,     0,    63,    64,     0,     0,
       0,   105,     0,     0,     0,     0,     0,    65,     0,     0,
      66,    67,     0,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206
};

static const yytype_int16 yycheck[] =
{
      15,    11,    12,   128,   174,    36,   125,   126,   326,   639,
     129,   180,   131,   132,   324,    13,   161,   162,   163,    13,
       6,    14,   167,   493,    57,     6,   180,    13,    57,     6,
      40,    41,    13,   178,   179,   180,    13,   182,   395,   509,
     397,     6,   399,     0,     6,     0,    56,    12,   358,   406,
      12,    13,   197,     9,   411,     6,   170,    42,     6,   204,
     174,   206,    13,   177,   185,    13,     6,     6,    23,    24,
      54,    55,   165,    13,    13,   189,    11,   191,   199,   193,
       9,   195,   175,   176,     9,   165,   103,   201,   202,   203,
       9,   205,    74,     6,   111,   175,   176,   188,    10,    12,
       9,    57,    58,    11,     6,    74,   186,   122,    11,   200,
      12,    23,    24,     7,     8,   130,   147,   201,   202,     4,
     204,   136,   571,   572,   573,   574,   575,    12,    57,     4,
      23,    24,    57,   582,   583,   584,   146,    11,    57,   124,
     125,   126,    38,    39,   129,    11,   131,   132,    57,    58,
      73,    74,    75,    76,   164,   140,   141,    11,     9,    11,
     193,   127,   128,   129,   193,   150,   151,    31,    32,    33,
     203,    11,    36,   208,   209,   185,    11,    11,    63,    11,
      65,    66,    67,    68,    69,    11,    71,    11,    63,   199,
      65,    66,    67,    68,    69,    11,    71,    11,    73,    74,
      75,    76,   195,    73,    74,    75,    57,    58,    11,   200,
     201,   202,   210,   204,   207,   208,   209,   102,    28,    29,
      30,    31,    32,    33,   210,    11,    36,   102,    11,   210,
      11,   162,   163,   210,    11,    11,   167,   193,   194,   195,
     196,   197,   198,   199,    11,   555,    11,    11,   210,    11,
      11,   182,   201,   202,   203,   204,   162,   163,    11,   210,
      11,   167,   210,    11,   193,    11,   197,    11,   193,    11,
     210,   210,    11,   204,   193,   206,   182,    11,   309,   200,
     201,   202,    11,   204,   193,   194,   195,   196,   197,   198,
     199,   197,   201,   202,   203,   204,    13,    11,   204,   656,
     206,    13,    13,    74,    74,     8,    36,     6,   433,     6,
       6,   941,     6,   204,    74,     6,     9,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,     6,   803,     6,     6,     4,    74,     4,   324,
     325,   326,   193,   194,   195,   196,   197,   198,   199,     6,
     201,   202,   203,   204,     6,     6,     6,   388,     6,     6,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,    11,    11,     6,     6,     6,    61,    62,
      63,    64,    65,     6,    11,    11,     6,     6,     6,   838,
     839,   840,   841,   842,   843,    63,     6,    65,    66,    67,
      68,    69,     6,    71,     6,     6,     6,   764,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,     6,   432,   139,   140,   141,   142,   143,   144,   145,
     146,     6,   417,    11,   102,   204,   105,   106,     6,     6,
     425,   204,   125,   126,     6,     6,   129,     6,   131,   132,
     768,     6,     6,     6,    12,   773,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    12,    12,    36,   161,   162,
     163,    12,   165,    12,   167,    12,     6,     6,     6,     6,
       6,     6,   175,   176,     6,     6,   666,     6,     6,   182,
       6,   858,    12,     6,     6,   188,   175,    12,    12,     4,
     635,    12,    12,   632,   197,    12,    12,   200,   637,    13,
     689,   204,   641,   206,   643,     6,     6,     1,    74,   698,
       6,   541,   542,     7,     8,   689,     6,   547,   548,   549,
       6,   551,   687,   688,   698,   690,    10,   200,    11,   640,
     664,   696,   697,   667,    10,    10,   647,    10,   543,    10,
     544,   545,   546,    10,    10,   550,    10,     9,     9,   926,
      10,     9,   204,   664,   665,   689,   669,    12,    12,   672,
     673,    12,    12,    12,    12,   699,   700,    12,    12,    12,
     683,   684,   685,   686,    12,   626,     6,    10,   689,     6,
       6,    12,    10,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    12,    12,    12,   629,
      12,    12,    12,    10,   639,     6,     6,   204,    12,    12,
      12,   105,   106,     6,    12,     6,   646,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    13,   632,    36,     6,
       9,   776,   637,   659,    10,    10,   641,    10,   643,   342,
     343,   344,    12,    12,     4,    12,    12,    12,    12,    12,
      12,     6,    10,     6,    10,     9,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,     9,   902,    10,   153,
     126,   175,   635,   637,   632,   641,   643,   853,   863,   148,
     720,   908,   184,   552,   689,    -1,    -1,   727,   728,   729,
     730,   731,   732,    63,    -1,    65,    66,    67,    68,    69,
      -1,    71,   742,    -1,   853,   880,    -1,   882,    -1,    -1,
      -1,    -1,   752,    -1,    -1,    -1,    -1,   757,   431,    -1,
      -1,    -1,    -1,    -1,   739,   740,   741,    -1,    -1,   744,
      -1,    -1,   102,   748,    -1,    -1,    -1,    -1,   859,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    -1,    36,   466,    -1,    -1,    -1,    -1,    -1,   774,
     775,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
     140,   141,   142,   143,   144,   145,   146,    -1,    -1,    -1,
     493,   902,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   855,    12,   509,    -1,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    -1,    36,
      -1,    -1,   862,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    -1,    -1,   853,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   893,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     905,   901,    -1,   924,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     895,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   933,    12,   940,   941,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    36,   632,
      -1,    -1,    -1,    -1,   637,    -1,    -1,   640,   641,    -1,
     643,    -1,    -1,    -1,   647,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,   662,
     663,   664,   665,    -1,    11,    -1,   669,    14,    -1,   672,
     673,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,
     683,   684,   685,   686,    -1,    -1,   689,    34,    -1,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      12,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    12,    36,   102,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   795,    -1,    -1,     1,    -1,    -1,    -1,    -1,
     803,     7,    -1,    -1,    -1,    -1,    -1,    13,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    -1,    36,
      -1,    37,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   195,    -1,
     853,    -1,    -1,    -1,    -1,    -1,   859,   204,    -1,    -1,
     207,   208,   209,    -1,    -1,    -1,    -1,    73,    74,    75,
      -1,    -1,    -1,    -1,   877,   878,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,    -1,   104,   902,
      -1,   107,    -1,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,   127,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    11,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,   210,    -1,    -1,    23,    24,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    11,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    -1,    -1,    -1,   102,    -1,    -1,    -1,
      -1,    -1,    34,    -1,    -1,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    36,    -1,    -1,    -1,
     102,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,    -1,
      -1,    -1,     9,    -1,    11,    -1,    -1,    14,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,
      -1,   207,   208,   209,    -1,    -1,    -1,    34,    -1,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,    -1,
      -1,    -1,    -1,   195,    11,    -1,    -1,    14,    -1,   201,
     202,    -1,   204,   205,   206,   102,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      -1,    36,    -1,     3,     4,   102,    -1,    -1,    -1,    -1,
      -1,    11,    -1,    -1,    14,    -1,   193,   194,   195,   196,
     197,   198,   199,    23,    24,    -1,   203,   204,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    -1,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    11,    -1,    -1,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,
      -1,    -1,   102,   200,   201,   202,    -1,   204,    34,    -1,
      -1,    37,    38,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    -1,
      36,    -1,     3,     4,    -1,    -1,   102,    -1,    -1,    -1,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   201,   202,    34,   204,    -1,    37,    38,    -1,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     3,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    34,   204,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,    -1,
      -1,    -1,     9,    -1,    11,   102,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
      37,    38,   203,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      -1,     3,     4,    -1,    -1,    -1,    -1,     9,    -1,    11,
      -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    24,    -1,    -1,   102,    -1,    -1,    -1,    -1,
      -1,    -1,    34,    -1,   201,    37,    38,    -1,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     102,    -1,    -1,     3,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    -1,    -1,    14,    -1,   193,   194,   195,   196,
     197,   198,   199,    23,    24,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    -1,    -1,    37,    38,    -1,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    39,    -1,
      -1,   193,   194,   195,   196,   197,   198,   199,    -1,    -1,
      -1,    -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      91,    92,    -1,    -1,    -1,    96,    97,    98,    99,   100,
     101,   102,    -1,   104,    -1,    -1,    -1,    -1,   109,   110,
     111,   112,   113,   114,    -1,    -1,   117,   118,   119,   120,
     121,   122,    -1,   124,   125,   126,   127,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   195,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      11,    -1,    -1,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    24,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     3,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    11,    -1,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    34,    -1,    -1,
      37,    38,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   102,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   212,   213,     0,    23,    24,   215,     1,     7,    73,
      75,    93,    94,    95,   107,   115,   116,   216,   219,   220,
     221,   222,   223,   231,   235,   239,   240,   241,   242,   243,
     244,   262,   264,   269,   270,   299,   217,     1,     7,     8,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,   105,   106,   175,   238,     7,     8,     3,
       4,    11,    14,    23,    24,    34,    37,    38,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,   102,   229,   230,   322,   323,
     326,   330,   326,    13,   271,   325,   326,   265,    13,   210,
     214,    37,    39,    74,    90,    91,    92,    96,    97,    98,
      99,   100,   101,   102,   104,   109,   110,   111,   112,   113,
     114,   117,   118,   119,   120,   121,   122,   124,   125,   126,
     127,   128,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   224,   225,   227,
     232,   236,   245,   247,   251,   252,   253,   254,   255,   256,
     257,   259,   260,   261,   274,   275,   277,   278,   279,   280,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   296,   300,   301,   302,   303,   304,   305,   306,
     307,   334,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   379,   380,   381,   382,   383,   384,   385,   386,   387,
     388,   389,   390,   391,   392,   393,   394,   395,   214,   218,
     326,   326,   330,   276,   325,   276,   276,   326,   322,   322,
     322,   322,   322,   324,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    11,
      11,    11,    11,    11,    11,    11,    11,    11,    11,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    36,    13,
      13,    74,    13,    74,   226,   246,   325,   233,   330,   308,
     309,   322,   323,   327,   330,   308,   229,   297,   298,   311,
     312,   318,   323,   330,    39,   325,   313,   314,   321,   323,
     330,   315,   316,   323,   330,   103,   111,   335,   325,   330,
     330,    74,    74,   326,   214,   335,   330,   330,   127,   128,
     129,   258,   258,   294,   228,   248,     9,    57,    58,   193,
     194,   195,   196,   197,   198,   199,   317,   322,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   408,   203,   204,
     397,   398,   397,   398,   310,   326,    14,   195,   207,   208,
     209,   320,   322,   406,   407,   397,   398,    57,    58,   201,
     202,   203,   204,   399,   408,   410,   408,   410,   204,   320,
     406,   320,   406,     9,   201,   202,   203,   204,   360,   396,
     408,   411,   412,   413,     9,    57,   399,     9,   399,     9,
     360,   396,   399,   397,   398,   200,   201,   202,   204,   409,
     409,   310,    14,   406,   317,   408,   408,   408,   408,   397,
     398,   310,   317,   408,   408,   408,   397,   398,   408,   397,
     398,    77,    78,    79,   105,   106,   175,   238,    77,    78,
      79,   238,     8,   214,    12,    11,   230,   330,   139,   140,
     141,   142,   143,   144,   145,   146,   330,   336,   330,   336,
     326,   326,   326,   326,   326,   326,   326,   326,   326,   326,
     326,   326,   326,   326,   326,   322,   322,   322,   330,   330,
     330,   330,   330,    12,   330,   331,   330,   330,   330,   330,
     332,   330,   330,   330,   230,   322,   322,   322,   322,   322,
     322,   322,   322,   322,   322,   322,   322,   322,   322,   322,
     322,   322,   322,   322,   322,   272,   263,   266,    74,     6,
     234,   214,     6,   237,   237,     6,   237,     6,   237,     9,
       6,     6,   237,     6,   237,   330,     6,     6,     6,   330,
       6,     6,     6,   322,   326,   229,   295,    74,     4,   249,
     250,   204,    11,    11,     6,     6,     6,     6,   406,     6,
      11,    11,     6,     6,    58,   201,   202,   204,   205,   206,
     320,   322,   401,     6,     6,     6,     6,     6,     6,     6,
       6,   204,    11,     6,   204,     6,     6,     6,     6,     6,
       6,   326,   326,   330,   276,   276,   276,   326,   326,   326,
     330,   326,   339,   230,    12,    12,    12,    12,    12,    12,
       6,   329,   329,   329,   329,   329,   329,     6,     6,     6,
       6,     6,     6,   329,   329,   329,    12,    12,    12,     6,
       6,     6,     6,    12,     6,    12,    12,    12,     6,   333,
      12,    12,     6,    12,    12,    13,   214,     6,   326,     4,
     309,   298,   312,   246,   281,   317,   314,   316,     6,   326,
     317,   328,    74,     6,     6,     6,     6,   237,   250,    10,
     200,   201,   202,   204,   201,   202,   204,   317,   408,   317,
     410,   408,   320,   320,   320,    11,    10,    10,    23,    24,
      10,    10,    10,    23,    10,   320,   320,   204,   320,   203,
     320,   399,   203,   399,   317,   360,   396,   408,   411,   399,
      10,     9,    10,     9,   399,   399,   360,   396,   408,   408,
      12,   326,    12,    12,    12,    12,    12,    12,   326,   326,
     326,   326,   326,   326,    12,    12,    12,   330,   330,   330,
     326,   330,   330,     6,   326,   273,   268,   326,    10,     6,
     237,   336,     6,   336,   330,   330,   229,    12,    12,    12,
      12,    12,    12,    12,    10,    10,   401,    23,    24,   319,
       6,   204,     6,   204,   329,   329,   329,   329,   329,   329,
      12,    12,    12,     6,    12,     6,   327,   330,    12,   214,
      13,     6,     6,   237,   317,     9,   337,   326,   337,    10,
     322,   322,   399,    10,   399,    10,    12,    12,    12,    12,
      12,    12,   325,   330,   267,   326,   281,   325,   338,   338,
      12,    12,   214,     6,   237,    10,     6,   326,    38,    39,
       9,     9,   325,   246,    10,    10
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   211,   212,   213,   213,   214,   214,   215,   215,   215,
     216,   216,   217,   216,   218,   216,   219,   219,   219,   219,
     219,   219,   219,   219,   220,   221,   222,   223,   223,   223,
     223,   223,   224,   226,   225,   228,   227,   229,   229,   229,
     230,   230,   231,   231,   231,   231,   231,   231,   231,   233,
     232,   234,   234,   235,   235,   235,   235,   235,   235,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,   236,   236,   236,   236,   236,   236,
     237,   237,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   239,   240,   240,   241,   242,   243,   244,   245,
     246,   246,   248,   247,   249,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   258,   258,   258,   259,   259,
     259,   259,   260,   260,   261,   261,   263,   262,   265,   266,
     267,   264,   268,   268,   268,   269,   271,   272,   273,   270,
     274,   275,   276,   276,   277,   278,   279,   280,   280,   280,
     280,   281,   281,   282,   282,   283,   283,   284,   284,   285,
     286,   287,   287,   287,   287,   288,   289,   290,   291,   292,
     294,   293,   295,   295,   296,   297,   297,   298,   299,   300,
     300,   300,   301,   302,   302,   303,   304,   305,   306,   307,
     307,   308,   308,   309,   309,   310,   311,   311,   312,   312,
     313,   313,   314,   314,   315,   315,   316,   316,   317,   318,
     319,   319,   320,   321,   322,   322,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   324,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   325,   326,   327,   328,   329,
     329,   330,   330,   330,   330,   330,   330,   330,   330,   330,
     330,   330,   331,   331,   332,   333,   333,   333,   334,   335,
     335,   335,   336,   336,   336,   336,   336,   336,   336,   336,
     337,   337,   338,   338,   338,   339,   339,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   340,   340,
     340,   340,   340,   341,   341,   342,   342,   342,   342,   343,
     343,   344,   345,   345,   346,   347,   347,   348,   349,   350,
     350,   351,   352,   353,   354,   354,   355,   355,   355,   356,
     356,   357,   357,   358,   358,   359,   359,   359,   359,   360,
     360,   361,   361,   361,   361,   361,   361,   361,   361,   362,
     362,   363,   363,   364,   364,   365,   366,   367,   367,   368,
     368,   368,   369,   369,   370,   371,   371,   372,   373,   374,
     375,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   386,   387,   388,   389,   390,   391,   392,
     392,   393,   393,   394,   395,   395,   396,   397,   397,   398,
     398,   399,   399,   400,   400,   401,   401,   402,   402,   403,
     403,   404,   404,   405,   405,   406,   406,   407,   407,   407,
     407,   408,   408,   408,   408,   408,   408,   408,   408,   409,
     409,   409,   409,   410,   410,   410,   410,   411,   411,   411,
     411,   412,   412,   413,   413
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     3,     1,     1,     0,     1,     1,
       2,     1,     0,     3,     0,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     2,     1,     2,     2,
       2,     1,     1,     0,     3,     0,     3,     1,     1,     1,
       1,     1,     0,     1,     1,     2,     2,     2,     2,     0,
       3,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     2,
       1,     3,     0,     3,     1,     2,     1,     1,     1,     1,
       1,     1,     2,     2,     0,     2,     2,     2,     3,     5,
       3,     5,     1,     2,     7,     1,     0,     5,     0,     0,
       0,     9,     1,     3,     5,     3,     0,     0,     0,     7,
       2,     1,     0,     1,     1,     1,     1,     2,     5,     6,
       8,     1,     3,     1,     3,     1,     3,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       0,     4,     1,     3,     3,     1,     3,     1,     4,     2,
       4,     6,     4,     2,     4,     2,     1,     1,     3,     1,
       3,     1,     3,     1,     1,     1,     1,     3,     1,     1,
       1,     3,     1,     1,     1,     3,     1,     1,     1,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     0,     5,     5,     5,     5,     7,     7,     7,     7,
       7,     5,     5,     5,     5,     5,     5,     7,     6,     6,
       6,     4,     4,     4,     3,     1,     1,     1,     1,     0,
       2,     1,     8,     6,     6,     3,     4,     4,     4,     8,
       4,     4,     1,     3,     2,     0,     3,     3,     7,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     3,     0,     6,     6,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     4,     4,     2,
       2,     4,     2,     4,     1,     2,     2,     1,     1,     2,
       2,     1,     1,     1,     2,     2,     2,     4,     2,     2,
       4,     6,     6,     6,     6,     4,     4,     4,     4,     3,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     1,     2,     2,     2,     2,     4,
       1,     2,     1,     2,     1,     2,     1,     2,     1,     2,
       1,     2,     2,     2,     1,     4,     2,     2,     2,     1,
       2,     2,     2,     2,     2,     2,     3,     1,     3,     1,
       3,     1,     4,     1,     4,     1,     4,     1,     4,     1,
       4,     1,     4,     1,     4,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     1,
       1,     3,     4,     3,     4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= T_EOF)
    {
      yychar = T_EOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 8: /* opt_diff_mark: "+"  */
#line 729 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           {
			error("syntax error, unexpected + at the beginning of the line (is it a leftover diff mark?)\n");
		}
#line 3046 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 9: /* opt_diff_mark: "-"  */
#line 732 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           {
			error("syntax error, unexpected - at the beginning of the line (is it a leftover diff mark?)\n");
		}
#line 3054 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 740 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                        {
			lexer_SetMode(LEXER_NORMAL);
			lexer_ToggleStringExpansion(true);
		}
#line 3063 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 13: /* line: error $@1 endofline  */
#line 743 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			fstk_StopRept();
			yyerrok;
		}
#line 3072 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 748 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                {
			lexer_SetMode(LEXER_NORMAL);
			lexer_ToggleStringExpansion(true);
		}
#line 3081 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 15: /* line: "label" error $@2 endofline  */
#line 751 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			struct Symbol *macro = sym_FindExactSymbol((yyvsp[-3].symName));

			if (macro && macro->type == SYM_MACRO)
				fprintf(stderr,
					"    To invoke `%s` as a macro it must be indented\n", (yyvsp[-3].symName));
			fstk_StopRept();
			yyerrok;
		}
#line 3095 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 24: /* if: "IF" const "newline"  */
#line 777 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           {
			lexer_IncIFDepth();

			if ((yyvsp[-1].constValue))
				lexer_RunIFBlock();
			else
				lexer_SetMode(LEXER_SKIP_TO_ELIF);
		}
#line 3108 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 25: /* elif: "ELIF" const "newline"  */
#line 787 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			if (lexer_GetIFDepth() == 0)
				fatalerror("Found ELIF outside an IF construct\n");

			if (lexer_RanIFBlock()) {
				if (lexer_ReachedELSEBlock())
					fatalerror("Found ELIF after an ELSE block\n");

				lexer_SetMode(LEXER_SKIP_TO_ENDC);
			} else if ((yyvsp[-1].constValue)) {
				lexer_RunIFBlock();
			} else {
				lexer_SetMode(LEXER_SKIP_TO_ELIF);
			}
		}
#line 3128 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 26: /* else: "ELSE" "newline"  */
#line 804 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			if (lexer_GetIFDepth() == 0)
				fatalerror("Found ELSE outside an IF construct\n");

			if (lexer_RanIFBlock()) {
				if (lexer_ReachedELSEBlock())
					fatalerror("Found ELSE after an ELSE block\n");

				lexer_SetMode(LEXER_SKIP_TO_ENDC);
			} else {
				lexer_RunIFBlock();
				lexer_ReachELSEBlock();
			}
		}
#line 3147 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 32: /* endc: "ENDC"  */
#line 827 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             {
			lexer_DecIFDepth();
		}
#line 3155 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 33: /* $@3: %empty  */
#line 832 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           {
			lexer_ToggleStringExpansion(false);
		}
#line 3163 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 34: /* def_id: "DEF" $@3 "identifier"  */
#line 834 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                       {
			lexer_ToggleStringExpansion(true);
			strcpy((yyval.symName), (yyvsp[0].symName));
		}
#line 3172 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 35: /* $@4: %empty  */
#line 840 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              {
			lexer_ToggleStringExpansion(false);
		}
#line 3180 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 36: /* redef_id: "REDEF" $@4 "identifier"  */
#line 842 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                       {
			lexer_ToggleStringExpansion(true);
			strcpy((yyval.symName), (yyvsp[0].symName));
		}
#line 3189 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 43: /* label: ":"  */
#line 854 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                          {
			sym_AddAnonLabel();
		}
#line 3197 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 44: /* label: "local identifier"  */
#line 857 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             {
			sym_AddLocalLabel((yyvsp[0].symName));
		}
#line 3205 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 45: /* label: "local identifier" ":"  */
#line 860 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     {
			sym_AddLocalLabel((yyvsp[-1].symName));
		}
#line 3213 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 46: /* label: "label" ":"  */
#line 863 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sym_AddLabel((yyvsp[-1].symName));
		}
#line 3221 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 47: /* label: "local identifier" "::"  */
#line 866 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			sym_AddLocalLabel((yyvsp[-1].symName));
			sym_Export((yyvsp[-1].symName));
		}
#line 3230 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 48: /* label: "label" "::"  */
#line 870 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         {
			sym_AddLabel((yyvsp[-1].symName));
			sym_Export((yyvsp[-1].symName));
		}
#line 3239 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 49: /* $@5: %empty  */
#line 876 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                       {
			// Parsing 'macroargs' will restore the lexer's normal mode
			lexer_SetMode(LEXER_RAW);
		}
#line 3248 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 50: /* macro: "identifier" $@5 macroargs  */
#line 879 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			fstk_RunMacro((yyvsp[-2].symName), (yyvsp[0].macroArg));
		}
#line 3256 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 51: /* macroargs: %empty  */
#line 884 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			(yyval.macroArg) = macro_NewArgs();
		}
#line 3264 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 52: /* macroargs: macroargs "string"  */
#line 887 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     {
			macro_AppendArg(&((yyval.macroArg)), strdup((yyvsp[0].string)));
		}
#line 3272 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 102: /* compoundeq: "+="  */
#line 947 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_ADD; }
#line 3278 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 103: /* compoundeq: "-="  */
#line 948 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_SUB; }
#line 3284 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 104: /* compoundeq: "*="  */
#line 949 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_MUL; }
#line 3290 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 105: /* compoundeq: "/="  */
#line 950 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_DIV; }
#line 3296 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 106: /* compoundeq: "%="  */
#line 951 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_MOD; }
#line 3302 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 107: /* compoundeq: "^="  */
#line 952 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_XOR; }
#line 3308 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 108: /* compoundeq: "|="  */
#line 953 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { (yyval.compoundEqual) = RPN_OR; }
#line 3314 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 109: /* compoundeq: "&="  */
#line 954 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_AND; }
#line 3320 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 110: /* compoundeq: "<<="  */
#line 955 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_SHL; }
#line 3326 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 111: /* compoundeq: ">>="  */
#line 956 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.compoundEqual) = RPN_SHR; }
#line 3332 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 112: /* equ: "label" "EQU" const  */
#line 959 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                          {
			warning(WARNING_OBSOLETE, "`%s EQU` is deprecated; use `DEF %s EQU`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddEqu((yyvsp[-2].symName), (yyvsp[0].constValue));
		}
#line 3341 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 113: /* assignment: "label" "=" const  */
#line 965 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			warning(WARNING_OBSOLETE, "`%s =` is deprecated; use `DEF %s =`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddVar((yyvsp[-2].symName), (yyvsp[0].constValue));
		}
#line 3350 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 114: /* assignment: "label" compoundeq const  */
#line 969 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           {
			const char *compoundEqOperator = NULL;
			switch ((yyvsp[-1].compoundEqual)) {
				case RPN_ADD: compoundEqOperator = "+="; break;
				case RPN_SUB: compoundEqOperator = "-="; break;
				case RPN_MUL: compoundEqOperator = "*="; break;
				case RPN_DIV: compoundEqOperator = "/="; break;
				case RPN_MOD: compoundEqOperator = "%="; break;
				case RPN_XOR: compoundEqOperator = "^="; break;
				case RPN_OR:  compoundEqOperator = "|="; break;
				case RPN_AND: compoundEqOperator = "&="; break;
				case RPN_SHL: compoundEqOperator = "<<="; break;
				case RPN_SHR: compoundEqOperator = ">>="; break;
				default: break;
			}

			warning(WARNING_OBSOLETE, "`%s %s` is deprecated; use `DEF %s %s`\n",
				(yyvsp[-2].symName), compoundEqOperator, (yyvsp[-2].symName), compoundEqOperator);
			compoundAssignment((yyvsp[-2].symName), (yyvsp[-1].compoundEqual), (yyvsp[0].constValue));
		}
#line 3375 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 115: /* equs: "label" "EQUS" string  */
#line 991 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			warning(WARNING_OBSOLETE, "`%s EQUS` is deprecated; use `DEF %s EQUS`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddString((yyvsp[-2].symName), (yyvsp[0].string));
		}
#line 3384 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 116: /* rb: "label" "RB" rs_uconst  */
#line 997 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			warning(WARNING_OBSOLETE, "`%s RB` is deprecated; use `DEF %s RB`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + (yyvsp[0].constValue));
		}
#line 3394 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 117: /* rw: "label" "RW" rs_uconst  */
#line 1004 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			warning(WARNING_OBSOLETE, "`%s RW` is deprecated; use `DEF %s RW`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + 2 * (yyvsp[0].constValue));
		}
#line 3404 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 118: /* rl: "label" "rl" rs_uconst  */
#line 1011 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			warning(WARNING_OBSOLETE, "`%s RL` is deprecated; use `DEF %s RL`\n", (yyvsp[-2].symName), (yyvsp[-2].symName));
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + 4 * (yyvsp[0].constValue));
		}
#line 3414 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 119: /* align: "ALIGN" align_spec  */
#line 1018 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                        { sect_AlignPC((yyvsp[0].alignSpec).alignment, (yyvsp[0].alignSpec).alignOfs); }
#line 3420 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 120: /* align_spec: uconst  */
#line 1021 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			if ((yyvsp[0].constValue) > 16) {
				error("Alignment must be between 0 and 16, not %u\n", (yyvsp[0].constValue));
				(yyval.alignSpec).alignment = (yyval.alignSpec).alignOfs = 0;
			} else {
				(yyval.alignSpec).alignment = (yyvsp[0].constValue);
				(yyval.alignSpec).alignOfs = 0;
			}
		}
#line 3434 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 121: /* align_spec: uconst "," const  */
#line 1030 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			if ((yyvsp[-2].constValue) > 16) {
				error("Alignment must be between 0 and 16, not %u\n", (yyvsp[-2].constValue));
				(yyval.alignSpec).alignment = (yyval.alignSpec).alignOfs = 0;
			} else if ((yyvsp[0].constValue) <= -(1 << (yyvsp[-2].constValue)) || (yyvsp[0].constValue) >= 1 << (yyvsp[-2].constValue)) {
				error("The absolute alignment offset (%" PRIu32
				      ") must be less than alignment size (%d)\n",
				      (uint32_t)((yyvsp[0].constValue) < 0 ? -(yyvsp[0].constValue) : (yyvsp[0].constValue)), 1 << (yyvsp[-2].constValue));
				(yyval.alignSpec).alignment = (yyval.alignSpec).alignOfs = 0;
			} else {
				(yyval.alignSpec).alignment = (yyvsp[-2].constValue);
				(yyval.alignSpec).alignOfs = (yyvsp[0].constValue) < 0 ? (1 << (yyvsp[-2].constValue)) + (yyvsp[0].constValue) : (yyvsp[0].constValue);
			}
		}
#line 3453 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 122: /* $@6: %empty  */
#line 1046 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			// Parsing 'opt_list' will restore the lexer's normal mode
			lexer_SetMode(LEXER_RAW);
		}
#line 3462 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 126: /* opt_list_entry: "string"  */
#line 1056 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { opt_Parse((yyvsp[0].string)); }
#line 3468 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 127: /* popo: "POPO"  */
#line 1059 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { opt_Pop(); }
#line 3474 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 128: /* pusho: "PUSHO"  */
#line 1062 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { opt_Push(); }
#line 3480 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 129: /* pops: "POPS"  */
#line 1065 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_PopSection(); }
#line 3486 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 130: /* pushs: "PUSHS"  */
#line 1068 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { sect_PushSection(); }
#line 3492 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 131: /* endsection: "ENDSECTION"  */
#line 1071 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_EndSection(); }
#line 3498 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 132: /* fail: "FAIL" string  */
#line 1074 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                    { fatalerror("%s\n", (yyvsp[0].string)); }
#line 3504 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 133: /* warn: "WARN" string  */
#line 1077 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                    { warning(WARNING_USER, "%s\n", (yyvsp[0].string)); }
#line 3510 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 134: /* assert_type: %empty  */
#line 1080 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.assertType) = ASSERT_ERROR; }
#line 3516 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 135: /* assert_type: "WARN" ","  */
#line 1081 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     { (yyval.assertType) = ASSERT_WARN; }
#line 3522 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 136: /* assert_type: "FAIL" ","  */
#line 1082 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     { (yyval.assertType) = ASSERT_ERROR; }
#line 3528 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 137: /* assert_type: "FATAL" ","  */
#line 1083 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                      { (yyval.assertType) = ASSERT_FATAL; }
#line 3534 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 138: /* assert: "ASSERT" assert_type relocexpr  */
#line 1086 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     {
			if (!rpn_isKnown(&(yyvsp[0].expr))) {
				if (!out_CreateAssert((yyvsp[-1].assertType), &(yyvsp[0].expr), "",
						      sect_GetOutputOffset()))
					error("Assertion creation failed: %s\n",
						strerror(errno));
			} else if ((yyvsp[0].expr).val == 0) {
				failAssert((yyvsp[-1].assertType));
			}
			rpn_Free(&(yyvsp[0].expr));
		}
#line 3550 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 139: /* assert: "ASSERT" assert_type relocexpr "," string  */
#line 1097 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                    {
			if (!rpn_isKnown(&(yyvsp[-2].expr))) {
				if (!out_CreateAssert((yyvsp[-3].assertType), &(yyvsp[-2].expr), (yyvsp[0].string),
						      sect_GetOutputOffset()))
					error("Assertion creation failed: %s\n",
						strerror(errno));
			} else if ((yyvsp[-2].expr).val == 0) {
				failAssertMsg((yyvsp[-3].assertType), (yyvsp[0].string));
			}
			rpn_Free(&(yyvsp[-2].expr));
		}
#line 3566 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 140: /* assert: "STATIC_ASSERT" assert_type const  */
#line 1108 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        {
			if ((yyvsp[0].constValue) == 0)
				failAssert((yyvsp[-1].assertType));
		}
#line 3575 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 141: /* assert: "STATIC_ASSERT" assert_type const "," string  */
#line 1112 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                       {
			if ((yyvsp[-2].constValue) == 0)
				failAssertMsg((yyvsp[-3].assertType), (yyvsp[0].string));
		}
#line 3584 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 142: /* shift: "SHIFT"  */
#line 1118 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { macro_ShiftCurrentArgs(1); }
#line 3590 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 143: /* shift: "SHIFT" const  */
#line 1119 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                    { macro_ShiftCurrentArgs((yyvsp[0].constValue)); }
#line 3596 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 144: /* load: "LOAD" sectmod string "," sectiontype sectorg sectattrs  */
#line 1122 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                  {
			sect_SetLoadSection((yyvsp[-4].string), (enum SectionType)(yyvsp[-2].constValue), (yyvsp[-1].constValue), &(yyvsp[0].sectSpec), (yyvsp[-5].sectMod));
		}
#line 3604 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 145: /* load: "ENDL"  */
#line 1125 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_EndLoadSection(); }
#line 3610 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 146: /* @7: %empty  */
#line 1128 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              {
			(yyval.captureTerminated) = lexer_CaptureRept(&captureBody);
		}
#line 3618 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 147: /* rept: "REPT" uconst "newline" @7 endofline  */
#line 1130 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			if ((yyvsp[-1].captureTerminated))
				fstk_RunRept((yyvsp[-3].constValue), captureBody.lineNo, captureBody.body,
					     captureBody.size);
		}
#line 3628 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 148: /* $@8: %empty  */
#line 1137 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			lexer_ToggleStringExpansion(false);
		}
#line 3636 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 149: /* $@9: %empty  */
#line 1139 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                       {
			lexer_ToggleStringExpansion(true);
		}
#line 3644 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 150: /* @10: %empty  */
#line 1141 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			(yyval.captureTerminated) = lexer_CaptureRept(&captureBody);
		}
#line 3652 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 151: /* for: "FOR" $@8 "identifier" $@9 "," for_args "newline" @10 endofline  */
#line 1143 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			if ((yyvsp[-1].captureTerminated))
				fstk_RunFor((yyvsp[-6].symName), (yyvsp[-3].forArgs).start, (yyvsp[-3].forArgs).stop, (yyvsp[-3].forArgs).step, captureBody.lineNo,
					    captureBody.body, captureBody.size);
		}
#line 3662 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 152: /* for_args: const  */
#line 1149 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                        {
			(yyval.forArgs).start = 0;
			(yyval.forArgs).stop = (yyvsp[0].constValue);
			(yyval.forArgs).step = 1;
		}
#line 3672 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 153: /* for_args: const "," const  */
#line 1154 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                      {
			(yyval.forArgs).start = (yyvsp[-2].constValue);
			(yyval.forArgs).stop = (yyvsp[0].constValue);
			(yyval.forArgs).step = 1;
		}
#line 3682 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 154: /* for_args: const "," const "," const  */
#line 1159 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			(yyval.forArgs).start = (yyvsp[-4].constValue);
			(yyval.forArgs).stop = (yyvsp[-2].constValue);
			(yyval.forArgs).step = (yyvsp[0].constValue);
		}
#line 3692 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 155: /* break: label "BREAK" endofline  */
#line 1166 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              {
			if (fstk_Break())
				lexer_SetMode(LEXER_SKIP_TO_ENDR);
		}
#line 3701 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 156: /* $@11: %empty  */
#line 1172 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              {
			lexer_ToggleStringExpansion(false);
		}
#line 3709 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 157: /* $@12: %empty  */
#line 1174 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                       {
			lexer_ToggleStringExpansion(true);
		}
#line 3717 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 158: /* @13: %empty  */
#line 1176 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			(yyval.captureTerminated) = lexer_CaptureMacroBody(&captureBody);
		}
#line 3725 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 159: /* macrodef: "MACRO" $@11 "identifier" $@12 "newline" @13 endofline  */
#line 1178 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			if ((yyvsp[-1].captureTerminated))
				sym_AddMacro((yyvsp[-4].symName), captureBody.lineNo, captureBody.body,
					     captureBody.size);
		}
#line 3735 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 160: /* rsset: "RSSET" uconst  */
#line 1185 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     { sym_AddVar("_RS", (yyvsp[0].constValue)); }
#line 3741 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 161: /* rsreset: "RSRESET"  */
#line 1188 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                { sym_AddVar("_RS", 0); }
#line 3747 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 162: /* rs_uconst: %empty  */
#line 1191 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.constValue) = 1; }
#line 3753 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 164: /* union: "UNION"  */
#line 1195 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { sect_StartUnion(); }
#line 3759 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 165: /* nextu: "NEXTU"  */
#line 1198 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { sect_NextUnionMember(); }
#line 3765 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 166: /* endu: "ENDU"  */
#line 1201 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_EndUnion(); }
#line 3771 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 167: /* ds: "DS" uconst  */
#line 1204 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { sect_Skip((yyvsp[0].constValue), true); }
#line 3777 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 168: /* ds: "DS" uconst "," ds_args trailing_comma  */
#line 1205 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                 {
			sect_RelBytes((yyvsp[-3].constValue), (yyvsp[-1].dsArgs).args, (yyvsp[-1].dsArgs).nbArgs);
			freeDsArgList(&(yyvsp[-1].dsArgs));
		}
#line 3786 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 169: /* ds: "DS" "ALIGN" "[" align_spec "]" trailing_comma  */
#line 1209 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                  {
			uint32_t n = sect_GetAlignBytes((yyvsp[-2].alignSpec).alignment, (yyvsp[-2].alignSpec).alignOfs);

			sect_Skip(n, true);
			sect_AlignPC((yyvsp[-2].alignSpec).alignment, (yyvsp[-2].alignSpec).alignOfs);
		}
#line 3797 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 170: /* ds: "DS" "ALIGN" "[" align_spec "]" "," ds_args trailing_comma  */
#line 1215 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                                  {
			uint32_t n = sect_GetAlignBytes((yyvsp[-4].alignSpec).alignment, (yyvsp[-4].alignSpec).alignOfs);

			sect_RelBytes(n, (yyvsp[-1].dsArgs).args, (yyvsp[-1].dsArgs).nbArgs);
			sect_AlignPC((yyvsp[-4].alignSpec).alignment, (yyvsp[-4].alignSpec).alignOfs);
			freeDsArgList(&(yyvsp[-1].dsArgs));
		}
#line 3809 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 171: /* ds_args: reloc_8bit  */
#line 1224 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             {
			initDsArgList(&(yyval.dsArgs));
			appendDsArgList(&(yyval.dsArgs), &(yyvsp[0].expr));
		}
#line 3818 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 172: /* ds_args: ds_args "," reloc_8bit  */
#line 1228 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			appendDsArgList(&(yyvsp[-2].dsArgs), &(yyvsp[0].expr));
			(yyval.dsArgs) = (yyvsp[-2].dsArgs);
		}
#line 3827 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 173: /* db: "DB"  */
#line 1234 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { sect_Skip(1, false); }
#line 3833 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 175: /* dw: "DW"  */
#line 1238 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { sect_Skip(2, false); }
#line 3839 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 177: /* dl: "DL"  */
#line 1242 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { sect_Skip(4, false); }
#line 3845 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 179: /* def_equ: def_id "EQU" const  */
#line 1246 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         { sym_AddEqu((yyvsp[-2].symName), (yyvsp[0].constValue)); }
#line 3851 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 180: /* redef_equ: redef_id "EQU" const  */
#line 1249 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           { sym_RedefEqu((yyvsp[-2].symName), (yyvsp[0].constValue)); }
#line 3857 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 181: /* def_set: def_id "=" const  */
#line 1252 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           { sym_AddVar((yyvsp[-2].symName), (yyvsp[0].constValue)); }
#line 3863 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 182: /* def_set: redef_id "=" const  */
#line 1253 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             { sym_AddVar((yyvsp[-2].symName), (yyvsp[0].constValue)); }
#line 3869 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 183: /* def_set: def_id compoundeq const  */
#line 1254 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                          { compoundAssignment((yyvsp[-2].symName), (yyvsp[-1].compoundEqual), (yyvsp[0].constValue)); }
#line 3875 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 184: /* def_set: redef_id compoundeq const  */
#line 1255 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            { compoundAssignment((yyvsp[-2].symName), (yyvsp[-1].compoundEqual), (yyvsp[0].constValue)); }
#line 3881 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 185: /* def_rb: def_id "RB" rs_uconst  */
#line 1258 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + (yyvsp[0].constValue));
		}
#line 3890 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 186: /* def_rw: def_id "RW" rs_uconst  */
#line 1264 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + 2 * (yyvsp[0].constValue));
		}
#line 3899 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 187: /* def_rl: def_id "rl" rs_uconst  */
#line 1270 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			sym_AddEqu((yyvsp[-2].symName), sym_GetConstantValue("_RS"));
			sym_AddVar("_RS", sym_GetConstantValue("_RS") + 4 * (yyvsp[0].constValue));
		}
#line 3908 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 188: /* def_equs: def_id "EQUS" string  */
#line 1276 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           { sym_AddString((yyvsp[-2].symName), (yyvsp[0].string)); }
#line 3914 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 189: /* redef_equs: redef_id "EQUS" string  */
#line 1279 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             { sym_RedefString((yyvsp[-2].symName), (yyvsp[0].string)); }
#line 3920 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 190: /* $@14: %empty  */
#line 1282 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              {
			lexer_ToggleStringExpansion(false);
		}
#line 3928 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 191: /* purge: "PURGE" $@14 purge_args trailing_comma  */
#line 1284 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                            {
			for (uint32_t i = 0; i < (yyvsp[-1].purgeArgs).nbArgs; i++)
				sym_Purge((yyvsp[-1].purgeArgs).args[i]);
			freePurgeArgList(&(yyvsp[-1].purgeArgs));
			lexer_ToggleStringExpansion(true);
		}
#line 3939 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 192: /* purge_args: scoped_id  */
#line 1292 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			initPurgeArgList(&(yyval.purgeArgs));
			appendPurgeArgList(&(yyval.purgeArgs), strdup((yyvsp[0].symName)));
		}
#line 3948 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 193: /* purge_args: purge_args "," scoped_id  */
#line 1296 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			appendPurgeArgList(&(yyvsp[-2].purgeArgs), strdup((yyvsp[0].symName)));
			(yyval.purgeArgs) = (yyvsp[-2].purgeArgs);
		}
#line 3957 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 197: /* export_list_entry: scoped_id  */
#line 1309 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { sym_Export((yyvsp[0].symName)); }
#line 3963 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 198: /* include: label "INCLUDE" string endofline  */
#line 1312 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       {
			fstk_RunInclude((yyvsp[-1].string));
			if (failedOnMissingInclude)
				YYACCEPT;
		}
#line 3973 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 199: /* incbin: "INCBIN" string  */
#line 1319 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                      {
			sect_BinaryFile((yyvsp[0].string), 0);
			if (failedOnMissingInclude)
				YYACCEPT;
		}
#line 3983 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 200: /* incbin: "INCBIN" string "," const  */
#line 1324 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			sect_BinaryFile((yyvsp[-2].string), (yyvsp[0].constValue));
			if (failedOnMissingInclude)
				YYACCEPT;
		}
#line 3993 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 201: /* incbin: "INCBIN" string "," const "," const  */
#line 1329 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                  {
			sect_BinaryFileSlice((yyvsp[-4].string), (yyvsp[-2].constValue), (yyvsp[0].constValue));
			if (failedOnMissingInclude)
				YYACCEPT;
		}
#line 4003 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 202: /* charmap: "CHARMAP" string "," const_8bit  */
#line 1336 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                          {
			charmap_Add((yyvsp[-2].string), (uint8_t)(yyvsp[0].constValue));
		}
#line 4011 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 203: /* newcharmap: "NEWCHARMAP" "identifier"  */
#line 1341 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                        { charmap_New((yyvsp[0].symName), NULL); }
#line 4017 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 204: /* newcharmap: "NEWCHARMAP" "identifier" "," "identifier"  */
#line 1342 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     { charmap_New((yyvsp[-2].symName), (yyvsp[0].symName)); }
#line 4023 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 205: /* setcharmap: "SETCHARMAP" "identifier"  */
#line 1345 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                        { charmap_Set((yyvsp[0].symName)); }
#line 4029 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 206: /* pushc: "PUSHC"  */
#line 1348 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { charmap_Push(); }
#line 4035 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 207: /* popc: "POPC"  */
#line 1351 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { charmap_Pop(); }
#line 4041 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 209: /* println: "PRINTLN"  */
#line 1357 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                {
			putchar('\n');
			fflush(stdout);
		}
#line 4050 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 210: /* println: "PRINTLN" print_exprs trailing_comma  */
#line 1361 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                           {
			putchar('\n');
			fflush(stdout);
		}
#line 4059 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 213: /* print_expr: const_no_str  */
#line 1371 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                               { printf("$%" PRIX32, (yyvsp[0].constValue)); }
#line 4065 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 214: /* print_expr: string  */
#line 1372 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { printf("%s", (yyvsp[0].string)); }
#line 4071 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 215: /* const_3bit: const  */
#line 1375 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                        {
			int32_t value = (yyvsp[0].constValue);

			if ((value < 0) || (value > 7)) {
				error("Immediate value must be 3-bit\n");
				(yyval.constValue) = 0;
			} else {
				(yyval.constValue) = value & 0x7;
			}
		}
#line 4086 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 218: /* constlist_8bit_entry: reloc_8bit_no_str  */
#line 1391 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         {
			sect_RelByte(&(yyvsp[0].expr), 0);
		}
#line 4094 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 219: /* constlist_8bit_entry: string  */
#line 1394 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			uint8_t *output = (uint8_t *)malloc(strlen((yyvsp[0].string))); // Cannot be larger than that
			size_t length = charmap_Convert((yyvsp[0].string), output);

			sect_AbsByteGroup(output, length);
			free(output);
		}
#line 4106 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 222: /* constlist_16bit_entry: reloc_16bit_no_str  */
#line 1407 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           {
			sect_RelWord(&(yyvsp[0].expr), 0);
		}
#line 4114 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 223: /* constlist_16bit_entry: string  */
#line 1410 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			uint8_t *output = (uint8_t *)malloc(strlen((yyvsp[0].string))); // Cannot be larger than that
			size_t length = charmap_Convert((yyvsp[0].string), output);

			sect_AbsWordGroup(output, length);
			free(output);
		}
#line 4126 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 226: /* constlist_32bit_entry: relocexpr_no_str  */
#line 1423 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         {
			sect_RelLong(&(yyvsp[0].expr), 0);
		}
#line 4134 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 227: /* constlist_32bit_entry: string  */
#line 1426 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			// Charmaps cannot increase the length of a string
			uint8_t *output = (uint8_t *)malloc(strlen((yyvsp[0].string)));
			size_t length = charmap_Convert((yyvsp[0].string), output);

			sect_AbsLongGroup(output, length);
			free(output);
		}
#line 4147 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 228: /* reloc_8bit: relocexpr  */
#line 1436 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			rpn_CheckNBit(&(yyvsp[0].expr), 8);
			(yyval.expr) = (yyvsp[0].expr);
		}
#line 4156 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 229: /* reloc_8bit_no_str: relocexpr_no_str  */
#line 1442 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     {
			rpn_CheckNBit(&(yyvsp[0].expr), 8);
			(yyval.expr) = (yyvsp[0].expr);
		}
#line 4165 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 230: /* reloc_8bit_offset: "+" relocexpr  */
#line 1448 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			rpn_CheckNBit(&(yyvsp[0].expr), 8);
			(yyval.expr) = (yyvsp[0].expr);
		}
#line 4174 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 231: /* reloc_8bit_offset: "-" relocexpr  */
#line 1452 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     {
			rpn_NEG(&(yyval.expr), &(yyvsp[0].expr));
			rpn_CheckNBit(&(yyval.expr), 8);
		}
#line 4183 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 232: /* reloc_16bit: relocexpr  */
#line 1458 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            {
			rpn_CheckNBit(&(yyvsp[0].expr), 16);
			(yyval.expr) = (yyvsp[0].expr);
		}
#line 4192 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 233: /* reloc_16bit_no_str: relocexpr_no_str  */
#line 1464 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                      {
			rpn_CheckNBit(&(yyvsp[0].expr), 16);
			(yyval.expr) = (yyvsp[0].expr);
		}
#line 4201 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 235: /* relocexpr: string  */
#line 1472 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			// Charmaps cannot increase the length of a string
			uint8_t *output = (uint8_t *)malloc(strlen((yyvsp[0].string)));
			uint32_t length = charmap_Convert((yyvsp[0].string), output);
			uint32_t r = str2int2(output, length);

			free(output);
			rpn_Number(&(yyval.expr), r);
		}
#line 4215 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 236: /* relocexpr_no_str: scoped_anon_id  */
#line 1483 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { rpn_Symbol(&(yyval.expr), (yyvsp[0].symName)); }
#line 4221 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 237: /* relocexpr_no_str: "number"  */
#line 1484 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { rpn_Number(&(yyval.expr), (yyvsp[0].constValue)); }
#line 4227 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 238: /* relocexpr_no_str: "!" relocexpr  */
#line 1485 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			rpn_LOGNOT(&(yyval.expr), &(yyvsp[0].expr));
		}
#line 4235 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 239: /* relocexpr_no_str: relocexpr "||" relocexpr  */
#line 1488 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGOR, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4243 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 240: /* relocexpr_no_str: relocexpr "&&" relocexpr  */
#line 1491 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			rpn_BinaryOp(RPN_LOGAND, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4251 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 241: /* relocexpr_no_str: relocexpr "==" relocexpr  */
#line 1494 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			rpn_BinaryOp(RPN_LOGEQ, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4259 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 242: /* relocexpr_no_str: relocexpr ">" relocexpr  */
#line 1497 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGGT, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4267 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 243: /* relocexpr_no_str: relocexpr "<" relocexpr  */
#line 1500 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGLT, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4275 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 244: /* relocexpr_no_str: relocexpr ">=" relocexpr  */
#line 1503 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGGE, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4283 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 245: /* relocexpr_no_str: relocexpr "<=" relocexpr  */
#line 1506 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGLE, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4291 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 246: /* relocexpr_no_str: relocexpr "!=" relocexpr  */
#line 1509 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_BinaryOp(RPN_LOGNE, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4299 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 247: /* relocexpr_no_str: relocexpr "+" relocexpr  */
#line 1512 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_ADD, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4307 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 248: /* relocexpr_no_str: relocexpr "-" relocexpr  */
#line 1515 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_SUB, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4315 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 249: /* relocexpr_no_str: relocexpr "^" relocexpr  */
#line 1518 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_XOR, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4323 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 250: /* relocexpr_no_str: relocexpr "|" relocexpr  */
#line 1521 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              {
			rpn_BinaryOp(RPN_OR, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4331 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 251: /* relocexpr_no_str: relocexpr "&" relocexpr  */
#line 1524 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_AND, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4339 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 252: /* relocexpr_no_str: relocexpr "<<" relocexpr  */
#line 1527 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_SHL, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4347 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 253: /* relocexpr_no_str: relocexpr ">>" relocexpr  */
#line 1530 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_SHR, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4355 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 254: /* relocexpr_no_str: relocexpr ">>>" relocexpr  */
#line 1533 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                {
			rpn_BinaryOp(RPN_USHR, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4363 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 255: /* relocexpr_no_str: relocexpr "*" relocexpr  */
#line 1536 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_MUL, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4371 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 256: /* relocexpr_no_str: relocexpr "/" relocexpr  */
#line 1539 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_DIV, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4379 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 257: /* relocexpr_no_str: relocexpr "%" relocexpr  */
#line 1542 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_MOD, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4387 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 258: /* relocexpr_no_str: relocexpr "**" relocexpr  */
#line 1545 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			rpn_BinaryOp(RPN_EXP, &(yyval.expr), &(yyvsp[-2].expr), &(yyvsp[0].expr));
		}
#line 4395 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 259: /* relocexpr_no_str: "+" relocexpr  */
#line 1548 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               { (yyval.expr) = (yyvsp[0].expr); }
#line 4401 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 260: /* relocexpr_no_str: "-" relocexpr  */
#line 1549 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               { rpn_NEG(&(yyval.expr), &(yyvsp[0].expr)); }
#line 4407 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 261: /* relocexpr_no_str: "~" relocexpr  */
#line 1550 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               { rpn_NOT(&(yyval.expr), &(yyvsp[0].expr)); }
#line 4413 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 262: /* relocexpr_no_str: "HIGH" "(" relocexpr ")"  */
#line 1551 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        { rpn_HIGH(&(yyval.expr), &(yyvsp[-1].expr)); }
#line 4419 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 263: /* relocexpr_no_str: "LOW" "(" relocexpr ")"  */
#line 1552 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       { rpn_LOW(&(yyval.expr), &(yyvsp[-1].expr)); }
#line 4425 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 264: /* relocexpr_no_str: "ISCONST" "(" relocexpr ")"  */
#line 1553 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                           { rpn_ISCONST(&(yyval.expr), &(yyvsp[-1].expr)); }
#line 4431 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 265: /* relocexpr_no_str: "BANK" "(" scoped_anon_id ")"  */
#line 1554 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                             {
			// '@' is also a T_ID; it is handled here
			rpn_BankSymbol(&(yyval.expr), (yyvsp[-1].symName));
		}
#line 4440 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 266: /* relocexpr_no_str: "BANK" "(" string ")"  */
#line 1558 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     { rpn_BankSection(&(yyval.expr), (yyvsp[-1].string)); }
#line 4446 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 267: /* relocexpr_no_str: "SIZEOF" "(" string ")"  */
#line 1559 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       { rpn_SizeOfSection(&(yyval.expr), (yyvsp[-1].string)); }
#line 4452 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 268: /* relocexpr_no_str: "STARTOF" "(" string ")"  */
#line 1560 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        { rpn_StartOfSection(&(yyval.expr), (yyvsp[-1].string)); }
#line 4458 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 269: /* relocexpr_no_str: "SIZEOF" "(" sectiontype ")"  */
#line 1561 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                            {
			rpn_SizeOfSectionType(&(yyval.expr), (enum SectionType)(yyvsp[-1].constValue));
		}
#line 4466 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 270: /* relocexpr_no_str: "STARTOF" "(" sectiontype ")"  */
#line 1564 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                             {
			rpn_StartOfSectionType(&(yyval.expr), (enum SectionType)(yyvsp[-1].constValue));
		}
#line 4474 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 271: /* $@15: %empty  */
#line 1567 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           {
			lexer_ToggleStringExpansion(false);
		}
#line 4482 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 272: /* relocexpr_no_str: "DEF" $@15 "(" scoped_anon_id ")"  */
#line 1569 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			rpn_Number(&(yyval.expr), sym_FindScopedValidSymbol((yyvsp[-1].symName)) != NULL);

			lexer_ToggleStringExpansion(true);
		}
#line 4492 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 273: /* relocexpr_no_str: "ROUND" "(" const opt_q_arg ")"  */
#line 1574 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                               {
			rpn_Number(&(yyval.expr), fix_Round((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4500 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 274: /* relocexpr_no_str: "CEIL" "(" const opt_q_arg ")"  */
#line 1577 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                              {
			rpn_Number(&(yyval.expr), fix_Ceil((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4508 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 275: /* relocexpr_no_str: "FLOOR" "(" const opt_q_arg ")"  */
#line 1580 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                               {
			rpn_Number(&(yyval.expr), fix_Floor((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4516 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 276: /* relocexpr_no_str: "FDIV" "(" const "," const opt_q_arg ")"  */
#line 1583 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                            {
			rpn_Number(&(yyval.expr), fix_Div((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4524 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 277: /* relocexpr_no_str: "FMUL" "(" const "," const opt_q_arg ")"  */
#line 1586 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                            {
			rpn_Number(&(yyval.expr), fix_Mul((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4532 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 278: /* relocexpr_no_str: "FMOD" "(" const "," const opt_q_arg ")"  */
#line 1589 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                            {
			rpn_Number(&(yyval.expr), fix_Mod((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4540 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 279: /* relocexpr_no_str: "POW" "(" const "," const opt_q_arg ")"  */
#line 1592 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                           {
			rpn_Number(&(yyval.expr), fix_Pow((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4548 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 280: /* relocexpr_no_str: "LOG" "(" const "," const opt_q_arg ")"  */
#line 1595 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                           {
			rpn_Number(&(yyval.expr), fix_Log((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4556 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 281: /* relocexpr_no_str: "SIN" "(" const opt_q_arg ")"  */
#line 1598 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                             {
			rpn_Number(&(yyval.expr), fix_Sin((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4564 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 282: /* relocexpr_no_str: "COS" "(" const opt_q_arg ")"  */
#line 1601 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                             {
			rpn_Number(&(yyval.expr), fix_Cos((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4572 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 283: /* relocexpr_no_str: "TAN" "(" const opt_q_arg ")"  */
#line 1604 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                             {
			rpn_Number(&(yyval.expr), fix_Tan((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4580 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 284: /* relocexpr_no_str: "ASIN" "(" const opt_q_arg ")"  */
#line 1607 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                              {
			rpn_Number(&(yyval.expr), fix_ASin((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4588 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 285: /* relocexpr_no_str: "ACOS" "(" const opt_q_arg ")"  */
#line 1610 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                              {
			rpn_Number(&(yyval.expr), fix_ACos((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4596 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 286: /* relocexpr_no_str: "ATAN" "(" const opt_q_arg ")"  */
#line 1613 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                              {
			rpn_Number(&(yyval.expr), fix_ATan((yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4604 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 287: /* relocexpr_no_str: "ATAN2" "(" const "," const opt_q_arg ")"  */
#line 1616 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                             {
			rpn_Number(&(yyval.expr), fix_ATan2((yyvsp[-4].constValue), (yyvsp[-2].constValue), (yyvsp[-1].constValue)));
		}
#line 4612 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 288: /* relocexpr_no_str: "STRCMP" "(" string "," string ")"  */
#line 1619 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                      {
			rpn_Number(&(yyval.expr), strcmp((yyvsp[-3].string), (yyvsp[-1].string)));
		}
#line 4620 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 289: /* relocexpr_no_str: "STRIN" "(" string "," string ")"  */
#line 1622 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                     {
			char const *p = strstr((yyvsp[-3].string), (yyvsp[-1].string));

			rpn_Number(&(yyval.expr), p ? p - (yyvsp[-3].string) + 1 : 0);
		}
#line 4630 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 290: /* relocexpr_no_str: "STRRIN" "(" string "," string ")"  */
#line 1627 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                      {
			char const *p = strrstr((yyvsp[-3].string), (yyvsp[-1].string));

			rpn_Number(&(yyval.expr), p ? p - (yyvsp[-3].string) + 1 : 0);
		}
#line 4640 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 291: /* relocexpr_no_str: "STRLEN" "(" string ")"  */
#line 1632 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       {
			rpn_Number(&(yyval.expr), strlenUTF8((yyvsp[-1].string)));
		}
#line 4648 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 292: /* relocexpr_no_str: "CHARLEN" "(" string ")"  */
#line 1635 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        {
			rpn_Number(&(yyval.expr), charlenUTF8((yyvsp[-1].string)));
		}
#line 4656 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 293: /* relocexpr_no_str: "INCHARMAP" "(" string ")"  */
#line 1638 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                          {
			rpn_Number(&(yyval.expr), charmap_HasChar((yyvsp[-1].string)));
		}
#line 4664 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 294: /* relocexpr_no_str: "(" relocexpr ")"  */
#line 1641 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              { (yyval.expr) = (yyvsp[-1].expr); }
#line 4670 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 295: /* uconst: const  */
#line 1644 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                        {
			(yyval.constValue) = (yyvsp[0].constValue);
			if ((yyval.constValue) < 0)
				fatalerror("Constant must not be negative: %d\n", (yyvsp[0].constValue));
		}
#line 4680 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 296: /* const: relocexpr  */
#line 1651 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = rpn_GetConstVal(&(yyvsp[0].expr)); }
#line 4686 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 297: /* const_no_str: relocexpr_no_str  */
#line 1654 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { (yyval.constValue) = rpn_GetConstVal(&(yyvsp[0].expr)); }
#line 4692 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 298: /* const_8bit: reloc_8bit  */
#line 1657 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { (yyval.constValue) = rpn_GetConstVal(&(yyvsp[0].expr)); }
#line 4698 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 299: /* opt_q_arg: %empty  */
#line 1660 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.constValue) = fix_Precision(); }
#line 4704 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 300: /* opt_q_arg: "," const  */
#line 1661 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                {
			if ((yyvsp[0].constValue) >= 1 && (yyvsp[0].constValue) <= 31) {
				(yyval.constValue) = (yyvsp[0].constValue);
			} else {
				error("Fixed-point precision must be between 1 and 31\n");
				(yyval.constValue) = fix_Precision();
			}
		}
#line 4717 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 302: /* string: "STRSUB" "(" string "," const "," uconst ")"  */
#line 1672 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                    {
			size_t len = strlenUTF8((yyvsp[-5].string));
			uint32_t pos = adjustNegativePos((yyvsp[-3].constValue), len, "STRSUB");

			strsubUTF8((yyval.string), sizeof((yyval.string)), (yyvsp[-5].string), pos, (yyvsp[-1].constValue));
		}
#line 4728 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 303: /* string: "STRSUB" "(" string "," const ")"  */
#line 1678 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                     {
			size_t len = strlenUTF8((yyvsp[-3].string));
			uint32_t pos = adjustNegativePos((yyvsp[-1].constValue), len, "STRSUB");

			strsubUTF8((yyval.string), sizeof((yyval.string)), (yyvsp[-3].string), pos, pos > len ? 0 : len + 1 - pos);
		}
#line 4739 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 304: /* string: "CHARSUB" "(" string "," const ")"  */
#line 1684 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                      {
			size_t len = charlenUTF8((yyvsp[-3].string));
			uint32_t pos = adjustNegativePos((yyvsp[-1].constValue), len, "CHARSUB");

			charsubUTF8((yyval.string), (yyvsp[-3].string), pos);
		}
#line 4750 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 305: /* string: "STRCAT" "(" ")"  */
#line 1690 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                {
			(yyval.string)[0] = '\0';
		}
#line 4758 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 306: /* string: "STRCAT" "(" strcat_args ")"  */
#line 1693 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                            {
			strcpy((yyval.string), (yyvsp[-1].string));
		}
#line 4766 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 307: /* string: "STRUPR" "(" string ")"  */
#line 1696 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       {
			upperstring((yyval.string), (yyvsp[-1].string));
		}
#line 4774 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 308: /* string: "STRLWR" "(" string ")"  */
#line 1699 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       {
			lowerstring((yyval.string), (yyvsp[-1].string));
		}
#line 4782 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 309: /* string: "STRRPL" "(" string "," string "," string ")"  */
#line 1702 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                     {
			strrpl((yyval.string), sizeof((yyval.string)), (yyvsp[-5].string), (yyvsp[-3].string), (yyvsp[-1].string));
		}
#line 4790 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 310: /* string: "STRFMT" "(" strfmt_args ")"  */
#line 1705 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                            {
			strfmt((yyval.string), sizeof((yyval.string)), (yyvsp[-1].strfmtArgs).format, (yyvsp[-1].strfmtArgs).nbArgs, (yyvsp[-1].strfmtArgs).args);
			freeStrFmtArgList(&(yyvsp[-1].strfmtArgs));
		}
#line 4799 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 311: /* string: "SECTION" "(" scoped_anon_id ")"  */
#line 1709 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                 {
			struct Symbol *sym = sym_FindScopedValidSymbol((yyvsp[-1].symName));

			if (!sym)
				fatalerror("Unknown symbol \"%s\"\n", (yyvsp[-1].symName));
			struct Section const *section = sym_GetSection(sym);

			if (!section)
				fatalerror("\"%s\" does not belong to any section\n", sym->name);
			// Section names are capped by rgbasm's maximum string length,
			// so this currently can't overflow.
			strcpy((yyval.string), section->name);
		}
#line 4817 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 313: /* strcat_args: strcat_args "," string  */
#line 1725 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                             {
			int ret = snprintf((yyval.string), sizeof((yyval.string)), "%s%s", (yyvsp[-2].string), (yyvsp[0].string));

			if (ret == -1)
				fatalerror("snprintf error in STRCAT: %s\n", strerror(errno));
			else if ((unsigned int)ret >= sizeof((yyval.string)))
				warning(WARNING_LONG_STR, "STRCAT: String too long '%s%s'\n",
					(yyvsp[-2].string), (yyvsp[0].string));
		}
#line 4831 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 314: /* strfmt_args: string strfmt_va_args  */
#line 1736 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                        {
			(yyval.strfmtArgs).format = strdup((yyvsp[-1].string));
			(yyval.strfmtArgs).capacity = (yyvsp[0].strfmtArgs).capacity;
			(yyval.strfmtArgs).nbArgs = (yyvsp[0].strfmtArgs).nbArgs;
			(yyval.strfmtArgs).args = (yyvsp[0].strfmtArgs).args;
		}
#line 4842 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 315: /* strfmt_va_args: %empty  */
#line 1744 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			initStrFmtArgList(&(yyval.strfmtArgs));
		}
#line 4850 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 316: /* strfmt_va_args: strfmt_va_args "," const_no_str  */
#line 1747 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                      {
			size_t i = nextStrFmtArgListIndex(&(yyvsp[-2].strfmtArgs));

			(yyvsp[-2].strfmtArgs).args[i].number = (yyvsp[0].constValue);
			(yyvsp[-2].strfmtArgs).args[i].isNumeric = true;
			(yyval.strfmtArgs) = (yyvsp[-2].strfmtArgs);
		}
#line 4862 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 317: /* strfmt_va_args: strfmt_va_args "," string  */
#line 1754 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                {
			size_t i = nextStrFmtArgListIndex(&(yyvsp[-2].strfmtArgs));

			(yyvsp[-2].strfmtArgs).args[i].string = strdup((yyvsp[0].string));
			(yyvsp[-2].strfmtArgs).args[i].isNumeric = false;
			(yyval.strfmtArgs) = (yyvsp[-2].strfmtArgs);
		}
#line 4874 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 318: /* section: "SECTION" sectmod string "," sectiontype sectorg sectattrs  */
#line 1763 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                                     {
			sect_NewSection((yyvsp[-4].string), (enum SectionType)(yyvsp[-2].constValue), (yyvsp[-1].constValue), &(yyvsp[0].sectSpec), (yyvsp[-5].sectMod));
		}
#line 4882 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 319: /* sectmod: %empty  */
#line 1768 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.sectMod) = SECTION_NORMAL; }
#line 4888 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 320: /* sectmod: "UNION"  */
#line 1769 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.sectMod) = SECTION_UNION; }
#line 4894 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 321: /* sectmod: "FRAGMENT"  */
#line 1770 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                 { (yyval.sectMod) = SECTION_FRAGMENT; }
#line 4900 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 322: /* sectiontype: "WRAM0"  */
#line 1773 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                               { (yyval.constValue) = SECTTYPE_WRAM0; }
#line 4906 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 323: /* sectiontype: "VRAM"  */
#line 1774 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.constValue) = SECTTYPE_VRAM; }
#line 4912 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 324: /* sectiontype: "ROMX"  */
#line 1775 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.constValue) = SECTTYPE_ROMX; }
#line 4918 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 325: /* sectiontype: "ROM0"  */
#line 1776 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.constValue) = SECTTYPE_ROM0; }
#line 4924 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 326: /* sectiontype: "HRAM"  */
#line 1777 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.constValue) = SECTTYPE_HRAM; }
#line 4930 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 327: /* sectiontype: "WRAMX"  */
#line 1778 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                               { (yyval.constValue) = SECTTYPE_WRAMX; }
#line 4936 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 328: /* sectiontype: "SRAM"  */
#line 1779 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                              { (yyval.constValue) = SECTTYPE_SRAM; }
#line 4942 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 329: /* sectiontype: "OAM"  */
#line 1780 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { (yyval.constValue) = SECTTYPE_OAM; }
#line 4948 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 330: /* sectorg: %empty  */
#line 1783 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.constValue) = -1; }
#line 4954 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 331: /* sectorg: "[" uconst "]"  */
#line 1784 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           {
			if ((yyvsp[-1].constValue) < 0 || (yyvsp[-1].constValue) >= 0x10000) {
				error("Address $%x is not 16-bit\n", (yyvsp[-1].constValue));
				(yyval.constValue) = -1;
			} else {
				(yyval.constValue) = (yyvsp[-1].constValue);
			}
		}
#line 4967 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 332: /* sectattrs: %empty  */
#line 1794 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         {
			(yyval.sectSpec).alignment = 0;
			(yyval.sectSpec).alignOfs = 0;
			(yyval.sectSpec).bank = -1;
		}
#line 4977 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 333: /* sectattrs: sectattrs "," "ALIGN" "[" align_spec "]"  */
#line 1799 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                            {
			(yyval.sectSpec).alignment = (yyvsp[-1].alignSpec).alignment;
			(yyval.sectSpec).alignOfs = (yyvsp[-1].alignSpec).alignOfs;
		}
#line 4986 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 334: /* sectattrs: sectattrs "," "BANK" "[" uconst "]"  */
#line 1803 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                       {
			// We cannot check the validity of this now
			(yyval.sectSpec).bank = (yyvsp[-1].constValue);
		}
#line 4995 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 383: /* z80_adc: "adc" op_a_n  */
#line 1861 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xCE);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5004 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 384: /* z80_adc: "adc" op_a_r  */
#line 1865 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x88 | (yyvsp[0].constValue)); }
#line 5010 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 385: /* z80_add: "add" op_a_n  */
#line 1868 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xC6);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5019 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 386: /* z80_add: "add" op_a_r  */
#line 1872 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x80 | (yyvsp[0].constValue)); }
#line 5025 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 387: /* z80_add: "add" "hl" "," reg_ss  */
#line 1873 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     { sect_AbsByte(0x09 | ((yyvsp[0].constValue) << 4)); }
#line 5031 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 388: /* z80_add: "add" "sp" "," reloc_8bit  */
#line 1874 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                         {
			sect_AbsByte(0xE8);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5040 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 389: /* z80_and: "and" op_a_n  */
#line 1881 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xE6);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5049 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 390: /* z80_and: "and" op_a_r  */
#line 1885 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0xA0 | (yyvsp[0].constValue)); }
#line 5055 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 391: /* z80_bit: "bit" const_3bit "," reg_r  */
#line 1888 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x40 | ((yyvsp[-2].constValue) << 3) | (yyvsp[0].constValue));
		}
#line 5064 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 392: /* z80_call: "call" reloc_16bit  */
#line 1894 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         {
			sect_AbsByte(0xCD);
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5073 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 393: /* z80_call: "call" ccode_expr "," reloc_16bit  */
#line 1898 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                            {
			sect_AbsByte(0xC4 | ((yyvsp[-2].constValue) << 3));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5082 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 394: /* z80_ccf: "ccf"  */
#line 1904 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x3F); }
#line 5088 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 395: /* z80_cp: "cp" op_a_n  */
#line 1907 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xFE);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5097 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 396: /* z80_cp: "cp" op_a_r  */
#line 1911 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { sect_AbsByte(0xB8 | (yyvsp[0].constValue)); }
#line 5103 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 397: /* z80_cpl: "cpl"  */
#line 1914 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x2F); }
#line 5109 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 398: /* z80_daa: "daa"  */
#line 1917 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x27); }
#line 5115 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 399: /* z80_dec: "dec" reg_r  */
#line 1920 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { sect_AbsByte(0x05 | ((yyvsp[0].constValue) << 3)); }
#line 5121 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 400: /* z80_dec: "dec" reg_ss  */
#line 1921 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x0B | ((yyvsp[0].constValue) << 4)); }
#line 5127 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 401: /* z80_di: "di"  */
#line 1924 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { sect_AbsByte(0xF3); }
#line 5133 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 402: /* z80_ei: "ei"  */
#line 1927 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { sect_AbsByte(0xFB); }
#line 5139 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 403: /* z80_halt: "halt"  */
#line 1930 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             {
			sect_AbsByte(0x76);
			if (haltNop) {
				if (warnOnHaltNop) {
					warnOnHaltNop = false;
					warning(WARNING_OBSOLETE,
						"Automatic `nop` after `halt` is deprecated\n");
				}
				sect_AbsByte(0x00);
			}
		}
#line 5155 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 404: /* z80_inc: "inc" reg_r  */
#line 1943 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { sect_AbsByte(0x04 | ((yyvsp[0].constValue) << 3)); }
#line 5161 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 405: /* z80_inc: "inc" reg_ss  */
#line 1944 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x03 | ((yyvsp[0].constValue) << 4)); }
#line 5167 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 406: /* z80_jp: "jp" reloc_16bit  */
#line 1947 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			sect_AbsByte(0xC3);
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5176 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 407: /* z80_jp: "jp" ccode_expr "," reloc_16bit  */
#line 1951 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                          {
			sect_AbsByte(0xC2 | ((yyvsp[-2].constValue) << 3));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5185 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 408: /* z80_jp: "jp" "hl"  */
#line 1955 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                     {
			sect_AbsByte(0xE9);
		}
#line 5193 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 409: /* z80_jr: "jr" reloc_16bit  */
#line 1960 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			sect_AbsByte(0x18);
			sect_PCRelByte(&(yyvsp[0].expr), 1);
		}
#line 5202 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 410: /* z80_jr: "jr" ccode_expr "," reloc_16bit  */
#line 1964 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                          {
			sect_AbsByte(0x20 | ((yyvsp[-2].constValue) << 3));
			sect_PCRelByte(&(yyvsp[0].expr), 1);
		}
#line 5211 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 411: /* z80_ldi: "ldi" "[" "hl" "]" "," T_MODE_A  */
#line 1970 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                         {
			sect_AbsByte(0x02 | (2 << 4));
		}
#line 5219 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 412: /* z80_ldi: "ldi" T_MODE_A "," "[" "hl" "]"  */
#line 1973 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                         {
			sect_AbsByte(0x0A | (2 << 4));
		}
#line 5227 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 413: /* z80_ldd: "ldd" "[" "hl" "]" "," T_MODE_A  */
#line 1978 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                         {
			sect_AbsByte(0x02 | (3 << 4));
		}
#line 5235 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 414: /* z80_ldd: "ldd" T_MODE_A "," "[" "hl" "]"  */
#line 1981 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                         {
			sect_AbsByte(0x0A | (3 << 4));
		}
#line 5243 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 415: /* z80_ldio: "ldh" T_MODE_A "," op_mem_ind  */
#line 1986 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        {
			rpn_CheckHRAM(&(yyvsp[0].expr), &(yyvsp[0].expr));

			sect_AbsByte(0xF0);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5254 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 416: /* z80_ldio: "ldh" op_mem_ind "," T_MODE_A  */
#line 1992 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        {
			rpn_CheckHRAM(&(yyvsp[-2].expr), &(yyvsp[-2].expr));

			sect_AbsByte(0xE0);
			sect_RelByte(&(yyvsp[-2].expr), 1);
		}
#line 5265 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 417: /* z80_ldio: "ldh" T_MODE_A "," c_ind  */
#line 1998 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			sect_AbsByte(0xF2);
		}
#line 5273 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 418: /* z80_ldio: "ldh" c_ind "," T_MODE_A  */
#line 2001 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			sect_AbsByte(0xE2);
		}
#line 5281 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 420: /* c_ind: "[" relocexpr "+" T_MODE_C "]"  */
#line 2007 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                {
			if (!rpn_isKnown(&(yyvsp[-3].expr)) || (yyvsp[-3].expr).val != 0xFF00)
				error("Expected constant expression equal to $FF00 for \"$ff00+c\"\n");
		}
#line 5290 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 429: /* z80_ld_hl: "ld" "hl" "," "sp" reloc_8bit_offset  */
#line 2023 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                                         {
			sect_AbsByte(0xF8);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5299 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 430: /* z80_ld_hl: "ld" "hl" "," reloc_16bit  */
#line 2027 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                         {
			sect_AbsByte(0x01 | (REG_HL << 4));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5308 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 431: /* z80_ld_sp: "ld" "sp" "," "hl"  */
#line 2033 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       { sect_AbsByte(0xF9); }
#line 5314 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 432: /* z80_ld_sp: "ld" "sp" "," reloc_16bit  */
#line 2034 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                         {
			sect_AbsByte(0x01 | (REG_SP << 4));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5323 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 433: /* z80_ld_mem: "ld" op_mem_ind "," "sp"  */
#line 2040 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                        {
			sect_AbsByte(0x08);
			sect_RelWord(&(yyvsp[-2].expr), 1);
		}
#line 5332 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 434: /* z80_ld_mem: "ld" op_mem_ind "," T_MODE_A  */
#line 2044 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                       {
			if (optimizeLoads && rpn_isKnown(&(yyvsp[-2].expr))
			 && (yyvsp[-2].expr).val >= 0xFF00) {
				if (warnOnLdOpt) {
					warnOnLdOpt = false;
					warning(WARNING_OBSOLETE,
						"Automatic `ld` to `ldh` optimization is deprecated\n");
				}
				sect_AbsByte(0xE0);
				sect_AbsByte((yyvsp[-2].expr).val & 0xFF);
				rpn_Free(&(yyvsp[-2].expr));
			} else {
				sect_AbsByte(0xEA);
				sect_RelWord(&(yyvsp[-2].expr), 1);
			}
		}
#line 5353 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 435: /* z80_ld_cind: "ld" c_ind "," T_MODE_A  */
#line 2062 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                  {
			sect_AbsByte(0xE2);
		}
#line 5361 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 436: /* z80_ld_rr: "ld" reg_rr "," T_MODE_A  */
#line 2067 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                   {
			sect_AbsByte(0x02 | ((yyvsp[-2].constValue) << 4));
		}
#line 5369 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 437: /* z80_ld_r: "ld" reg_r "," reloc_8bit  */
#line 2072 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			sect_AbsByte(0x06 | ((yyvsp[-2].constValue) << 3));
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5378 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 438: /* z80_ld_r: "ld" reg_r "," reg_r  */
#line 2076 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			if (((yyvsp[-2].constValue) == REG_HL_IND) && ((yyvsp[0].constValue) == REG_HL_IND))
				error("LD [HL],[HL] not a valid instruction\n");
			else
				sect_AbsByte(0x40 | ((yyvsp[-2].constValue) << 3) | (yyvsp[0].constValue));
		}
#line 5389 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 439: /* z80_ld_a: "ld" reg_r "," c_ind  */
#line 2084 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                               {
			if ((yyvsp[-2].constValue) == REG_A)
				sect_AbsByte(0xF2);
			else
				error("Destination operand must be A\n");
		}
#line 5400 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 440: /* z80_ld_a: "ld" reg_r "," reg_rr  */
#line 2090 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                {
			if ((yyvsp[-2].constValue) == REG_A)
				sect_AbsByte(0x0A | ((yyvsp[0].constValue) << 4));
			else
				error("Destination operand must be A\n");
		}
#line 5411 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 441: /* z80_ld_a: "ld" reg_r "," op_mem_ind  */
#line 2096 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                    {
			if ((yyvsp[-2].constValue) == REG_A) {
				if (optimizeLoads && rpn_isKnown(&(yyvsp[0].expr))
				 && (yyvsp[0].expr).val >= 0xFF00) {
					if (warnOnLdOpt) {
						warnOnLdOpt = false;
						warning(WARNING_OBSOLETE,
							"Automatic `ld` to `ldh` optimization is deprecated\n");
					}
					sect_AbsByte(0xF0);
					sect_AbsByte((yyvsp[0].expr).val & 0xFF);
					rpn_Free(&(yyvsp[0].expr));
				} else {
					sect_AbsByte(0xFA);
					sect_RelWord(&(yyvsp[0].expr), 1);
				}
			} else {
				error("Destination operand must be A\n");
				rpn_Free(&(yyvsp[0].expr));
			}
		}
#line 5437 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 442: /* z80_ld_ss: "ld" "bc" "," reloc_16bit  */
#line 2119 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                         {
			sect_AbsByte(0x01 | (REG_BC << 4));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5446 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 443: /* z80_ld_ss: "ld" "de" "," reloc_16bit  */
#line 2123 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                         {
			sect_AbsByte(0x01 | (REG_DE << 4));
			sect_RelWord(&(yyvsp[0].expr), 1);
		}
#line 5455 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 444: /* z80_nop: "nop"  */
#line 2131 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x00); }
#line 5461 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 445: /* z80_or: "or" op_a_n  */
#line 2134 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xF6);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5470 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 446: /* z80_or: "or" op_a_r  */
#line 2138 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  { sect_AbsByte(0xB0 | (yyvsp[0].constValue)); }
#line 5476 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 447: /* z80_pop: "pop" reg_tt  */
#line 2141 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0xC1 | ((yyvsp[0].constValue) << 4)); }
#line 5482 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 448: /* z80_push: "push" reg_tt  */
#line 2144 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                    { sect_AbsByte(0xC5 | ((yyvsp[0].constValue) << 4)); }
#line 5488 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 449: /* z80_res: "res" const_3bit "," reg_r  */
#line 2147 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x80 | ((yyvsp[-2].constValue) << 3) | (yyvsp[0].constValue));
		}
#line 5497 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 450: /* z80_ret: "ret"  */
#line 2153 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0xC9); }
#line 5503 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 451: /* z80_ret: "ret" ccode_expr  */
#line 2154 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       { sect_AbsByte(0xC0 | ((yyvsp[0].constValue) << 3)); }
#line 5509 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 452: /* z80_reti: "reti"  */
#line 2157 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_AbsByte(0xD9); }
#line 5515 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 453: /* z80_rl: "rl" reg_r  */
#line 2160 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                 {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x10 | (yyvsp[0].constValue));
		}
#line 5524 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 454: /* z80_rla: "rla"  */
#line 2166 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x17); }
#line 5530 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 455: /* z80_rlc: "rlc" reg_r  */
#line 2169 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x00 | (yyvsp[0].constValue));
		}
#line 5539 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 456: /* z80_rlca: "rlca"  */
#line 2175 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_AbsByte(0x07); }
#line 5545 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 457: /* z80_rr: "rr" reg_r  */
#line 2178 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                 {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x18 | (yyvsp[0].constValue));
		}
#line 5554 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 458: /* z80_rra: "rra"  */
#line 2184 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x1F); }
#line 5560 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 459: /* z80_rrc: "rrc" reg_r  */
#line 2187 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x08 | (yyvsp[0].constValue));
		}
#line 5569 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 460: /* z80_rrca: "rrca"  */
#line 2193 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { sect_AbsByte(0x0F); }
#line 5575 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 461: /* z80_rst: "rst" reloc_8bit  */
#line 2196 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                       {
			rpn_CheckRST(&(yyvsp[0].expr), &(yyvsp[0].expr));
			if (!rpn_isKnown(&(yyvsp[0].expr)))
				sect_RelByte(&(yyvsp[0].expr), 0);
			else
				sect_AbsByte(0xC7 | (yyvsp[0].expr).val);
			rpn_Free(&(yyvsp[0].expr));
		}
#line 5588 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 462: /* z80_sbc: "sbc" op_a_n  */
#line 2206 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xDE);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5597 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 463: /* z80_sbc: "sbc" op_a_r  */
#line 2210 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x98 | (yyvsp[0].constValue)); }
#line 5603 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 464: /* z80_scf: "scf"  */
#line 2213 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { sect_AbsByte(0x37); }
#line 5609 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 465: /* z80_set: "set" const_3bit "," reg_r  */
#line 2216 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                     {
			sect_AbsByte(0xCB);
			sect_AbsByte(0xC0 | ((yyvsp[-2].constValue) << 3) | (yyvsp[0].constValue));
		}
#line 5618 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 466: /* z80_sla: "sla" reg_r  */
#line 2222 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x20 | (yyvsp[0].constValue));
		}
#line 5627 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 467: /* z80_sra: "sra" reg_r  */
#line 2228 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x28 | (yyvsp[0].constValue));
		}
#line 5636 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 468: /* z80_srl: "srl" reg_r  */
#line 2234 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                  {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x38 | (yyvsp[0].constValue));
		}
#line 5645 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 469: /* z80_stop: "stop"  */
#line 2240 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             {
			sect_AbsByte(0x10);
			sect_AbsByte(0x00);
		}
#line 5654 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 470: /* z80_stop: "stop" reloc_8bit  */
#line 2244 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                        {
			sect_AbsByte(0x10);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5663 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 471: /* z80_sub: "sub" op_a_n  */
#line 2250 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xD6);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5672 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 472: /* z80_sub: "sub" op_a_r  */
#line 2254 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0x90 | (yyvsp[0].constValue)); }
#line 5678 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 473: /* z80_swap: "swap" reg_r  */
#line 2257 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xCB);
			sect_AbsByte(0x30 | (yyvsp[0].constValue));
		}
#line 5687 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 474: /* z80_xor: "xor" op_a_n  */
#line 2263 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   {
			sect_AbsByte(0xEE);
			sect_RelByte(&(yyvsp[0].expr), 1);
		}
#line 5696 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 475: /* z80_xor: "xor" op_a_r  */
#line 2267 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                   { sect_AbsByte(0xA8 | (yyvsp[0].constValue)); }
#line 5702 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 476: /* op_mem_ind: "[" reloc_16bit "]"  */
#line 2270 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                                { (yyval.expr) = (yyvsp[-1].expr); }
#line 5708 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 478: /* op_a_r: T_MODE_A "," reg_r  */
#line 2274 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                         { (yyval.constValue) = (yyvsp[0].constValue); }
#line 5714 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 480: /* op_a_n: T_MODE_A "," reloc_8bit  */
#line 2278 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              { (yyval.expr) = (yyvsp[0].expr); }
#line 5720 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 496: /* ccode_expr: "!" ccode_expr  */
#line 2310 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                           {
			(yyval.constValue) = (yyvsp[0].constValue) ^ 1;
		}
#line 5728 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 497: /* ccode: "nz"  */
#line 2315 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                          { (yyval.constValue) = CC_NZ; }
#line 5734 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 498: /* ccode: "z"  */
#line 2316 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                         { (yyval.constValue) = CC_Z; }
#line 5740 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 499: /* ccode: "nc"  */
#line 2317 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                          { (yyval.constValue) = CC_NC; }
#line 5746 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 500: /* ccode: "c"  */
#line 2318 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = CC_C; }
#line 5752 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 501: /* reg_r: T_MODE_B  */
#line 2321 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_B; }
#line 5758 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 502: /* reg_r: T_MODE_C  */
#line 2322 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_C; }
#line 5764 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 503: /* reg_r: T_MODE_D  */
#line 2323 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_D; }
#line 5770 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 504: /* reg_r: T_MODE_E  */
#line 2324 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_E; }
#line 5776 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 505: /* reg_r: T_MODE_H  */
#line 2325 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_H; }
#line 5782 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 506: /* reg_r: T_MODE_L  */
#line 2326 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_L; }
#line 5788 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 507: /* reg_r: "[" "hl" "]"  */
#line 2327 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              { (yyval.constValue) = REG_HL_IND; }
#line 5794 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 508: /* reg_r: T_MODE_A  */
#line 2328 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                           { (yyval.constValue) = REG_A; }
#line 5800 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 509: /* reg_tt: "bc"  */
#line 2331 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_BC; }
#line 5806 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 510: /* reg_tt: "de"  */
#line 2332 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_DE; }
#line 5812 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 511: /* reg_tt: "hl"  */
#line 2333 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_HL; }
#line 5818 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 512: /* reg_tt: "af"  */
#line 2334 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_AF; }
#line 5824 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 513: /* reg_ss: "bc"  */
#line 2337 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_BC; }
#line 5830 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 514: /* reg_ss: "de"  */
#line 2338 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_DE; }
#line 5836 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 515: /* reg_ss: "hl"  */
#line 2339 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_HL; }
#line 5842 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 516: /* reg_ss: "sp"  */
#line 2340 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                            { (yyval.constValue) = REG_SP; }
#line 5848 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 517: /* reg_rr: "[" "bc" "]"  */
#line 2343 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              { (yyval.constValue) = REG_BC_IND; }
#line 5854 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 518: /* reg_rr: "[" "de" "]"  */
#line 2344 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                                              { (yyval.constValue) = REG_DE_IND; }
#line 5860 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 519: /* reg_rr: hl_ind_inc  */
#line 2345 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { (yyval.constValue) = REG_HL_INDINC; }
#line 5866 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;

  case 520: /* reg_rr: hl_ind_dec  */
#line 2346 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"
                             { (yyval.constValue) = REG_HL_INDDEC; }
#line 5872 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"
    break;


#line 5876 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/215b8502ae944a74/parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= T_EOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == T_EOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 2357 "/home/desttinghim/Code/random/rgbds/src/asm/parser.y"


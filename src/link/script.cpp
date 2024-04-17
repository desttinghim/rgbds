// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "script.hpp"


// Unqualified %code blocks.
#line 13 "/home/desttinghim/Code/random/rgbds/src/link/script.y"

	#include <algorithm>
	#include <array>
	#include <assert.h>
	#include <bit>
	#include <cinttypes>
	#include <fstream>
	#include <locale>
	#include <stdio.h>
	#include <string_view>
	#include <vector>

	#include "helpers.hpp"
	#include "itertools.hpp"
	#include "util.hpp"

	#include "link/main.hpp"
	#include "link/section.hpp"

	using namespace std::literals;

	static void includeFile(std::string &&path);
	static void incLineNo(void);

	static void setSectionType(SectionType type);
	static void setSectionType(SectionType type, uint32_t bank);
	static void setAddr(uint32_t addr);
	static void makeAddrFloating(void);
	static void alignTo(uint32_t alignment, uint32_t offset);
	static void pad(uint32_t length);
	static void placeSection(std::string const &name, bool isOptional);

	static yy::parser::symbol_type yylex(void);

	struct Keyword {
		std::string_view name;
		yy::parser::symbol_type (* tokenGen)(void);
	};
#line 62 "/home/desttinghim/Code/random/rgbds/src/link/script.y"

	static std::array keywords{
		Keyword{"ORG"sv,      yy::parser::make_ORG},
		Keyword{"FLOATING"sv, yy::parser::make_FLOATING},
		Keyword{"INCLUDE"sv,  yy::parser::make_INCLUDE},
		Keyword{"ALIGN"sv,    yy::parser::make_ALIGN},
		Keyword{"DS"sv,       yy::parser::make_DS},
		Keyword{"OPTIONAL"sv, yy::parser::make_OPTIONAL},
	};

#line 96 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif



// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 169 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"

  /// Build a parser object.
  parser::parser ()
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr)
#else

#endif
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/



  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_kind_type
  parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_section_type: // section_type
        value.YY_MOVE_OR_COPY< SectionType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_optional: // optional
        value.YY_MOVE_OR_COPY< bool > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_string: // string
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_number: // number
        value.YY_MOVE_OR_COPY< uint32_t > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s)
  {
    switch (that.kind ())
    {
      case symbol_kind::S_section_type: // section_type
        value.move< SectionType > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_optional: // optional
        value.move< bool > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_string: // string
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_number: // number
        value.move< uint32_t > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_section_type: // section_type
        value.copy< SectionType > (that.value);
        break;

      case symbol_kind::S_optional: // optional
        value.copy< bool > (that.value);
        break;

      case symbol_kind::S_string: // string
        value.copy< std::string > (that.value);
        break;

      case symbol_kind::S_number: // number
        value.copy< uint32_t > (that.value);
        break;

      default:
        break;
    }

    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_section_type: // section_type
        value.move< SectionType > (that.value);
        break;

      case symbol_kind::S_optional: // optional
        value.move< bool > (that.value);
        break;

      case symbol_kind::S_string: // string
        value.move< std::string > (that.value);
        break;

      case symbol_kind::S_number: // number
        value.move< uint32_t > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " (";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex ());
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_section_type: // section_type
        yylhs.value.emplace< SectionType > ();
        break;

      case symbol_kind::S_optional: // optional
        yylhs.value.emplace< bool > ();
        break;

      case symbol_kind::S_string: // string
        yylhs.value.emplace< std::string > ();
        break;

      case symbol_kind::S_number: // number
        yylhs.value.emplace< uint32_t > ();
        break;

      default:
        break;
    }



      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 4: // line: "INCLUDE" string newline
#line 84 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                             { includeFile(std::move(yystack_[1].value.as < std::string > ())); }
#line 630 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 5: // line: directive newline
#line 85 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                        { incLineNo(); }
#line 636 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 6: // line: newline
#line 86 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
              { incLineNo(); }
#line 642 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 7: // line: error newline
#line 87 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                    { yyerrok; incLineNo(); }
#line 648 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 8: // directive: section_type
#line 90 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                        { setSectionType(yystack_[0].value.as < SectionType > ()); }
#line 654 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 9: // directive: section_type number
#line 91 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                               { setSectionType(yystack_[1].value.as < SectionType > (), yystack_[0].value.as < uint32_t > ()); }
#line 660 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 10: // directive: "FLOATING"
#line 92 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                    { makeAddrFloating(); }
#line 666 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 11: // directive: "ORG" number
#line 93 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                      { setAddr(yystack_[0].value.as < uint32_t > ()); }
#line 672 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 12: // directive: "ALIGN" number
#line 94 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                        { alignTo(yystack_[0].value.as < uint32_t > (), 0); }
#line 678 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 13: // directive: "ALIGN" number "," number
#line 95 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                                     { alignTo(yystack_[2].value.as < uint32_t > (), yystack_[0].value.as < uint32_t > ()); }
#line 684 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 14: // directive: "DS" number
#line 96 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                     { pad(yystack_[0].value.as < uint32_t > ()); }
#line 690 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 15: // directive: string optional
#line 97 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                           { placeSection(yystack_[1].value.as < std::string > (), yystack_[0].value.as < bool > ()); }
#line 696 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 16: // optional: %empty
#line 100 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                 { yylhs.value.as < bool > () = false; }
#line 702 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;

  case 17: // optional: "OPTIONAL"
#line 101 "/home/desttinghim/Code/random/rgbds/src/link/script.y"
                   { yylhs.value.as < bool > () = true; }
#line 708 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"
    break;


#line 712 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        std::string msg = YY_("syntax error");
        error (YY_MOVE (msg));
      }


    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;


      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.what ());
  }

#if YYDEBUG || 0
  const char *
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytname_[yysymbol];
  }
#endif // #if YYDEBUG || 0









  const signed char parser::yypact_ninf_ = -9;

  const signed char parser::yytable_ninf_ = -3;

  const signed char
  parser::yypact_[] =
  {
       0,    -1,    -9,    -8,    -9,     1,    -2,     2,     5,     4,
      17,     0,    15,    -9,    -9,    16,    18,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,     8,    -9
  };

  const signed char
  parser::yydefact_[] =
  {
       0,     0,     6,     0,    10,     0,     0,     0,    16,     8,
       0,     0,     0,     7,    11,     0,    12,    14,    17,    15,
       9,     1,     3,     5,     4,     0,    13
  };

  const signed char
  parser::yypgoto_[] =
  {
      -9,    10,    -9,    -9,    -9
  };

  const signed char
  parser::yydefgoto_[] =
  {
       0,    10,    11,    12,    19
  };

  const signed char
  parser::yytable_[] =
  {
      -2,     1,    13,     2,    14,     3,     4,     5,     6,     7,
      16,     8,    15,     9,    17,    18,    20,    21,    23,    24,
      26,    22,    25
  };

  const signed char
  parser::yycheck_[] =
  {
       0,     1,     3,     3,    12,     5,     6,     7,     8,     9,
      12,    11,    11,    13,    12,    10,    12,     0,     3,     3,
      12,    11,     4
  };

  const signed char
  parser::yystos_[] =
  {
       0,     1,     3,     5,     6,     7,     8,     9,    11,    13,
      15,    16,    17,     3,    12,    11,    12,    12,    10,    18,
      12,     0,    15,     3,     3,     4,    12
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    14,    15,    15,    16,    16,    16,    16,    17,    17,
      17,    17,    17,    17,    17,    17,    18,    18
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     0,     2,     3,     2,     1,     2,     1,     2,
       1,     2,     2,     4,     2,     2,     0,     1
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "newline", "\",\"",
  "\"ORG\"", "\"FLOATING\"", "\"INCLUDE\"", "\"ALIGN\"", "\"DS\"",
  "\"OPTIONAL\"", "string", "number", "section_type", "$accept", "lines",
  "line", "directive", "optional", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const signed char
  parser::yyrline_[] =
  {
       0,    80,    80,    81,    84,    85,    86,    87,    90,    91,
      92,    93,    94,    95,    96,    97,   100,   101
  };

  void
  parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 1023 "/home/desttinghim/Code/random/rgbds/zig-cache/tmp/fbf1518776d1881e/script.cpp"

#line 104 "/home/desttinghim/Code/random/rgbds/src/link/script.y"


#define scriptError(context, fmt, ...) \
	::error(NULL, 0, "%s(%" PRIu32 "): " fmt, \
	        context.path.c_str(), context.lineNo __VA_OPT__(,) __VA_ARGS__)

// Lexer.

struct LexerStackEntry {
	std::filebuf file;
	std::string path;
	uint32_t lineNo;

	explicit LexerStackEntry(std::string &&path_) : file(), path(path_), lineNo(1) {}
};
static std::vector<LexerStackEntry> lexerStack;
static bool atEof;

void yy::parser::error(std::string const &msg) {
	auto const &script = lexerStack.back();
	scriptError(script, "%s", msg.c_str());
}

static void includeFile(std::string &&path) {
	// `emplace_back` can invalidate references to the stack's elements!
	// This is why `newContext` must be gotten before `prevContext`.
	auto &newContext = lexerStack.emplace_back(std::move(path));
	auto &prevContext = lexerStack[lexerStack.size() - 2];

	if (!newContext.file.open(newContext.path, std::ios_base::in)) {
		// The order is important: report the error, increment the line number, modify the stack!
		scriptError(prevContext, "Could not open included linker script \"%s\"",
		            newContext.path.c_str());
		++prevContext.lineNo;
		lexerStack.pop_back();
	} else {
		// The lexer will use the new entry to lex the next token.
		++prevContext.lineNo;
	}
}

static void incLineNo(void) {
	++lexerStack.back().lineNo;
}

static bool isWhiteSpace(int c) {
	return c == ' ' || c == '\t';
}

static bool isNewline(int c) {
	return c == '\r' || c == '\n';
}

static bool isIdentChar(int c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

static bool isDecDigit(int c) {
	return c >= '0' && c <= '9';
}

static bool isBinDigit(int c) {
	return c >= '0' && c <= '1';
}

static bool isHexDigit(int c) {
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static uint8_t parseHexDigit(int c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	} else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	} else {
		unreachable_();
	}
}

yy::parser::symbol_type yylex(void) {
try_again: // Can't use a `do {} while(0)` loop, otherwise compilers (wrongly) think it can end.
	auto &context = lexerStack.back();
	auto c = context.file.sbumpc();

	// First, skip leading whitespace.
	while (isWhiteSpace(c)) {
		c = context.file.sbumpc();
	}
	// Then, skip a comment if applicable.
	if (c == ';') {
		while (!isNewline(c)) {
			c = context.file.sbumpc();
		}
	}

	// Alright, what token should we return?
	if (c == EOF) {
		// Basically yywrap().
		if (lexerStack.size() != 1) {
			lexerStack.pop_back();
			goto try_again;
		} else if (!atEof) {
			// Inject a newline at EOF, to avoid errors for files that don't end with one.
			atEof = true;
			return yy::parser::make_newline();
		} else {
			return yy::parser::make_YYEOF();
		}
	} else if (c == ',') {
		return yy::parser::make_COMMA();
	} else if (isNewline(c)) {
		// Handle CRLF.
		if (c == '\r' && context.file.sgetc() == '\n') {
			context.file.sbumpc();
		}
		return yy::parser::make_newline();
	} else if (c == '"') {
		std::string str;

		for (c = context.file.sgetc(); c != '"'; c = context.file.sgetc()) {
			if (c == EOF || isNewline(c)) {
				scriptError(context, "Unterminated string");
				break;
			}
			context.file.sbumpc();
			if (c == '\\') {
				c = context.file.sgetc();
				if (c == EOF || isNewline(c)) {
					scriptError(context, "Unterminated string");
					break;
				} else if (c == 'n') {
					c = '\n';
				} else if (c == 'r') {
					c = '\r';
				} else if (c == 't') {
					c = '\t';
				} else if (c != '\\' && c != '"') {
					scriptError(context, "Cannot escape character %s", printChar(c));
				}
				context.file.sbumpc();
			}
			str.push_back(c);
		}
		context.file.sbumpc(); // Consume the closing quote.

		return yy::parser::make_string(std::move(str));
	} else if (c == '$') {
		c = context.file.sgetc();
		if (!isHexDigit(c)) {
			scriptError(context, "No hexadecimal digits found after '$'");
			return yy::parser::make_number(0);
		}

		uint32_t number = parseHexDigit(c);
		context.file.sbumpc();
		for (c = context.file.sgetc(); isHexDigit(c); c = context.file.sgetc()) {
			number = number * 16 + parseHexDigit(c);
			context.file.sbumpc();
		}
		return yy::parser::make_number(number);
	} else if (c == '%') {
		c = context.file.sgetc();
		if (!isBinDigit(c)) {
			scriptError(context, "No binary digits found after '%%'");
			return yy::parser::make_number(0);
		}

		uint32_t number = c - '0';
		context.file.sbumpc();
		for (c = context.file.sgetc(); isBinDigit(c); c = context.file.sgetc()) {
			number = number * 2 + (c - '0');
			context.file.sbumpc();
		}
		return yy::parser::make_number(number);
	} else if (isDecDigit(c)) {
		uint32_t number = c - '0';
		for (c = context.file.sgetc(); isDecDigit(c); c = context.file.sgetc()) {
			number = number * 10 + (c - '0');
		}
		return yy::parser::make_number(number);
	} else if (isIdentChar(c)) { // Note that we match these *after* digit characters!
		std::string ident;
		auto strUpperCmp = [](char cmp, char ref) {
			// `locale::classic()` yields the "C" locale.
			assert(!std::use_facet<std::ctype<char>>(std::locale::classic())
			       .is(std::ctype_base::lower, ref));
			return std::use_facet<std::ctype<char>>(std::locale::classic())
			       .toupper(cmp) == ref;
		};

		ident.push_back(c);
		for (c = context.file.sgetc(); isIdentChar(c); c = context.file.snextc()) {
			ident.push_back(c);
		}

		for (SectionType type : EnumSeq(SECTTYPE_INVALID)) {
			if (std::equal(RANGE(ident), RANGE(sectionTypeInfo[type].name), strUpperCmp)) {
				return yy::parser::make_section_type(type);
			}
		}

		for (Keyword const &keyword : keywords) {
			if (std::equal(RANGE(ident), RANGE(keyword.name), strUpperCmp)) {
				return keyword.tokenGen();
			}
		}

		scriptError(context, "Unknown keyword \"%s\"", ident.c_str());
		goto try_again; // Try lexing another token.
	} else {
		scriptError(context, "Unexpected character '%s'", printChar(c));
		// Keep reading characters until the EOL, to avoid reporting too many errors.
		for (c = context.file.sgetc(); !isNewline(c); c = context.file.sgetc()) {
			if (c == EOF) {
				break;
			}
		}
		goto try_again;
	}
	// Not marking as unreachable; this will generate a warning if any codepath forgets to return.
}

// Semantic actions.

static std::array<std::vector<uint16_t>, SECTTYPE_INVALID> curAddr;
static SectionType activeType; // Index into curAddr
static uint32_t activeBankIdx; // Index into curAddr[activeType]
static bool isPcFloating;
static uint16_t floatingAlignMask;
static uint16_t floatingAlignOffset;

static void setActiveTypeAndIdx(SectionType type, uint32_t idx) {
	activeType = type;
	activeBankIdx = idx;
	isPcFloating = false;
	if (curAddr[activeType].size() <= activeBankIdx) {
		curAddr[activeType].resize(activeBankIdx + 1, sectionTypeInfo[type].startAddr);
	}
}

static void setSectionType(SectionType type) {
	auto const &context = lexerStack.back();

	if (nbbanks(type) != 1) {
		scriptError(context, "A bank number must be specified for %s",
		            sectionTypeInfo[type].name.c_str());
		// Keep going with a default value for the bank index.
	}

	setActiveTypeAndIdx(type, 0); // There is only a single bank anyway, so just set the index to 0.
}

static void setSectionType(SectionType type, uint32_t bank) {
	auto const &context = lexerStack.back();
	auto const &typeInfo = sectionTypeInfo[type];

	if (bank < typeInfo.firstBank) {
		scriptError(context, "%s bank %" PRIu32 " doesn't exist (the minimum is %" PRIu32 ")",
		            typeInfo.name.c_str(), bank, typeInfo.firstBank);
		bank = typeInfo.firstBank;
	} else if (bank > typeInfo.lastBank) {
		scriptError(context, "%s bank %" PRIu32 " doesn't exist (the maximum is %" PRIu32 ")",
		            typeInfo.name.c_str(), bank, typeInfo.lastBank);
	}

	setActiveTypeAndIdx(type, bank - typeInfo.firstBank);
}

static void setAddr(uint32_t addr) {
	auto const &context = lexerStack.back();
	if (activeType == SECTTYPE_INVALID) {
		scriptError(context, "Cannot set the current address: no memory region is active");
		return;
	}

	auto &pc = curAddr[activeType][activeBankIdx];
	auto const &typeInfo = sectionTypeInfo[activeType];

	if (addr < pc) {
		scriptError(context, "Cannot decrease the current address (from $%04x to $%04x)", pc, addr);
	} else if (addr > endaddr(activeType)) { // Allow "one past the end" sections.
		scriptError(context, "Cannot set the current address to $%04" PRIx32 ": %s ends at $%04" PRIx16 "",
		            addr, typeInfo.name.c_str(), endaddr(activeType));
		pc = endaddr(activeType);
	} else {
		pc = addr;
	}
	isPcFloating = false;
}

static void makeAddrFloating(void) {
	auto const &context = lexerStack.back();
	if (activeType == SECTTYPE_INVALID) {
		scriptError(context, "Cannot make the current address floating: no memory region is active");
		return;
	}

	isPcFloating = true;
	floatingAlignMask = 0;
	floatingAlignOffset = 0;
}

static void alignTo(uint32_t alignment, uint32_t alignOfs) {
	auto const &context = lexerStack.back();
	if (activeType == SECTTYPE_INVALID) {
		scriptError(context, "Cannot align: no memory region is active");
		return;
	}

	if (isPcFloating) {
		if (alignment >= 16) {
			setAddr(floatingAlignOffset);
		} else {
			uint32_t alignSize = 1u << alignment;

			if (alignOfs >= alignSize) {
				scriptError(context, "Cannot align: The alignment offset (%" PRIu32
						      ") must be less than alignment size (%" PRIu32 ")\n",
					    alignOfs, alignSize);
				return;
			}

			floatingAlignMask = alignSize - 1;
			floatingAlignOffset = alignOfs % alignSize;
		}
		return;
	}

	auto const &typeInfo = sectionTypeInfo[activeType];
	auto &pc = curAddr[activeType][activeBankIdx];

	if (alignment > 16) {
		scriptError(context, "Cannot align: The alignment (%" PRIu32 ") must be less than 16\n",
			    alignment);
		return;
	}

	// Let it wrap around, this'll trip the final check if alignment == 16.
	uint16_t length = alignOfs - pc;

	if (alignment < 16) {
		uint32_t alignSize = 1u << alignment;

		if (alignOfs >= alignSize) {
			scriptError(context, "Cannot align: The alignment offset (%" PRIu32
					      ") must be less than alignment size (%" PRIu32 ")\n",
				    alignOfs, alignSize);
			return;
		}

		assert(pc >= typeInfo.startAddr);
		length %= alignSize;
	}

	if (uint16_t offset = pc - typeInfo.startAddr; length > typeInfo.size - offset) {
		scriptError(context, "Cannot align: the next suitable address after $%04"
				     PRIx16 " is $%04" PRIx16 ", past $%04" PRIx16,
			    pc, (uint16_t)(pc + length), (uint16_t)(endaddr(activeType) + 1));
		return;
	}

	pc += length;
}

static void pad(uint32_t length) {
	auto const &context = lexerStack.back();
	if (activeType == SECTTYPE_INVALID) {
		scriptError(context, "Cannot increase the current address: no memory region is active");
		return;
	}

	if (isPcFloating) {
		floatingAlignOffset = (floatingAlignOffset + length) & floatingAlignMask;
		return;
	}

	auto const &typeInfo = sectionTypeInfo[activeType];
	auto &pc = curAddr[activeType][activeBankIdx];

	assert(pc >= typeInfo.startAddr);
	if (uint16_t offset = pc - typeInfo.startAddr; length + offset > typeInfo.size) {
		scriptError(context, "Cannot increase the current address by %u bytes: only %u bytes to $%04" PRIx16,
		            length, typeInfo.size - offset, (uint16_t)(endaddr(activeType) + 1));
	} else {
		pc += length;
	}
}

static void placeSection(std::string const &name, bool isOptional) {
	auto const &context = lexerStack.back();
	if (activeType == SECTTYPE_INVALID) {
		scriptError(context, "No memory region has been specified to place section \"%s\" in",
		            name.c_str());
		return;
	}

	auto *section = sect_GetSection(name.c_str());
	if (!section) {
		if (!isOptional) {
			scriptError(context, "Unknown section \"%s\"", name.c_str());
		}
		return;
	}

	auto const &typeInfo = sectionTypeInfo[activeType];
	assert(section->offset == 0);
	// Check that the linker script doesn't contradict what the code says.
	if (section->type == SECTTYPE_INVALID) {
		// SDCC areas don't have a type assigned yet, so the linker script is used to give them one.
		for (Section *fragment = section; fragment; fragment = fragment->nextu) {
			fragment->type = activeType;
		}
	} else if (section->type != activeType) {
		scriptError(context, "\"%s\" is specified to be a %s section, but it is already a %s section",
		            name.c_str(), typeInfo.name.c_str(), sectionTypeInfo[section->type].name.c_str());
	}

	uint32_t bank = activeBankIdx + typeInfo.firstBank;
	if (section->isBankFixed && bank != section->bank) {
		scriptError(context, "The linker script places section \"%s\" in %s bank %" PRIu32 ", but it was already defined in bank %" PRIu32,
		            name.c_str(), sectionTypeInfo[section->type].name.c_str(), bank, section->bank);
	}
	section->isBankFixed = true;
	section->bank = bank;

	if (!isPcFloating) {
		uint16_t &org = curAddr[activeType][activeBankIdx];
		if (section->isAddressFixed && org != section->org) {
			scriptError(context, "The linker script assigns section \"%s\" to address $%04" PRIx16 ", but it was already at $%04" PRIx16,
			            name.c_str(), org, section->org);
		} else if (section->isAlignFixed && (org & section->alignMask) != section->alignOfs) {
			uint8_t alignment = std::countr_one(section->alignMask);
			scriptError(context, "The linker script assigns section \"%s\" to address $%04" PRIx16 ", but that would be ALIGN[%" PRIu8 ", %" PRIu16 "] instead of the requested ALIGN[%" PRIu8 ", %" PRIu16 "]",
			            name.c_str(), org, alignment, (uint16_t)(org & section->alignMask), alignment, section->alignOfs);
		}
		section->isAddressFixed = true;
		section->isAlignFixed = false; // This can't be set when the above is.
		section->org = org;

		uint16_t curOfs = org - typeInfo.startAddr;
		if (section->size > typeInfo.size - curOfs) {
			scriptError(context, "The linker script assigns section \"%s\" to address $%04" PRIx16 ", but then it would overflow %s by %" PRIx16 " bytes",
			            name.c_str(), org, typeInfo.name.c_str(),
			            (uint16_t)(section->size - (typeInfo.size - curOfs)));
			// Fill as much as possible without going out of bounds.
			org = typeInfo.startAddr + typeInfo.size;
		} else {
			org += section->size;
		}
	} else {
		section->isAddressFixed = false;
		section->isAlignFixed = floatingAlignMask != 0;
		section->alignMask = floatingAlignMask;
		section->alignOfs = floatingAlignOffset;

		floatingAlignOffset = (floatingAlignOffset + section->size) & floatingAlignMask;
	}
}

// External API.

void script_ProcessScript(char const *path) {
	activeType = SECTTYPE_INVALID;

	lexerStack.clear();
	atEof = false;
	auto &newContext = lexerStack.emplace_back(std::string(path));

	if (!newContext.file.open(newContext.path, std::ios_base::in)) {
		error(NULL, 0, "Could not open linker script \"%s\"", newContext.path.c_str());
		lexerStack.clear();
	} else {
		yy::parser linkerScriptParser;
		// We don't care about the return value, as any error increments the global error count,
		// which is what `main` checks.
		(void)linkerScriptParser.parse();

		// Free up working memory.
		for (auto &region : curAddr) {
			region.clear();
		}
	}
}

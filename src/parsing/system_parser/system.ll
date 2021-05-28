/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

%{
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <sstream>

#include "tchecker/parsing/declaration.hh"
#include "tchecker/parsing/parsing.hh"
#include "tchecker/utils/log.hh"

#include "system.tab.hh"


// Tell Flex the lexer's prototype ...
#define YY_DECL \
tchecker::parsing::system::parser_t::symbol_type spyylex \
(std::string const & filename, \
tchecker::log_t & log, \
tchecker::parsing::system_declaration_t * & system_declaration)

// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
#undef yywrap
#define yywrap() 1


// The location of the current token.
static tchecker::parsing::system::location loc;
	
void sp_reset_locations() {
	loc.initialize(nullptr, 1, 1);
}
	
using namespace tchecker::parsing;
%}

%option noyywrap nounput batch noinput
/* %option debug */

id        [[:alpha:]_.!][[:alnum:]_.!]*
integer   [-+]?[0-9]+

%{
  // Code run each time a pattern is matched.
	// move token's length wide
  #define YY_USER_ACTION  loc.columns(static_cast<int>(spyyleng));
%}

%x COMMENT
%x ATTR_KEY
%x ATTR_VALUE


%%

%{
  // Code run each time spyylex is called.
  loc.step();
%}


"#"            { BEGIN COMMENT; }
":"            { return system::parser_t::make_TOK_COLON(loc); }
"@"            { return system::parser_t::make_TOK_AT(loc); }
"?"            { return system::parser_t::make_TOK_QMARK(loc); }
"{"            { BEGIN ATTR_KEY;
	               return system::parser_t::make_TOK_LBRACE(loc); }
"}"            { return system::parser_t::make_TOK_RBRACE(loc); }
"clock"        { return system::parser_t::make_TOK_CLOCK(loc); }
"edge"         { return system::parser_t::make_TOK_EDGE(loc); }
"event"        { return system::parser_t::make_TOK_EVENT(loc); }
"int"          { return system::parser_t::make_TOK_INT(loc); }
"location"     { return system::parser_t::make_TOK_LOCATION(loc); }
"process"      { return system::parser_t::make_TOK_PROCESS(loc); }
"sync"         { return system::parser_t::make_TOK_SYNC(loc); }
"system"       { return system::parser_t::make_TOK_SYSTEM(loc); }
{id}           { return system::parser_t::make_TOK_ID(spyytext, loc); }
{integer}      { return system::parser_t::make_TOK_INTEGER(spyytext, loc); }
[ \t]+         { loc.step(); }
[\n]+          { loc.lines(static_cast<int>(spyyleng)); loc.step();
	               return system::parser_t::make_TOK_EOL(loc); }
<<EOF>>        { return system::parser_t::make_TOK_EOF(loc); }



<COMMENT>{
[\n]+          { BEGIN INITIAL;
                 loc.lines(static_cast<int>(spyyleng)); loc.step();
	               return system::parser_t::make_TOK_EOL(loc); }
.*             ;
}



<ATTR_KEY>{
"}"            { BEGIN INITIAL;
                 return system::parser_t::make_TOK_RBRACE(loc); }
":"            { BEGIN ATTR_VALUE;
                 return system::parser_t::make_TOK_COLON(loc); }
{id}           { return system::parser_t::make_TOK_ID(spyytext, loc); }
[ \t]+         { loc.step(); }
[\n]+          { loc.lines(static_cast<int>(spyyleng)); loc.step(); }
}



<ATTR_VALUE>{
":"            { BEGIN ATTR_KEY;
                 return system::parser_t::make_TOK_COLON(loc); }
[ \t]*"}"      { BEGIN INITIAL;
                 return system::parser_t::make_TOK_RBRACE(loc); }
[\n]+          { loc.lines(static_cast<int>(spyyleng)); loc.step(); }
[^:{}#\n]*     { return system::parser_t::make_TOK_TEXT(spyytext, loc); }
}


<*>.|\n        { std::stringstream msg;
                 msg << loc << " Invalid character: " << spyytext;
                 throw std::runtime_error(msg.str()); }




%%

namespace tchecker {

  namespace parsing {

    tchecker::parsing::system_declaration_t * parse_system_declaration(std::string const & filename, tchecker::log_t & log)
    {
      if (filename.empty() || (filename == "-"))
        return tchecker::parsing::parse_system_declaration(stdin, "", log);
      
      std::FILE * f = std::fopen(filename.c_str(), "r");
      if (f == nullptr)
        throw std::runtime_error("cannot open " + filename + ": " + strerror(errno));
        
      tchecker::parsing::system_declaration_t * sysdecl = nullptr;
      try {
        sysdecl = tchecker::parsing::parse_system_declaration(f, filename, log);
        std::fclose(f);
      }
      catch (...) {
        std::fclose(f);
        throw;
      }
      return sysdecl;
    }
    
    
    tchecker::parsing::system_declaration_t *
    parse_system_declaration(std::FILE * f, std::string const & filename, tchecker::log_t & log)
		{
      std::size_t old_error_count = log.error_count();
      
      spyyrestart(f);

      // Initialise
      sp_reset_locations();
      BEGIN INITIAL;
      
      // Parse
      tchecker::parsing::system_declaration_t * sysdecl = nullptr;
      
      try {
        tchecker::parsing::system::parser_t parser(filename, log, sysdecl);
				parser.parse();
        spyy_flush_buffer(YY_CURRENT_BUFFER);
      }
      catch (...) {
        delete sysdecl;
        spyy_flush_buffer(YY_CURRENT_BUFFER);
        throw;
      }
      
      if (log.error_count() > old_error_count) {
        delete sysdecl;
        sysdecl = nullptr;
      }
      
      return sysdecl;
    }
    
  }
  
}






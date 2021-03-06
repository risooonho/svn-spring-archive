#ifndef TDF_GRAMMAR_HPP_INCLUDED
#define TDF_GRAMMAR_HPP_INCLUDED 
#include <map>
#include <iostream>
#include <string>
#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols.hpp>
#include <boost/spirit/attribute.hpp>
#include <boost/spirit/dynamic.hpp>
#include <boost/spirit/phoenix.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/iterator/file_iterator.hpp>
#include <boost/spirit/utility/grammar_def.hpp>
#include <boost/spirit/iterator/position_iterator.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#include <boost/spirit/error_handling/exceptions.hpp>
#include "StdAfx.h"
#include "TdfParser.h"


/**
 * \brief Simple std::ostream Actor, with fixed prefix or suffix. 
 * This actor prints the item parsed using the ostream, enclosed
 * by suffix and prefix string. 
 */
class ostream_actor
{
  private:
    std::ostream & ref;
    std::string prefix, suffix;

  public:
    ostream_actor( std::ostream & ref_, std::string const& addition = "" ) 
      : ref( ref_ ), suffix(addition) {};
    ostream_actor( std::string prefix, std::ostream & ref_, std::string const& addition = "" ) 
      : ref( ref_ ), prefix(prefix), suffix(addition) {};
 
    template<typename T2> void operator()(T2 const& val ) const { ref << prefix << val << suffix; }

    template<typename IteratorT>
    void operator()( IteratorT const& first, IteratorT const& last ) const { ref << prefix << std::string(first, last) << suffix; }
};

/**
 * \name Actor functions
 * uese these two functions to create a semantic action with ostream_actor. 
 * \{
 */
inline ostream_actor ostream_a( std::string const& prefix, std::ostream & ref, std::string const& suffix ="" ) 
{ return ostream_actor(prefix, ref, suffix); }
inline ostream_actor ostream_a( std::ostream & ref, std::string const& suffix= ""  ) 
{ return ostream_actor(ref, suffix ); }
/**
 * \}
 */


struct tdf_grammar : public boost::spirit::grammar<tdf_grammar> {
  enum Errors {
    semicolon_expected
      , equals_sign_expected
      , square_bracket_expected
      , brace_expected 
  };
  typedef std::map<std::string,std::string> map_type;
  typedef map_type::value_type value_t;
  typedef std::pair<map_type::iterator,bool> insert_ret;

  struct section_closure : boost::spirit::closure<section_closure, TdfParser::TdfSection *>{ member1 context; };

  struct string_closure : boost::spirit::closure<string_closure, std::string>{ member1 name; };

  TdfParser::TdfSection *section;
  mutable std::list<std::string>  *junk;
  tdf_grammar( TdfParser::TdfSection* sec, std::list<std::string> * junk_data )
    : section(sec), junk(junk_data)
    { }
  template<typename ScannerT>
  struct definition {

    boost::spirit::rule<ScannerT>  tdf, gather_junk_line;
    boost::spirit::rule<ScannerT, string_closure::context_t> name;
    boost::spirit::rule<ScannerT, section_closure::context_t> section;

    boost::spirit::assertion<Errors> expect_semicolon, expect_equals_sign, expect_square_bracket, expect_brace;

    std::string temp1;

    definition(tdf_grammar const& self) 
      : expect_semicolon(semicolon_expected)
      , expect_equals_sign(equals_sign_expected)
      , expect_square_bracket(square_bracket_expected)
      , expect_brace(brace_expected)
    { 
      using namespace boost::spirit;
      using namespace phoenix;
      tdf = 
        *( 
            section(self.section) 
            | gather_junk_line  // if this rule gets hit then section did not consume everything,
         ) 
        ;

      gather_junk_line = 
        lexeme_d[
        (+(~chset<>("}[\n")))
        [ push_back_a( * self.junk  ) ]
        [ ostream_a( "Junk detected:", std::cout, "\n" ) ] // debug printouts
        ]
        ;

      name =  
        //(+chset<>("a-zA-Z0-9_+,-")) 
        (+(~chset<>(";{[]}=\n"))) // allows pretty much everything that isnt already used
        [ name.name = construct_<std::string>(arg1, arg2) ] 
        ;

      section = 
        '[' 
        >> name
   //     [ ostream_a( "Just parsed a section: ", std::cout, "\n") ] // prints section name
        [ section.context = bind( &TdfParser::TdfSection::construct_subsection )(section.context, arg1)  ]
        >> expect_square_bracket( ch_p(']') )
        >> expect_brace (ch_p('{') )
        >> *
        (
         (
          name
          [var(temp1) = arg1] 
          >> ch_p('=') // turn this into expect_equals_sign( ch_p('=') ) if you want more strict parsing
          >> lexeme_d[ (*~ch_p(';')) // might be empty too!
          [ bind( &TdfParser::TdfSection::add_name_value)(section.context, var(temp1), construct_<std::string>(arg1,arg2) ) ]
          ]
          >> expect_semicolon( ch_p(';') )
         )
         | section(section.context)
         | gather_junk_line  // if this rule gets touched we either hit a closing section } or there really is junk
        )
        >> expect_brace( ch_p('}') )
        ;
    }
    boost::spirit::rule<ScannerT> const& start() const { return tdf; }
  };
};

#endif


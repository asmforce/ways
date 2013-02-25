#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "notation.hpp"
#include <elib/aliases.hpp>
using namespace elib::aliases;


namespace fsm
{
  namespace internals
  {
    struct transition
    {
      enum  {
        action_invalid,
        action_continue,
        action_continue_skip,
        action_continue_keep,
        action_clear,
        action_clear_skip,
        action_clear_keep,
        action_token,
        action_token_skip,
        action_token_keep,
        action_warning,
        action_failure
      };

      transition()
      : option_on(false),
        eos(false),
        option_go(false),
        option_clear(false),
        option_token(false),
        option_failure(false),
        option_keep(false),
        option_skip(false)
      {}

      bool option_on;
      std::vector<char> chars;
      bool eos;

      bool option_go;
      std::string target_state;

      bool option_clear;

      bool option_token;
      std::string token_name;

      bool option_failure;
      std::string failure_message;

      bool option_warning;
      std::string warning_message;

      bool option_keep;
      bool option_skip;

      u32 line, column;
    };

    struct state
    {
      std::vector<transition> transitions;
      u32 line, column;
    };
  }


  bool translate( std::istream &in, std::ostream &out )
  {
    std::map<std::string, internals::state> states;
    notation::source src( in );
    u32 line, column;

    while( src >> notation::ws() >> notation::pos(line,column) >> notation::keyword("state") >> true )
    {
      std::string state_name;

      // std::clog << "debug: keyword `state` at <" << line << ';' << column << '>' << std::endl;

      if( src >> notation::ws() >> notation::pos(line,column) >> notation::id(state_name) >> false )  {
        std::cerr << "error: missing expected symbolic name of state at <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      // std::clog << "debug: state name `" << state_name << "` at <" << line << ';' << column << '>' << std::endl;

      if( src >> notation::pos(line,column) >> notation::ws() >> notation::delim(':') >> false )  {
        std::cerr << "error: missing expected colon since <" << line << ';' << column << '>' << std::endl;
        return false;
      }

      src >> notation::pos( line, column );
      // std::clog << "debug: state `" << state_name << "` declaration opened at <" << line << ';' << column << '>' << std::endl;
      internals::state &state = states[state_name];
      state.line = line;
      state.column = column;

      while( src >> notation::ws() >> notation::keyword("transition") >> notation::pos(line,column) >> true )
      {
        internals::transition &transition = *state.transitions.insert( state.transitions.end(), internals::transition() );
        transition.line = line;
        transition.column = column;

        // std::clog << "debug: transition (from state `" << state_name << "`) declaration opened at <" << line << ';' << column << '>' << std::endl;

        std::string option_name, option_value;

        while( src >> notation::ws() >> notation::pos(line, column) >> notation::id(option_name) >> true )
        {
          // std::clog << "debug: option name `" << option_name << "` at <" << line << ';' << column << '>' << std::endl;

          if( option_name == "on" )
          {
            if( transition.option_on == true )  {
              std::cerr << "error: redefinition of option `on` at <" << line << ';' << column << "> since <" << transition.line << ';' << transition.column << '>' << std::endl;
              return false;
            }

            transition.option_on = true;

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim('(') >> false )  {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            src >> notation::pos(line, column) >> notation::ws();
            if( src >> notation::keyword("end") >> true )
            {
              // std::clog << "debug: on(end)" << std::endl;
              transition.eos = true;
            } else if( src >> notation::str(option_value) >> true ) {
              // std::clog << "debug: on(\"" << option_value << '\')" << std::endl;
              transition.chars.assign( option_value.begin(), option_value.end() );
            }

            if( transition.eos == false && transition.chars.empty() )  {
              std::cerr << "error: empty character set specified since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim(')') >> false )  {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if( option_name == "go" ) {
            if( transition.option_go == true )  {
              std::cerr << "error: redefinition of option `go` at <" << line << ';' << column << "> since <" << transition.line << ';' << transition.column << '>' << std::endl;
              return false;
            }

            transition.option_go = true;

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim('(') >> false )  {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::id(option_value) >> true )  {
              // std::clog << "debug: go(\"" << option_value << '\')" << std::endl;
              transition.target_state = option_value;
            } else {
              std::cerr << "error: missing expected target state since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim(')') >> false )  {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if( option_name == "keep" ) {
            // std::clog << "debug: keep" << std::endl;
            if( transition.option_keep == true )  {
              std::cerr << "error: redefinition of option `keep` at <" << line << ';' << column << "> since <" << transition.line << ';' << transition.column << '>' << std::endl;
              return false;
            }

            transition.option_keep = true;
          } else if( option_name == "skip" ) {
            // std::clog << "debug: skip" << std::endl;
            if( transition.option_skip == true )  {
              std::cerr << "error: redefinition of option `skip` at <" << line << ';' << column << "> since <" << transition.line << ';' << transition.column << '>' << std::endl;
              return false;
            }

            transition.option_skip = true;
          } else if( option_name == "token" ) {
            if( transition.option_token == true )  {
              std::cerr << "error: redefinition of option `token` at <" << line << ';' << column << "> since <" << transition.line << ';' << transition.column << '>' << std::endl;
              return false;
            }

            transition.option_token = true;

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim('(') >> false )  {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::id(option_value) >> true )  {
              // std::clog << "debug: token(\"" << option_value << '\')" << std::endl;
              transition.token_name = option_value;
            } else {
              std::cerr << "error: missing expected token name since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim(')') >> false )  {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if( option_name == "warning" ) {
            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim('(') >> false )  {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            transition.option_warning = true;

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::str(option_value) >> true )  {
              // std::clog << "debug: warning(\"" << option_value << '\')" << std::endl;
              transition.warning_message = option_value;
            } else {
              std::cerr << "error: missing expected warning message since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim(')') >> false )  {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if( option_name == "failure" ) {
            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim('(') >> false )  {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            transition.option_failure = true;

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::str(option_value) >> true )  {
              // std::clog << "debug: failure(\"" << option_value << '\')" << std::endl;
              transition.failure_message = option_value;
            } else {
              std::cerr << "error: missing expected failure message since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if( src >> notation::pos(line, column) >> notation::ws() >> notation::delim(')') >> false )  {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else {
            std::cerr << "error: unknown transition option `" << option_name << "` at <" << line << ';' << column << '>' << std::endl;
            return false;
          }
        }

        if( src >> notation::ws() >> notation::pos(line,column) >> notation::delim(';') >> false )  {
          std::cerr << "error: missing expected transition option or semicolon since <" << line << ';' << column << '>' << std::endl;
          return false;
        }
        // std::clog << "debug: transition (from state `" << state_name << "`) declaration closed at <" << line << ';' << column+1 << '>' << std::endl;
      }

      if( src >> notation::ws() >> notation::pos(line,column) >> notation::delim(';') >> false )  {
        std::cerr << "error: missing expected semicolon before <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      // std::clog << "debug: state `" << state_name << "` declaration closed at <" << line << ';' << column+1 << '>' << std::endl;

      // states[state_name] = state;
    }

    if( src.end() )
    {
      if( states.size() > 0 )
      {
        std::clog << "declared " << states.size() << " state(s)" << std::endl;
        for( std::map<std::string,internals::state>::iterator i = states.begin(); i != states.end(); ++i )
        {
          std::pair<const std::string,internals::state> &p = *i;
          std::vector<internals::transition> &transitions = p.second.transitions;

          std::clog << "state " << p.first << " <" << p.second.transitions.size() << " transitions>" << std::endl;
          for( u32 i = 0; i < transitions.size(); ++i )
          {
            internals::transition &t = transitions[i];
            std::clog << "\ttransition";
            if( t.option_on )
            {
              std::clog << " on( ";
              if( t.eos )
                std::clog << "end ";
              if( false == t.chars.empty() )
              {
                std::clog << '\"';
                for( u32 i = 0; i < t.chars.size(); ++i )
                  std::clog << t.chars[i];
                std::clog << "\" ";
              }
              std::clog << ')';
            }
            if( t.option_go )
              std::clog << " go(" << t.target_state << ')';
            if( t.option_clear )
              std::clog << " clear";
            if( t.option_token )
              std::clog << " token(" << t.token_name << ')';
            if( t.option_failure )
              std::clog << " failure(" << t.failure_message << ')';
            if( t.option_warning )
              std::clog << " warning(" << t.warning_message << ')';
            if( t.option_keep )
              std::clog << " keep";
            if( t.option_skip )
              std::clog << " skip";
            std::clog << ';' << std::endl;
          }
        }
        // TODO: build DFA
        return true;
      } else {
        std::cerr << "error: missing declaration" << std::endl;
        return false;
      }
    } else {
      std::cerr << "error: missing expected keyword `state` at <" << line << ';' << column << '>' << std::endl;
      return false;
    }
  }
}


int main( int argc, char **argv )
{
  return fsm::translate(std::cin, std::cout) ? 0 : 1;
}

#ifndef WAYS_HPP
#define WAYS_HPP

#include <iostream>
#include <elib/aliases.hpp>
using namespace elib::aliases;

#include <stack>
#include <map>
#include <vector>
#include <string>

#define DEBUG

class Ways {
private:
    struct Rule;
    struct RuleGroup;
    struct Transition;
    struct CClassGenerationNode;

    struct Rule {
    public:
      Rule() :
      optionOn(false),
      optionGo(false),
      optionClear(false),
      optionKeep(false),
      optionSkip(false),
      optionToken(false),
      optionFailure(false),
      optionEntry(false),
      onEos(false) {}

    public:
      bool optionOn;
      bool optionGo;
      bool optionClear;
      bool optionKeep;
      bool optionSkip;
      bool optionToken;
      bool optionFailure;
      bool optionEntry;

      bool onEos;
      std::string onChars;
      std::string goState;
      std::string tokenName;
      std::string failureMessage;

      u32 line, column;
    };

    struct RuleGroup {
      std::vector<Rule> rules;
      std::string stateName;  // For diagnosis only
    };

    struct Transition {
    public:
      enum {
        ActionInvalid,
        ActionContinue,
        ActionClear,
        ActionToken,
        ActionFailure
      };

      enum {
        ModeLeave,
        ModeKeep,
        ModeSkip
      };

    public:
      Transition() : state(0), action(ActionInvalid), mode(ModeLeave), entry(false), arg(0) {}

    public:
      u32 state;
      u8 action;
      u8 mode;
      bool entry;
      u32 arg;
    };

    /**
     * Currently the 8-bit encodings are only supported.
     *   That allows to achieve a high performance using static
     * tables (maps) with constant complexity for most of operations.
     *
     * The size of tables is equal to input charset size (i.e. 256).
    **/
    static const u32 charsetSize;

    static const char *KEYWORD_ON;
    static const char *KEYWORD_GO;
    static const char *KEYWORD_ENTRY;
    static const char *KEYWORD_KEEP;
    static const char *KEYWORD_SKIP;
    static const char *KEYWORD_CLEAR;
    static const char *KEYWORD_TOKEN;
    static const char *KEYWORD_FAILURE;
    static const char *KEYWORD_TRANSITION;
    static const char *KEYWORD_END;

    static const char DELIM_COLON;
    static const char DELIM_SEMICOLON;
    static const char DELIM_LPAREN;
    static const char DELIM_RPAREN;
public:
    /**
     * Parses @in stream and generates (prints to @out) transition tables
     *  for fsm (lexer).
     * Returns true if succeeds or false if fails.
    **/
    static bool translate(std::istream &in, std::ostream &out);

private:
    /**
     * Parses @in stream and builds intermediate representation
     * @state_map: state name -> corresponding id in @definition vector
     * <convention>@definition must be empty</convention>
     * <convention>@stateMap must be empty</convention>
    **/
    static bool parse(std::istream &in, std::map<std::string, u32> &stateMap, std::vector<RuleGroup> &definition);

    /**
     * Prints out a human-readable representation of the specified character
    **/
    static void escape(std::ostream &out, u8 c);
};

#endif // WAYS_HPP

#include <elib/aliases.hpp>
using namespace elib::aliases;

#include "ways.hpp"
#include "notation.hpp"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <stack>
#include <cassert>
#include <sstream>


#ifdef DEBUG
  #define DEBUG_PRINTLN(MESSAGE)  std::clog << MESSAGE << std::endl;
#else
  #define DEBUG_PRINTLN(...)
#endif

const u32 Ways::charsetSize = 256;

const char *Ways::KEYWORD_ON = "on";
const char *Ways::KEYWORD_GO = "go";
const char *Ways::KEYWORD_ENTRY = "entry";
const char *Ways::KEYWORD_KEEP = "keep";
const char *Ways::KEYWORD_SKIP = "skip";
const char *Ways::KEYWORD_CLEAR = "clear";
const char *Ways::KEYWORD_TOKEN = "token";
const char *Ways::KEYWORD_FAILURE = "failure";
const char *Ways::KEYWORD_TRANSITION = "transition";
const char *Ways::KEYWORD_END = "end";

const char Ways::DELIM_COLON = ':';
const char Ways::DELIM_SEMICOLON = ';';
const char Ways::DELIM_LPAREN = '(';
const char Ways::DELIM_RPAREN = ')';


bool Ways::parse(std::istream &in, std::map<std::string, u32> &stateMap, std::vector<RuleGroup> &definition) {
    notation::source src(in);
    u32 line, column;

    while (src >> notation::ws() >> notation::pos(line, column) >> notation::keyword("state") >> true) {
      std::string stateName;
      u32 stateId;

      DEBUG_PRINTLN("keyword `state` at <" << line << ';' << column << '>');

      if (src >> notation::ws() >> notation::pos(line, column) >> notation::id(stateName) >> false) {
        std::cerr << "error: missing expected symbolic name of state at <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      DEBUG_PRINTLN("state name `" << stateName << "` at <" << line << ';' << column << '>');

      if (stateMap.count(stateName) > 0) {
        DEBUG_PRINTLN("redefinition of state `" << stateName << "` at <" << line << ';' << column << '>');
      }

      if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_COLON) >> false) {
        std::cerr << "error: missing expected colon since <" << line << ';' << column << '>' << std::endl;
        return false;
      }

      src >> notation::pos(line, column); // for debug only
      DEBUG_PRINTLN(std::endl << std::endl << std::endl << "state `" << stateName << "` declaration opened at <" << line << ';' << column << '>');

      // find RuleGroup corresponding to state_name or create a new one
      if (stateMap.count(stateName)) {
        stateId = stateMap[stateName];
      } else {
        definition.push_back(RuleGroup());
        definition.back().stateName = stateName;
        stateId = definition.size() - 1;
        stateMap[stateName] = stateId;
      }

      std::vector<Rule> &rules = definition[stateId].rules;

      while (src >> notation::ws() >> notation::keyword(KEYWORD_TRANSITION) >> notation::pos(line, column) >> true) {
        rules.push_back(Rule());

        Rule &rule = rules.back();
        rule.line = line;
        rule.column = column;

        DEBUG_PRINTLN(std::endl << "transition (from state `" << stateName << "`) declaration opened at <" << line << ';' << column << '>');

        std::string optionName;

        while (src >> notation::ws() >> notation::pos(line, column) >> notation::id(optionName) >> true) {
          DEBUG_PRINTLN("option name `" << optionName << "` at <" << line << ';' << column << '>');

          if (optionName == KEYWORD_ON) {
            if (rule.optionOn == true) {
              std::cerr << "error: redefinition of option `on` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
              return false;
            }

            rule.optionOn = true;

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_LPAREN) >> false) {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            src >> notation::pos(line, column) >> notation::ws();
            if (src >> notation::keyword(KEYWORD_END) >> true)
            {
              DEBUG_PRINTLN("on(end)");
              rule.onEos = true;
            } else if (src >> notation::str(rule.onChars) >> true) {
              DEBUG_PRINTLN("on(\"" << rule.onChars << "\")");
            }

            if (rule.onEos == false && rule.onChars.empty()) {
              std::cerr << "error: empty character set specified since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_RPAREN) >> false) {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if (optionName == KEYWORD_GO) {
            if (rule.optionGo == true) {
              std::cerr << "error: redefinition of option `go` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
              return false;
            }

            rule.optionGo = true;

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_LPAREN) >> false) {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::id(rule.goState) >> true) {
              DEBUG_PRINTLN("go(\"" << rule.goState << "\")");
            } else {
              std::cerr << "error: missing expected target state since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_RPAREN) >> false) {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if (optionName == KEYWORD_ENTRY) {
            if (rule.optionEntry == true) {
              std::cerr << "warning: redefinition of option `entry` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
            }

            rule.optionEntry = true;
          } else if (optionName == KEYWORD_KEEP) {
            if (rule.optionKeep == true) {
              std::cerr << "warning: redefinition of option `keep` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
            }

            rule.optionKeep = true;
          } else if (optionName == KEYWORD_SKIP) {
            if (rule.optionSkip == true) {
              std::cerr << "warning: redefinition of option `skip` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
            }

            rule.optionSkip = true;
          } else if (optionName == KEYWORD_CLEAR) {
            if (rule.optionClear == true) {
              std::cerr << "warning: redefinition of option `clear` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
            }

            rule.optionClear = true;
          } else if (optionName == KEYWORD_TOKEN) {
            if (rule.optionToken == true) {
              std::cerr << "error: redefinition of option `token` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
              return false;
            }

            rule.optionToken = true;

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_LPAREN) >> false) {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::id(rule.tokenName) >> true) {
              DEBUG_PRINTLN("token(\"" << rule.tokenName << "\")");
            } else {
              std::cerr << "error: missing expected token name since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_RPAREN) >> false) {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else if (optionName == KEYWORD_FAILURE) {
            if (rule.optionFailure == true) {
              std::cerr << "error: redefinition of option `failure` at <" << line << ';' << column << "> since <" << rule.line << ';' << rule.column << '>' << std::endl;
              return false;
            }

            rule.optionFailure = true;

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_LPAREN) >> false) {
              std::cerr << "error: missing expected left parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::str(rule.failureMessage) >> true) {
              DEBUG_PRINTLN("failure(\"" << rule.failureMessage << "\")");
            } else {
              std::cerr << "error: missing expected failure message since <" << line << ';' << column << '>' << std::endl;
              return false;
            }

            if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_RPAREN) >> false) {
              std::cerr << "error: missing expected right parenthesis since <" << line << ';' << column << '>' << std::endl;
              return false;
            }
          } else {
            std::cerr << "error: unknown transition option `" << optionName << "` at <" << line << ';' << column << '>' << std::endl;
            return false;
          }
        }

        if (src >> notation::ws() >> notation::pos(line, column) >> notation::delim(DELIM_SEMICOLON) >> false) {
          std::cerr << "error: missing expected transition option or semicolon since <" << line << ';' << column << '>' << std::endl;
          return false;
        }
        DEBUG_PRINTLN("transition (from state `" << stateName << "`) declaration closed at <" << line << ';' << column+1 << '>');
      }

      if (src >> notation::ws() >> notation::pos(line, column) >> notation::delim(DELIM_SEMICOLON) >> false) {
        std::cerr << "error: missing expected semicolon before <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      DEBUG_PRINTLN("state `" << stateName << "` declaration closed at <" << line << ';' << column+1 << '>');
    }

    if (src.end()) {
      if (definition.size() > 0) {
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


bool Ways::translate(std::istream &in, std::ostream &out) {
  std::map<std::string, u32> stateMap;
  std::vector<RuleGroup> definition;

  if (false == parse(in, stateMap, definition))
    return false;

  //std::vector<std::string> failureMessages;
  //std::vector<std::string> tokenNames;

  // direct mapping characters into cclasses
  u8 classMap[charsetSize];
  u16 classUsage[charsetSize];
  // cclass id 0 is reserved, it represents unallocated characters
  u8 maxClassId = 0;

  std::fill_n(classMap, charsetSize, u8(0));
  std::fill_n(classUsage, charsetSize, u16(0));
  classUsage[0] = charsetSize;

  for (u32 stateId = 0; stateId < definition.size(); ++stateId) {
    RuleGroup &group = definition[stateId];
    std::vector<Rule> &rules = group.rules;

    if (rules.empty()) {
      std::cerr << "warning: no transitions specified (state `" << group.stateName << "`)" << std::endl;
      continue;
    }

    // Default rule (with `on` option omitted) may not be defined more than one time within the same state
    bool hasDefaultRule = false;

    DEBUG_PRINTLN("new state (rules group): " << group.stateName);

    for (u32 i = 0; i < rules.size(); ++i) {
      Rule &rule = rules[i];

      bool charsUsage[charsetSize];
      std::fill_n(charsUsage, charsetSize, false);

      DEBUG_PRINTLN("new rule");
      if (!rule.optionKeep && !rule.optionSkip && !rule.optionGo && !rule.optionFailure) {
        std::cerr << "error: infinite transition declared (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
        std::cerr << "// at least one of the following options is needed: `keep`, `skip`, `go`, `failure`" << std::endl;
        return false;
      }

      if (rule.optionOn) {
        std::set<char> chars(rule.onChars.begin(), rule.onChars.end());
        u8 relocations[charsetSize];

        std::fill_n(relocations, charsetSize, u8(0));

#ifdef DEBUG
        {
          std::stringstream stringStream;
          for (u32 i = 0; i < rule.onChars.length(); ++i) {
            escape(stringStream, rule.onChars[i]);
          }
          DEBUG_PRINTLN("on(`" << stringStream.str() << "`)");
        }
#endif

        for (std::set<char>::iterator i = chars.begin(); i != chars.end(); i++) {
          u8 c = *i;

          if (charsUsage[c]) {
            std::cerr << "error: input character `" << *i << "` which is already in use specified for transition (state `" << group.stateName << "`) since <" << rule.line << ';' << rule.column << '>' << std::endl;
            return false;
          }
          charsUsage[c] = true;

          // Check if already allocated/relocated
          u8 oldClassId = classMap[c];
          u8 newClassId = relocations[oldClassId];

          if (newClassId == 0) {
              maxClassId++;
              newClassId = maxClassId;
              relocations[oldClassId] = newClassId;
              DEBUG_PRINTLN("allocated class: " << u32(newClassId));
          }

          classUsage[oldClassId]--;
          DEBUG_PRINTLN("classUsage[" << u32(oldClassId) << "]: " << classUsage[oldClassId]);
          classUsage[newClassId]++;
          DEBUG_PRINTLN("classUsage[" << u32(newClassId) << "]: " << classUsage[newClassId]);

          if (classUsage[oldClassId] == 0) {
            DEBUG_PRINTLN("recycled class: " << u32(newClassId));
            std::swap(classUsage[oldClassId], classUsage[newClassId]);
            relocations[oldClassId] = oldClassId;
            maxClassId--;
          }
        }

        // Flush relocations
        for (std::set<char>::iterator i = chars.begin(); i != chars.end(); i++) {
          u8 &cclassId = classMap[u8(*i)];
          cclassId = relocations[cclassId];
        }

        // Если больше нет необходимости хранить оригинальные строки, то лучше сразу
        //   их удалить, чтобы сэкономить лишний килобайт памяти
        //rl.on_chars.clear();
      } else {
        DEBUG_PRINTLN("used as default");
        if (hasDefaultRule) {
          std::cerr << "error: redefinition of default (with `on` option omitted) transition (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
          return false;
        }
        hasDefaultRule = true;
      }

#ifdef DEBUG
    {
      for (u32 clazz = 1; clazz < 256; clazz++) {
        if (classUsage[clazz] > 0) {
          std::stringstream stringStream;
          for (u32 c = 0; c < 256; c++) {
            if (classMap[c] == clazz) {
              escape(stringStream, c);
            }
          }
          DEBUG_PRINTLN(clazz << ": \"" << stringStream.str() << "\"");
        }
      }
    }
#endif
    }
  }


  DEBUG_PRINTLN(std::endl << std::endl);

  return true;
}

void Ways::escape(std::ostream &out, u8 c) {
    const u8 SPECIAL_CHARACTER_MAX = 31;

    if (c <= SPECIAL_CHARACTER_MAX) {
      switch ((char)c) {
      case '\0':
        out << "\\0";
        break;

      case '\n':
        out << "\\n";
        break;

      case '\t':
        out << "\\t";
        break;

      case '\r':
        out << "\\r";
        break;

      default:
        out << "\\d{" << u32(c) << '}';
      }
    } else {
      switch ((char) c) {
      case '\\':
        out << "\\\\";
        break;

      case '\'':
        out << "\\\'";
        break;

      case '\"':
        out << "\\\"";
        break;

      default:
        out << c;
      }
    }
}

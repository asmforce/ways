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

const u32 Ways::INVALID_ID = u32(-1);

const char *Ways::KEYWORD_INITIAL = "initial";
const char *Ways::KEYWORD_STATE = "state";
const char *Ways::KEYWORD_ON = "on";
const char *Ways::KEYWORD_GO = "go";
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


bool Ways::parse(std::istream &in, std::map<std::string, u32> &stateMap, std::vector<RuleGroup> &definition, u32 &initialStateId) {
    notation::source src(in);
    u32 line, column;

    initialStateId = INVALID_ID;

    while (src >> notation::ws() >> notation::pos(line, column) >> notation::keyword(KEYWORD_STATE) >> true) {
      std::string stateName;
      u32 stateId;

      DEBUG_PRINTLN("keyword `state` at <" << line << ';' << column << '>');

      if (src >> notation::ws() >> notation::pos(line, column) >> notation::id(stateName) >> false) {
        std::cerr << "error: missing expected symbolic name of state at <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      DEBUG_PRINTLN("state name `" << stateName << "` at <" << line << ';' << column << '>');

#ifdef DEBUG
      if (stateMap.count(stateName) > 0) {
        DEBUG_PRINTLN("redefinition of state `" << stateName << "` at <" << line << ';' << column << '>');
      }
#endif

      bool isInitial = false;
      if (src >> notation::ws() >> notation::keyword(KEYWORD_INITIAL) >> true) {
        isInitial = true;
      }

      if (src >> notation::pos(line, column) >> notation::ws() >> notation::delim(DELIM_COLON) >> false) {
        std::cerr << "error: missing expected colon since <" << line << ';' << column << '>' << std::endl;
        return false;
      }

      src >> notation::pos(line, column); // For debug only
      DEBUG_PRINTLN(std::endl << std::endl << "state `" << stateName << "` declaration opened at <" << line << ';' << column << '>');

      // Find RuleGroup corresponding to state_name or create a new one
      if (stateMap.count(stateName)) {
        stateId = stateMap[stateName];
      } else {
        RuleGroup group;
        group.stateName = stateName;
        definition.push_back(group);
        stateId = definition.size() - 1;
        stateMap[stateName] = stateId;
      }

      RuleGroup &group = definition[stateId];
      std::vector<Rule> &rules = group.rules;

      if (isInitial) {
        DEBUG_PRINTLN("this one is initial");
        if (initialStateId != INVALID_ID && stateId != initialStateId) {
          std::cerr << "error: state `" << definition[initialStateId].stateName << "` was earlier declared as initial" << std::endl;
          return false;
        }
        initialStateId = stateId;
      }
#ifdef DEBUG
      else if (stateId == initialStateId) {
        DEBUG_PRINTLN("this one is initial");
      }
#endif

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

            std::string onChars;
            src >> notation::pos(line, column) >> notation::ws();
            if (src >> notation::keyword(KEYWORD_END) >> true) {
              DEBUG_PRINTLN("on(end)");
              rule.onEos = true;
            } else if (src >> notation::str(onChars) >> true) {
              rule.onChars.insert(onChars.begin(), onChars.end());
#ifdef DEBUG
              std::stringstream stringStream;
              for (u32 i = 0; i < onChars.length(); ++i) {
                escape(stringStream, onChars[i]);
              }
              DEBUG_PRINTLN("on(\"" << stringStream.str() << "\")");
#endif
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
        DEBUG_PRINTLN("transition (from state `" << stateName << "`) declaration closed at <" << line << ';' << column << '>');
      }

      if (src >> notation::ws() >> notation::pos(line, column) >> notation::delim(DELIM_SEMICOLON) >> false) {
        std::cerr << "error: missing expected semicolon before <" << line << ';' << column << '>' << std::endl;
        return false;
      }
      DEBUG_PRINTLN("state `" << stateName << "` declaration closed at <" << line << ';' << column << '>');
    }

    if (src.end()) {
      if (definition.size() > 0) {
        return true;
      } else {
        std::cerr << "error: missing declaration" << std::endl;
        return false;
      }
    } else {
      std::cerr << "error: missing declaration at <" << line << ';' << column << '>' << std::endl;
      return false;
    }
}


bool Ways::translate(std::istream &in, std::ostream &out) {
  std::map<std::string, u32> stateMap;
  std::vector<RuleGroup> definition;
  u32 initialStateId;

  if (false == parse(in, stateMap, definition, initialStateId))
    return false;

  std::vector<std::string> tokens;
  std::map<std::string, u32> tokenMap;
  std::vector<std::string> failureMessages;
  std::map<std::string, u32> failureMap;

  // Direct mapping characters into cclasses
  u8 classMap[charsetSize];
  u16 classUsage[charsetSize];
  // Class id 0 is reserved, it represents unallocated characters
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

    for (u32 ruleId = 0; ruleId < rules.size(); ++ruleId) {
      Rule &rule = rules[ruleId];

      bool charsUsage[charsetSize];
      std::fill_n(charsUsage, charsetSize, false);

      DEBUG_PRINTLN("new rule");
      if (!rule.optionKeep && !rule.optionSkip && !rule.optionGo && !rule.optionFailure) {
        std::cerr << "error: infinite transition declared (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
        std::cerr << "// at least one of the following options is needed: `keep`, `skip`, `go`, `failure`" << std::endl;
        return false;
      }

      if (rule.optionOn) {
        u8 relocations[charsetSize];

        std::fill_n(relocations, charsetSize, u8(0));

#ifdef DEBUG
        {
          std::stringstream stringStream;
          for (std::set<char>::iterator i = rule.onChars.begin(); i != rule.onChars.end(); i++) {
            escape(stringStream, *i);
          }
          if (rule.onEos) {
            stringStream << KEYWORD_END;
          }
          DEBUG_PRINTLN("on(`" << stringStream.str() << "`)");
        }
#endif

        for (std::set<char>::iterator i = rule.onChars.begin(); i != rule.onChars.end(); i++) {
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
        for (std::set<char>::iterator i = rule.onChars.begin(); i != rule.onChars.end(); i++) {
          u8 &cclassId = classMap[u8(*i)];
          cclassId = relocations[cclassId];
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
      } else {
        DEBUG_PRINTLN("used as default");
        if (hasDefaultRule) {
          std::cerr << "error: redefinition of default (with `on` option omitted) transition (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
          return false;
        }
        hasDefaultRule = true;
      }
    }
  }

  const u32 stateCount = stateMap.size();
  // All allocated classes + unallocated characters class + eos
  const u32 classCount = maxClassId + 2;

  std::vector< std::vector<Transition> > transitions(stateCount);

  DEBUG_PRINTLN("now we have " << stateCount << " state(s) and " << classCount << " class(es)");

  for (u32 stateId = 0; stateId < stateCount; ++stateId) {
    RuleGroup &group = definition[stateId];
    std::vector<Rule> &rules = group.rules;

    transitions[stateId].resize(classCount);

    Transition defaultTransition;
    bool hasDefaultRule = false;
    std::set<u8> defaultClasses;

    // Universal set for now
    for (u32 clazz = 0; clazz < classCount; ++clazz) {
      defaultClasses.insert(clazz);
    }

    for (u32 ruleId = 0; ruleId < rules.size(); ++ruleId) {
      Rule &rule = rules[ruleId];
      Transition transition;

      // Default values
      transition.action = Transition::ActionContinue;
      transition.mode = Transition::ModeLeave;
      transition.state = stateId;

      if (rule.optionFailure) {
        transition.action = Transition::ActionFailure;
        if (rule.optionGo || rule.optionClear || rule.optionToken) {
          std::cerr << "error: option `failure` is incompatible with `go`, `clear` and `token` options of transition (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
          return false;
        }

        u32 failureId;
        if (failureMap.count(rule.failureMessage)) {
          failureId = failureMap[rule.failureMessage];
        } else {
          failureMessages.push_back(rule.failureMessage);
          failureId = failureMessages.size() - 1;
          failureMap[rule.failureMessage] = failureId;
        }
        transition.arg = failureId;
      }

      if (rule.optionGo) {
        u32 nextStateId;
        if (stateMap.count(rule.goState)) {
          nextStateId = stateMap[rule.goState];
        } else {
          std::cerr << "error: unknown next state `" << rule.goState << "` transition at <" << rule.line << ";" << rule.column << ">" << std::endl;
          return false;
        }
        transition.state = nextStateId;
      }

      if (rule.optionToken) {
        transition.action = Transition::ActionToken;
        if (rule.optionClear) {
          std::cerr << "error: option `token` is incompatible with `clear` option of transition (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
          return false;
        }

        u32 tokenId;
        if (tokenMap.count(rule.tokenName)) {
          tokenId = tokenMap[rule.tokenName];
        } else {
          tokens.push_back(rule.tokenName);
          tokenId = tokens.size() - 1;
          tokenMap[rule.tokenName] = tokenId;
        }
        transition.arg = tokenId;
      }

      if (rule.optionClear) {
        transition.action = Transition::ActionClear;
      }

      if (rule.optionKeep) {
        transition.mode = Transition::ModeKeep;
        if (rule.optionSkip) {
          std::cerr << "error: option `keep` is incompatible with `keep` option of transition (state `" << group.stateName << "`) at <" << rule.line << ';' << rule.column << '>' << std::endl;
          return false;
        }
      }

      if (rule.optionSkip) {
        transition.mode = Transition::ModeSkip;
      }

      if (rule.optionOn) {
        std::set<u8> classes;
        for (std::set<char>::const_iterator i = rule.onChars.begin(); i != rule.onChars.end(); i++) {
          classes.insert(classMap[u8(*i)]);
        }
        if (rule.onEos) {
          // Eos is represented by a class with maximum id
          classes.insert(classCount-1);
        }
        for (std::set<u8>::const_iterator i = classes.begin(); i != classes.end(); i++) {
          transitions[stateId][*i] = transition;
          defaultClasses.erase(*i);
        }
      } else {
        hasDefaultRule = true;
        defaultTransition = transition;
      }
    }

    if (hasDefaultRule) {
      for (std::set<u8>::const_iterator i = defaultClasses.begin(); i != defaultClasses.end(); i++) {
        transitions[stateId][*i] = defaultTransition;
      }
    }
  }

  out << "#include <elib/aliases.hpp>" << std::endl << std::endl;

  out << "namespace Ways {" << std::endl;
  out << "  using namespace elib::aliases;" << std::endl << std::endl;

  out << "  const u32 charsetSize = " << charsetSize << ';' << std::endl;
  out << "  const u32 classCount = " << classCount << ';' << std::endl;
  out << "  const u32 stateCount = " << stateCount << ';' << std::endl;
  if (initialStateId == INVALID_ID) {
    initialStateId = 0;
  }
  out << "  const u32 initialStateId = " << initialStateId << ';' << std::endl << std::endl;

  out << "  const u8 classMap[charsetSize] = {";
  for (u32 i = 0; i < charsetSize; ++i) {
    const u8 clazz = classMap[i];
    if (i % 16 == 0) {
      out << std::endl << "    ";
    }
    if (clazz < 100) {
      if (clazz < 10) {
        out << "   ";
      } else {
        out << "  ";
      }
    } else {
      out << ' ';
    }
    out << u32(clazz) << (i == charsetSize-1 ? "" : ",");
  }
  out << std::endl << "  };" << std::endl << std::endl;

  if (!failureMessages.empty()) {
    out << "  const char *failureMessages[] = {" << std::endl;
    for (u32 i = 0; i < failureMessages.size(); ++i) {
      std::string &message = failureMessages[i];
      out << "    \"";
      for (u32 j = 0; j < message.length(); ++j) {
        escape(out, message[j]);
      }
      out << "\"" << (i == failureMessages.size()-1 ? "" : ",") << std::endl;
    }
    out << "  };" << std::endl << std::endl;
  }

  if (!tokens.empty()) {
    out << "  struct Tokens {" << std::endl;
    out << "    enum {" << std::endl;
    for (u32 i = 0; i < tokens.size(); ++i) {
      out << "      " << tokens[i] << (i == tokens.size()-1 ? "" : ",") << std::endl;
    }
    out << "    };" << std::endl << "  };" << std::endl << std::endl;
  }

  out << "  struct Transition {" << std::endl
      << "  public:" << std::endl
      << "    enum {" << std::endl
      << "      ActionInvalid," << std::endl
      << "      ActionContinue," << std::endl
      << "      ActionClear," << std::endl
      << "      ActionToken," << std::endl
      << "      ActionFailure" << std::endl
      << "    };" << std::endl
      << std::endl
      << "    enum {" << std::endl
      << "      ModeLeave," << std::endl
      << "      ModeKeep," << std::endl
      << "      ModeSkip" << std::endl
      << "    };" << std::endl
      << "  " << std::endl
      << "  public:" << std::endl
      << "    Transition() : state(0), action(ActionInvalid), mode(ModeLeave), arg(0) {}" << std::endl
      << std::endl
      << "  public:" << std::endl
      << "    u32 state;" << std::endl
      << "    u8 action;" << std::endl
      << "    u8 mode;" << std::endl
      << "    u32 arg;" << std::endl
      << "  };" << std::endl << std::endl;

  out << "  const Transition transitions[][] = {" << std::endl;
  for (u32 stateId = 0; stateId < stateCount; ++stateId) {
    std::vector<Transition> &row = transitions[stateId];
    out << "    {";
    for (u32 classId = 0; classId < classCount; ++classId) {
      Transition &tr = row[classId];
      out << "{" << tr.state << ", " << u32(tr.action) << ", " << u32(tr.mode) << ", " << tr.arg << (classId == classCount-1 ? "}" : "}, ");
    }
    out << (stateId == stateCount-1 ? "}" : "},") << std::endl;
  }
  out << "  };" << std::endl;
  out << "}  // namespace" << std::endl;

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
        out << "\\x" << std::hex << u32(c) << std::dec;
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

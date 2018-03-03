#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Tool/Parser.hpp"
#include "Command.hpp"

static constexpr char TOKEN_SPLIT = ' ';
static constexpr char TOKEN_TAB   = '\t';
static constexpr char TOKEN_QUOTE = '"';
static constexpr std::array<char, 2> 
          TOKEN_WHITESPACE = { TOKEN_SPLIT, TOKEN_TAB };

static auto quote = "\""_P;
static auto quoteContent = many_of(
  R"( !#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~)");
static auto quoteStringParser = quote < quoteContent > quote;

static auto normalValParser = many_of(
  "!#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~)");

static auto argNameParser = 
  quoteStringParser
| many_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.\\/");

static auto argValParser = "="_P < (quoteStringParser | normalValParser);

static auto kvArgumentParser = combine(argNameParser, argValParser, [](const std::string& name, const std::string value) {
  return std::make_pair(name, value);
});

static auto valArgumentParser = fmap([](std::string val) -> std::pair<std::string, std::string> {
  return std::make_pair("", val);
}, quoteStringParser | normalValParser);

static auto argumentParser = kvArgumentParser | valArgumentParser;

template<typename ...Cs>
bool same(char a, Cs... cs);

template<typename C,typename ...Cs>
bool same(char a, C c, Cs... cs) {
  return same(a, c) || same(a, cs...);
}
template<typename C>
bool same(char a, C c) {
  return a == c;
}

template<typename ...Symbols>
void ignore(std::string& str, Symbols... cs) {
  for(uint i = 0, size = str.length(); i < size; i++) {
    if(!same(str[i], cs...)) {
      str.erase(0, i);
      return;
    }
  }

  str.clear();
}

uint findWhiteSpace(const std::string& str) {
  uint pos = std::string::npos;
  for(char c: TOKEN_WHITESPACE) {
    pos = std::min(pos, str.find(c));
  }

  return pos;
}


std::optional<std::pair<std::string, std::string>> consume(std::string& str) {
  auto parseResult = argumentParser(str);
  if (!parseResult) return {};
  str = std::move(parseResult->second);
  return parseResult->first;
}

Command::Arg::Arg(std::string kv) {
  const auto& arg = split(kv.c_str(), "=");

  switch(arg.size()) {
    case 1:
      strcpy_s(name, arg[0].c_str()); return;
    case 2:
      strcpy_s(name, arg[0].c_str());
      strcpy_s(value, arg[1].c_str()); return;
    default:
      Console::error("Unexpected argument: " + kv);
  }
}

Command::Arg::Arg(std::string v, std::string k) {
  strcpy_s(name, k.c_str());
  strcpy_s(value, v.c_str());
}

Command::Command(const std::string& cmd): mRawText(cmd) {
  mLegal = parse();
}

bool Command::parse() {
  mArgs.clear();
  std::string str = mRawText;
  size_t argStart = findWhiteSpace(str);
  if(argStart == std::string::npos) {
    mName = str;
    return true;
  }

  mName = str.substr(0, argStart);
  str.erase(0, argStart);

  ignore(str, TOKEN_SPLIT, TOKEN_TAB);
  while (str.length() != 0) {
    EXPECTS(str[0] != TOKEN_SPLIT);

    // parse the arg
    const auto& arg = consume(str);
    if(!arg) {
      Console::error("When parsing argument, unexpected character at: \x1b[130m" + str);
      return false;
    }
    mArgs.emplace_back(arg->second, arg->first);


    if(findWhiteSpace(str) != 0 && !str.empty()) {
      Console::error("should be any space or tab here, unexpected character at: " + str);
      return false;
    }
    ignore(str, TOKEN_SPLIT, TOKEN_TAB);
  }

  return true;
}

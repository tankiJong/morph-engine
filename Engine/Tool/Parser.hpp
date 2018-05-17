#pragma once
#include <xstring>
#include <optional>
#include <algorithm>
#include <type_traits>

// reference: https://www.youtube.com/watch?v=PJwd4JLYJJY
// https://github.com/lefticus/constexpr_all_the_things


using parse_input_t = std::string;

template <typename T>
using parse_result_t = std::optional<std::pair<T, std::string>>;

// Get various types out of a parser
template <typename P>
using opt_pair_parse_t = std::invoke_result_t<P, parse_input_t>;
template <typename P>
using pair_parse_t = typename opt_pair_parse_t<P>::value_type;
template <typename P>
using parse_t = typename pair_parse_t<P>::first_type;

template <typename P, typename T, typename F>
std::pair<T, parse_input_t> accumulate_parse(
  parse_input_t s, P&& p, T init, F&& f) {
  while (!s.empty()) {
    auto r = p(s);
    if (!r) return std::make_pair(init, s);
    init = f(init, r->first);
    s = r->second;
  }
  return std::make_pair(init, s);
}

template <typename P, typename T, typename F>
std::pair<T, parse_input_t> accumulate_n_parse(
  parse_input_t s, P&& p, std::size_t n, T init, F&& f) {
  while (n != 0) {
    auto r = p(s);
    if (!r) return std::make_pair(init, s);
    init = f(init, r->first);
    s = r->second;
    --n;
  }
  return std::make_pair(init, s);
}


inline auto make_char_parser(char c) {
  return [=](parse_input_t s) -> parse_result_t<char> {
    if (s.empty() || s[0] != c) return {};
    return
      parse_result_t<char>(std::make_pair(c, parse_input_t(s.data() + 1, s.size() - 1)));
  };
}

inline auto one_of(std::string symbols) {
  return [=](parse_input_t s) ->parse_result_t<char> {
    if (s.empty()) return {};
    auto j = std::find(symbols.cbegin(), symbols.cend(), s[0]);
    if (j != symbols.cend()) {
      return
        std::make_pair(s[0], parse_input_t(s.data() + 1, s.size() - 1));
    }
    return {};
  };
}

inline auto none_of(std::string sybs) {
  return [=](parse_input_t s) ->parse_result_t<char> {
    if (s.empty()) return {};
    auto j = std::find(sybs.cbegin(), sybs.cend(), s[0]);
    if (j == sybs.cend()) {
      return std::make_pair(s[0], parse_input_t(s.data() + 1, s.size() - 1));
    }
    return {};
  };
}

inline auto many_of(std::string symbols) {
  return [=](parse_input_t s) ->parse_result_t<std::string> {
    if (s.empty()) return {};
    for (size_t i = 0, size = s.length(); i < size; i++) {
      if(std::find(symbols.cbegin(), symbols.cend(), s[i]) == symbols.cend()) {
        if(i==0u) {
          return {};
        }
        return std::make_pair(s.substr(0, i), parse_input_t(s.data() + i, s.length() - i));
      }
    }

    return std::make_pair(s, "");
  };
}

inline auto except_of(std::string symbols) {
  return[=](parse_input_t s)-> parse_result_t<std::string> {
    if (s.empty()) return {};
    if(symbols.empty()) {
      return std::make_pair(s, "");
    }
    for(size_t i = 0, size = s.length(); i < size; i++) {
      if(std::find(symbols.cbegin(), symbols.cend(), s[i]) != symbols.cend()) {
        if(i==0u) {
          return {};
        }
        return std::make_pair(s.substr(0, i), parse_input_t(s.data() + i, s.length() - i));
      }
    }

    return std::make_pair(s, "");
  };
}

inline auto make_string_parser(std::string str) {
  return [=](parse_input_t s) ->parse_result_t<std::string> {
    if (s.empty()) return {};
    auto j = std::mismatch(str.cbegin(), str.cend(), s.cbegin(), s.cend());
    if (j.first == str.cend()) {
      auto size = std::distance(s.cbegin(), j.second);
      return
        std::make_pair(std::string(str),
                       parse_input_t(s.data() + size, s.size() - size));
    }

    return {};
  };
}

template<typename F, typename P>
inline auto fmap(F&& f, P&& p) {
  using R = parse_result_t<std::invoke_result_t<F, parse_t<P>>>;

  return[f = std::forward<F>(f), p = std::forward<P>(p)]
  (parse_input_t i)->R {
    auto r = p(i);
    if (!r) return {};
    return R(std::make_pair(f(r->first), r->second));
  };
}

template<typename P, typename F>
inline auto bind(P&& p, F&& f) {
  using R = std::invoke_result_t<F, parse_t<P>, parse_input_t>; // parse_result_t of P
  return [=](parse_input_t i) -> R {
    auto r = p(i);
    if (!r) return {};
    return f(r->first, r->second);
  };
}

template<typename P1, typename P2,
  std::enable_if_t<std::is_same_v<parse_t<P1>, parse_t<P2>>, int> = 0>
  inline auto operator | (P1&& p1, P2&& p2) {
  return [=](parse_input_t i) {
    auto r1 = p1(i);
    if (r1) return r1;
    return p2(i);
  };
}

template <typename T>
inline auto fail(T) {
  return [=](parse_input_t) -> parse_input_t<T> {
    return {};
  };
}

template <typename P1, typename P2, typename F,
  typename R = std::invoke_result_t<F, parse_t<P1>, parse_t<P2>>>
  inline auto combine(P1&& p1, P2&& p2, F&& f) {
  return [=](parse_input_t i) -> parse_result_t<R> {
    auto r1 = p1(i);
    if (!r1) return {};
    auto r2 = p2(r1->second);
    if (!r2) return {};
    return parse_result_t<R>(std::make_pair(f(r1->first, r2->first), r2->second));
  };
}

template<typename P1, typename P2,
  typename = parse_t<P1>, typename = parse_t<P2>>
  inline auto operator<(P1&& p1, P2&& p2) {
  return combine(std::forward<P1>(p1),
                 std::forward<P2>(p2),
                 [](auto, auto r) { return r; });
}

template<typename P1, typename P2,
  typename = parse_t<P1>, typename = parse_t<P2>>
  inline auto operator<<(P1&& p1, P2&& p2) {
  return combine(std::forward<P1>(p1),
                 std::forward<P2>(p2),
                 [](auto l, auto r) { return std::make_pair(l,r); });
}

template<typename P1, typename P2,
  typename = parse_t<P1>, typename = parse_t<P2>>
  inline auto operator>(P1&& p1, P2&& p2) {
  return combine(std::forward<P1>(p1),
                 std::forward<P2>(p2),
                 [](auto r, auto) { return r; });
}

template<typename P1, typename P2,
  typename = parse_t<P1>, typename = parse_t<P2>>
  inline auto operator>>(P1&& p1, P2&& p2) {
  return combine(std::forward<P1>(p1),
                 std::forward<P2>(p2),
                 [](auto l, auto r) { return std::make_pair(l,r); });
}

// many
template <typename T, typename P, typename F>
inline auto many(P&& p, T&& init, F&& f) {
  return[p = std::forward<P>(p), init = std::forward<T>(init),
    f = std::forward<F>(f)](parse_input_t s) {
    return parse_result_t<T>(
      accumulate_parse(s, p, init, f));
  };
}
// many1
template <typename P, typename T, typename F>
inline auto many1(P&& p, T&& init, F&& f) {
  return[p = std::forward<P>(p), init = std::forward<T>(init),
    f = std::forward<F>(f)](parse_input_t s)->parse_result_t<T> {
    auto r = p(s);
    if (!r) return {};
    return parse_result_t<T>(
      accumulate_parse(r->second, p, f(init, r->first), f));
  };
}
// exactly_n
template <typename P, typename T, typename F>
inline auto exactly_n(P&& p, std::size_t n, T&& init, F&& f) {
  return[p = std::forward<P>(p), n, init = std::forward<T>(init),
    f = std::forward<F>(f)](parse_input_t s) {
    return parse_result_t<T>(
      accumulate_n_parse(s, p, n, init, f));
  };
}
// separated_by

template <typename P1, typename P2, typename F>
constexpr auto separated_by(P1&& p1, P2&& p2, F&& f) {
  using T = parse_t<P1>;
  return[p1 = std::forward<P1>(p1), p2 = std::forward<P2>(p2),
    f = std::forward<F>(f)](
      parse_input_t s)->parse_result_t<T> {
    auto r = p1(s);
    if (!r) return {};
    auto p = p2 < p1;
    return parse_result_t<T>(
      accumulate_parse(r->second, p, r->first, f));
  };
}

inline auto operator ""_P(const char* str, size_t len) {
  return make_string_parser(std::string(str, len));
}


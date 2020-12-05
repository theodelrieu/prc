#include <boost/fusion/include/adapt_struct.hpp>

#define BOOST_SPIRIT_X3_DEBUG
#include <boost/spirit/home/x3.hpp>

#include <iostream>
#include <string>

// namespace prc::equilab
// {
// namespace x3 = boost::spirit::x3;
// namespace ascii = boost::spirit::x3::ascii;
//
// using ascii::char_;
// using x3::double_;
// using x3::int_;
// using x3::lexeme;
// using x3::lit;
//
// namespace ast
// {
// struct range
// {
//   std::string name;
//   std::vector<std::string> combos;
// };
//
// }
//
// namespace parser
// {
// inline char constexpr delim = 0xb0;
//
// x3::rule<struct range, ast::range> const range = "range";
// auto const range_name = lexeme[+(char_ - ' ') >> ' '];
// auto const combo = lexeme[x3::alnum - ",}"];
//
// // clang-format off
// // TODO bien gerer la grammaire des combos (EN ENTIER)
// auto const range_def =
//   '.' > range_name > '{' > +(combo % ',') > '}'
//   ;
//
// // FORMAT =
// //   "[Userdefined]" + '\n'
// //   RANGES
// //
// // RANGES = (RANGE_NAME + ' ' + '{' + RANGE + '}' + '\n')+
// // RANGE = ((WEIGHT + ':')? + COMBOS) | ((WEIGHT + ':')? + COMBOS +
// GROUP_INFO)+
// //
// // COMBOS = (COMBO + (',' + COMBO)*) + ':'
// // GROUP_INFO = DELIM + GROUP_INDEX + DELIM + PARENT_GROUP_INDEX + DELIM +
// GROUP_RGB + DELIM + (GROUP_NAME)? + DELIM + NESTING_INDEX + DELIM
// // GROUP_INDEX = integer
// // GROUP_RGB = (3-digit int)(3-digit int)(3-digit int)
// // GROUP_NAME = UTF16_CHARS
// // NESTING_INDEX = integer
// // WEIGHT = float
// // RANGE_NAME = '.' + UTF16_CHARS
// // DELIM = 0xb0
// // CRLF = \n
//
// // clang-format on
//
// BOOST_SPIRIT_DEFINE(range);
// }
//
// void parse(std::string_view sv)
// {
//   auto begin = sv.begin();
//   auto end = sv.end();
//   auto ld = [&](auto& ctx) {
//     auto s = _attr(ctx);
//     std::cout << "range name:" << s.name << std::endl;
//     for (auto const& combo : s.combos)
//     {
//       std::cout << combo << std::endl;
//     }
//     std::cout << "range name:" << _attr(ctx).name << std::endl;
//   };
//   auto r = x3::phrase_parse(
//       begin, end, (lit("[Userdefined]") >> parser::range[ld]), ascii::space);
//   if (!r)
//   {
//     std::cerr << "failed to parse" << std::endl;
//   }
// }
// }
//
// BOOST_FUSION_ADAPT_STRUCT(prc::equilab::ast::range, name, combos);

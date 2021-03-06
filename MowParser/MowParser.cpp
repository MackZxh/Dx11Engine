// MowParser.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"
#include "lexertl/memory_file.hpp"
#include "lexertl/utf_iterators.hpp"
#include "lexertl/debug.hpp"
#include "lexertl/generate_cpp.hpp"
#include <conio.h>

int main()
{
	// typedef\s*(\S*<.*>)[\s|\r|\n]*(\S[^;]*)
	// using $2 = $1
	// replace "typedef" style to "using=" style

	using urules = lexertl::basic_rules<char, char32_t>;
	using usm = lexertl::basic_state_machine<char32_t>;
	using utf_in_iter = lexertl::basic_utf8_in_iterator<const char *, char32_t>;
	using utf_out_iter = lexertl::basic_utf8_out_iterator<utf_in_iter>;
	urules rules;
	usm sm;

	lexertl::memory_file file("01-material.mtl");

	const char *begin = file.data();
	const char *end = begin + file.size();

	lexertl::recursive_match_results<utf_in_iter> results(utf_in_iter(begin, end), utf_in_iter(end, end));

	enum {
		eEOF, eString, eNumber, eBoolean, eOpenOb, eName, eCloseOb,
		eOpenArr, eCloseArr, eNull
	};

	rules.insert_macro("STRING", "[\"]([ -\\x10ffff]{-}[\"\\\\]|"
		"\\\\([\"\\\\/bfnrt]|u[0-9a-fA-F]{4}))*[\"]");
	rules.insert_macro("NUMBER", "-?(0|[1-9]\\d*)([.]\\d+)?([eE][-+]?\\d+)?");
	rules.insert_macro("BOOL", "true|false");
	rules.insert_macro("NULL", "null");


	//rules.push_state("ARRAY");
	//rules.push_state("ARR_COMMA");
	//rules.push_state("ARR_VALUE");
	//rules.push("INITIAL", "[[]", eOpenArr, ">ARRAY:END");
	//rules.push("COLON", ":", rules.skip(), "OB_VALUE");
	//rules.push("ARRAY,ARR_COMMA", "\\]", eCloseArr, "<");
	//rules.push("ARRAY,ARR_VALUE", "[[]", eOpenArr, ">ARRAY:ARR_COMMA");
	//rules.push("ARR_COMMA", ",", rules.skip(), "ARR_VALUE");
	//rules.push("ARRAY,ARR_VALUE", "{STRING}", eString, "ARR_COMMA");
	//rules.push("ARRAY,ARR_VALUE", "{NUMBER}", eNumber, "ARR_COMMA");
	//rules.push("ARRAY,ARR_VALUE", "{BOOL}", eBoolean, "ARR_COMMA");
	//rules.push("ARRAY,ARR_VALUE", "{NULL}", eNull, "ARR_COMMA");
	//rules.push("ARRAY,ARR_VALUE", "[{]", eOpenOb, ">OBJECT:ARR_COMMA");


	rules.push_state("END");
	rules.push_state("OBJECT");
	rules.push_state("NAME");
	rules.push_state("OB_VALUE");
	rules.push_state("SPACE");

	rules.push("INITIAL", "[{]", eOpenOb, ">OBJECT:END");
	rules.push("OBJECT", "[}]", eCloseOb, "<");

	rules.push("OBJECT,NAME", "{STRING}", eName, "SPACE");
	rules.push("SPACE", "[ \t]+", rules.skip(), "OB_VALUE");

	rules.push("OB_VALUE", "{STRING}", eString, ">OBJECT:END");
	rules.push("OB_VALUE", "{NUMBER}", eNumber, ">OBJECT:END");
	rules.push("OB_VALUE", "{BOOL}", eBoolean, ">OBJECT:END");
	rules.push("OB_VALUE", "{NULL}", eNull, ">OBJECT:END");
	rules.push("OB_VALUE", "[{]", eOpenOb, ">OBJECT:END");

	//rules.push("OB_COMMA", ",", rules.skip(), "NAME");

	rules.push("*", "[\r\n]+", rules.skip(), ".");

	using ugenerator = lexertl::basic_generator<urules, usm>;
	ugenerator::build(rules, sm);
	sm.minimise();

	lexertl::table_based_cpp::generate_cpp("lookup", sm, false, std::cout);

	using udebug = lexertl::basic_debug<usm, char>;
	udebug::dump(sm, std::cout);

	try
	{
		lexertl::lookup(sm, results);

		while (results.id != eEOF)
		{
			std::cout << "Id: " << results.id << " token: " <<
				std::string(utf_out_iter(results.first, results.second),
					utf_out_iter(results.second, results.second)) <<
				" state = " << results.state << '\n';
			lexertl::lookup(sm, results);
		}

		std::cout << "Stack has " << results.stack.size() << " values on it.\n";
		std::cout << results.stack.top().first << " " << results.stack.top().second;
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	_getch();

	return 0;
}

void ShowError(const char* msg) {
	std::cout << msg << std::endl;
}
// MowParser.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "lexertl/generator.hpp"
#include "lexertl/lookup.hpp"
#include "lexertl/memory_file.hpp"
#include "lexertl/utf_iterators.hpp"

int main()
{
	// typedef\s*(\S*<.*>)[\s|\r|\n]*(\S[^;]*)
	// using $2 = $1
	// replace "typedef" style to "using=" style

	using urules  =lexertl::basic_rules<char, char32_t> ;
	using usm = lexertl::basic_state_machine<char32_t>;
	using utf_in_iter = lexertl::basic_utf8_in_iterator<const char *, char32_t>;
	using utf_out_iter = lexertl::basic_utf8_out_iterator<utf_in_iter>;
	urules rules;
	usm sm;

	lexertl::memory_file file("C:\\json.txt");

	const char *begin = file.data();
	const char *end = begin + file.size();

	lexertl::recursive_match_results<utf_in_iter> results(utf_in_iter(begin, end),
		utf_in_iter(end, end));
	enum {
		eEOF, eString, eNumber, eBoolean, eOpenOb, eName, eCloseOb,
		eOpenArr, eCloseArr, eNull
	};

    return 0;
}

void ShowError(const char* msg) {
	std::cout << msg << std::endl;
}
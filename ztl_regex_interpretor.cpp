#include "forward.h"
#include "ztl_regex_interpretor.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	RegexInterpretor::RegexInterpretor(const wstring& pattern, const vector<RegexControl>& optional)
	{
		RegexLex lexer(pattern);
		lexer.ParsingPattern();
		RegexParser parser(lexer);
		parser.RegexParsing();
		this->machine = make_shared<AutoMachine>(parser);
		this->nfa = machine->BuildOptimizeNFA();
		this->control = optional;
	}
}
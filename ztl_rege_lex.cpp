#include "forward.h"
#include "ztl_regex_lex.h"
namespace ztl
{
	unordered_map<wstring, function<void(int& index, Ptr<vector<RegexToken>>& tokens)>> RegexLex::action_map;

}
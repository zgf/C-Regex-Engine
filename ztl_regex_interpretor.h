#pragma once
#include "forward.h"
namespace ztl
{
	class RegexMatchResult
	{
	public:
		//整个表达式匹配成功后得到捕获组结果
		unordered_map<wstring, wstring> group;
		//表达式匹配结果
		wstring matched;
	};
	
	class RegexInterpretor
	{
	private:
		wstring pattern;
		vector<RegexControl> control;
		Ptr<AutoMachine> machine;
		//匹配从输入的什么地方开始
		int match_start = 0;
		pair<State*,State*> nfa;
	public:
		RegexInterpretor() =delete;
		RegexInterpretor(const wstring& pattern,const vector<RegexControl>& control);
	public:
		//从指定的起始位置开始，在输入字符串中搜索正则表达式的第一个匹配项，并且仅搜索指定数量的字符。
		const vector<RegexMatchResult> 	Match(wstring& input,const int start = 0);
		//从指定的起始位置开始，判断输入字符串中是否存在正则表达式的第一个匹配项
		bool IsMatch(wstring& input,const int index = 0);
		//从字符串中的指定起始位置开始，在指定的输入字符串中搜索正则表达式的所有匹配项。
		const vector<RegexMatchResult> 	Matches(wstring& input, int start = 0);
		void Replace(wstring& input, wstring& repalce, int start);
	public:

	};
}
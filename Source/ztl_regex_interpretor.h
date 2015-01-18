#pragma once

/*
设计:
//加个预处理阶段
//对于EC 预处理pattern 所有 (捕获都替换成是(?:
//对于IC 遍历一次语法树,把char都替换一次
//MT $ ^ 预处理pattern 替换成对应的零宽断言
//RTL 从右到左匹配文本
//ODFA 替换匿名捕获,不使用捕获
//
enum class RegexControl
{
ExplicitCapture,//不匿名捕获
IgnoreCase,//大小写不敏感的匹配
Multiline,// $^ 匹配行结尾和开头
RightToLeft,//
Singleline,//.匹配所有字符
OnlyDFA,//所有匿名捕获变非捕获,不使用命名捕获,不使用高级功能
};
*/
namespace ztl
{
	class State;
	enum class RegexControl:int;
	class AutoMachine;
	class DFA;
	class CharRange;
	class GroupIterm
	{
	public:
		int		position;
		int		length;
		wstring content;
	};
	class RegexMatchResult
	{
	public:
		//整个表达式匹配成功后得到捕获组结果
		unordered_map<wstring, GroupIterm>  group;
		vector<GroupIterm>					anonymity_group;

		//表达式匹配结果
		wstring								matched;
		//匹配在串的位置
		int									start;
		int									length;
		bool								success = false;
	public:
		int Length() const
		{
			return length;
		}
		wstring Content() const
		{
			return matched;
		}
		pair<int,int> Position()const
		{
			return { start, length };
		}
		bool Success() const
		{
			return success;
		}

	};
	class SaveState
	{
	public:
		State*			 states;
		int		         edge_index;
		int	             input_index;
		//当前状态通过当前边消耗的字符长度
		int	             length = 0;
		//	bool             meet_final;
		SaveState() = default;
		SaveState(State* _state, int edge, int input) :states(_state), edge_index(edge), input_index(input)
		{
		}
	};
	class RegexInterpretor
	{
	public:
		wstring						pattern;
		shared_ptr<vector<RegexControl>>   optional;
		shared_ptr<AutoMachine>			machine;
	public:
		RegexInterpretor() = delete;
		RegexInterpretor(const wstring& pattern, const shared_ptr<vector<RegexControl>>_optional);
		RegexInterpretor(const wstring& pattern);

	public:
		//从指定的起始位置开始，在输入字符串中搜索正则表达式的第一个匹配项，并且仅搜索指定数量的字符。
		RegexMatchResult 				Match(const wstring& input, const int start = 0);
		RegexMatchResult 				Match(const wstring& input, wstring::const_iterator start);
		//从指定的起始位置开始，判断输入字符串中是否存在正则表达式的第一个匹配项
		bool							IsMatch(const wstring& input, const int start = 0);
		bool							IsMatch(const wstring& input, wstring::const_iterator start);

		//从字符串中的指定起始位置开始，在指定的输入字符串中搜索正则表达式的所有匹配项。
		const vector<RegexMatchResult> 	Matches(const wstring& input, int start = 0);
		const vector<RegexMatchResult> 	Matches(const wstring& input, wstring::const_iterator start);

		wstring							Replace(const wstring& input, const wstring& repalce, int start = 0);
		wstring							Replace(const wstring& input, const wstring& repalce, wstring::const_iterator start);
	private:
		//在start到end范围内寻找正则的第一个匹配
		template<typename iterator_type>
		RegexMatchResult RegexMatchOne(const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);
		//在start到end范围内寻找正则的全部匹配
		template<typename iterator_type>
		vector<RegexMatchResult> RegexMatchAll(const wstring& input, iterator_type start, iterator_type end);
			//DFA 匹配,从start开始,不移动start,看能否到达终结状态
		//结果保存在save_stack.back()内
		RegexMatchResult MatchSucced(const wstring& input, vector<SaveState>& save_stack, RegexMatchResult& result);
		RegexMatchResult MatchFailed();

	public:
		int GetWCharIndex(const wchar_t character)const;
		template<typename iterator_type>
		RegexMatchResult NFAMatch(const pair<State*,State*>& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);

		template<typename iterator_type>
		bool DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);
	};
	wstring LinearStringToRegexString(const wstring& literal);
}
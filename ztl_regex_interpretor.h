#pragma once
#include "forward.h"
#include "ztl_regex_automachine.h"
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
	class GroupIterm
	{
	public:
		int position;
		int length;
		wstring content;
	};
	class RegexMatchResult
	{
	public:
		//整个表达式匹配成功后得到捕获组结果
		unordered_map<wstring, GroupIterm> group;
		//表达式匹配结果
		wstring matched;
		//匹配在串的位置
		int start;
		int end;
		bool success;
	};
	class SaveState
	{
	public:
		State* states;
		int    edge_index;
		int	   input_index;
		bool meet_final;
		SaveState() = default;
		SaveState(State* _state, int edge, int input) :states(_state), edge_index(edge), input_index(input)
		{

		}
	};
	class RegexInterpretor
	{
	private:
		wstring pattern;
		vector<RegexControl> control;
		Ptr<AutoMachine> machine;
		pair<State*,State*> nfa;
		vector<SaveState> state_stack;
	private:
		static unordered_map<Edge::EdgeType, function<bool(const wstring& input, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& machine, RegexMatchResult& result)>> actions;
	public:
		RegexInterpretor() =delete;
		RegexInterpretor(const wstring& pattern,const vector<RegexControl>& control);
	public:
		//从指定的起始位置开始，在输入字符串中搜索正则表达式的第一个匹配项，并且仅搜索指定数量的字符。
		RegexMatchResult 	Match(const wstring& input,const int start = 0);
		//从指定的起始位置开始，判断输入字符串中是否存在正则表达式的第一个匹配项
		bool IsMatch(const wstring& input,const int index = 0);
		//从字符串中的指定起始位置开始，在指定的输入字符串中搜索正则表达式的所有匹配项。
		const vector<RegexMatchResult> 	Matches(const wstring& input, int start = 0);
		wstring Replace(const wstring& input,const wstring& repalce, int start);
	private:
		void RightToLeft(const wstring& text);
		//在start到end范围内寻找正则的第一个匹配
		RegexMatchResult RegexMatchOne(const wstring& input, const int start, const int end);
		//在start到end范围内寻找正则的全部匹配
		vector<RegexMatchResult> RegexMatchAll(const wstring& input, const int start, const int end);
	};
}
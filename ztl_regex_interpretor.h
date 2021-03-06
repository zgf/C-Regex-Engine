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
		Ptr<vector<RegexControl>>   optional;
		Ptr<AutoMachine>			machine;
	public:
		RegexInterpretor() = delete;
		RegexInterpretor(const wstring& pattern, const Ptr<vector<RegexControl>>_optional);
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
		RegexMatchResult NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);

		template<typename iterator_type>
		bool DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);
	};
	template<typename iterator_type>
	RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
	{
		RegexMatchResult result;
		auto current_input_index = input_index;
		if(machine->dfa_expression != nullptr)
		{
			while(current_input_index < end)
			{
				SaveState save;
				if(DFAMatch(*machine->dfa_expression, save, input, current_input_index, start, end))
				{
					//成功匹配
					result.success = true;
					result.start = save.input_index;
					result.length = save.length;
					result.matched = input.substr(save.input_index, save.length);
					return result;
				}
				else
				{
					current_input_index++;
				}
			}
		}
		else
		{
			while(current_input_index < end)
			{
				result = NFAMatch(*machine->nfa_expression, input, current_input_index, start, end);
				if(result.success == true)
				{
					return result;
				}
				current_input_index++;
			}
		}
		return result;
	}

	//在start到end范围内寻找正则的全部匹配
	template<typename iterator_type>
	vector<RegexMatchResult> RegexInterpretor::RegexMatchAll(const wstring& input, iterator_type start, iterator_type end)
	{
		vector<RegexMatchResult> result;
		auto next_start_index = start;
		while(next_start_index < end)
		{
			auto match_result = RegexMatchOne(input, next_start_index, start, end);
			if(match_result.success == true)
			{
				result.emplace_back(move(match_result));
				next_start_index = start + match_result.start + match_result.length;
			}
			else
			{
				next_start_index += 1;
			}
		}
		return result;
	}
	template<typename iterator_type>
	bool RegexInterpretor::DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
	{
		auto& table = dfa.dfa;
		auto& finalset = dfa.finalset;
		auto final_index = machine->table->range_table->size();
		//从0开始
		auto current_state_index = 0;
		auto current_input_index = input_index;
		save_state.length = 0;
		save_state.input_index = input_index - start;
		//追踪含final边的DFA 第一个参数是DFA状态号,第二个参数是当时的input_index
		//vector<pair<int, int>> tracks;
		while(current_state_index != finalset)
		{
			if(current_input_index != end)
			{
				auto index = GetWCharIndex(*current_input_index);
				if(table[current_state_index][index] != -1)
				{
					current_state_index = table[current_state_index][index];
					current_input_index++;
					save_state.length++;
				}
				else if(table[current_state_index][final_index] != -1)
				{
					current_state_index = table[current_state_index][final_index];
				}
				else
				{
					return false;
				}

			}
			else if(table[current_state_index][final_index] != -1)
			{
				current_state_index = table[current_state_index][final_index];
			}
			else
			{
				return false;
			}
		}
		//有可能DFA会不捕获字符, a* 匹配 bbbaaa因为a*可以匹配空串,所以bbbaaa会匹配成功

		return save_state.length != 0;
	}

	//NFA 匹配,从start开始,不移动start,看能否到达终结状态
	//边都排序一次,final放最后,不移动start,只匹配一次.
	template<typename iterator_type>
	RegexMatchResult RegexInterpretor::NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
	{
		RegexMatchResult result;
		bool is_new_state = true;
		//	const auto& finalset = nfa.second;
		auto current_state = nfa.first;
		auto current_input_index = input_index;

		auto current_edge_index = 0;
		//设置进入时栈深度
		vector<SaveState> state_stack;
		state_stack.emplace_back(SaveState());
		result.anonymity_group.resize(machine->anonymity_captures.size());
		while(true)
		{
			if(current_state == nfa.second)
			{
				state_stack.pop_back();
				auto sumlength = std::accumulate(state_stack.begin(), state_stack.end(), 0, [](const int& left, const SaveState& save)
				{
					assert(save.length != -1);
					return left + save.length;
				});

				auto& match_result = IsSucceedMatch(sumlength, input, input_index, start, end, state_stack, result);
				if(match_result.success == true)
				{
					return match_result;
				}
				else if(!state_stack.empty())
				{
					is_new_state = false;
				}
				else
				{
					return MatchFailed();
				}
			}
			auto& save = state_stack.back();
			//设置save和各个变量状态
			SetState(is_new_state, save, current_edge_index, start, current_input_index, current_state);
			for(; current_edge_index < current_state->output.size(); current_edge_index++)
			{
				save.edge_index = current_edge_index;
				bool match_result = StateGoto(current_state->output[current_edge_index]->type, *this, input, start, end, current_input_index, state_stack, result);
				if(match_result == true)
				{
					current_state = current_state->output[current_edge_index]->target;
					state_stack.emplace_back(SaveState());
					is_new_state = true;
					goto LoopEnd;
				}
			}
			//所有边都匹配失败
			//不需要看看有没有遇到过final
			//到达final边时是最后一条边.然后直接跳出while
			//弹出当前状态,执行上一状态的下一条边
			state_stack.pop_back();
			is_new_state = false;

			if(state_stack.empty())
			{
				//所有的都失败了.没有上一状态报告失败
				return MatchFailed();
			}
		LoopEnd:;
		}
	}
}
#include "forward.h"
#include "ztl_regex_interpretor.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	void RegexInterpretor::RightToLeft(const wstring& input)
	{
		if(find(control.begin(), control.end(), RegexControl::RightToLeft) != control.end())
		{
			reverse(input.begin(), input.end());
		}
	}
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
	//为了贪婪匹配 边需要重排序,final放到最后.遇到final后,直接跳出
	RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, const int start, const int end)
	{
		State* current_state = nfa.first;
		assert(!nfa.first->output.empty());
		auto edge_index = 0;
		auto current_input_index = start;
		RegexMatchResult result;
		bool meet_final;
		while(current_input_index != end)
		{
			this->state_stack.emplace_back(SaveState(current_state, 0, current_input_index));
			auto stack_index = state_stack.size() - 1;
			for(auto i = 0; i < current_state->output.size();)
			{
				//先保存当前状态
				Edge* current_edge = current_state->output[i];
				if(current_edge->type != Edge::EdgeType::Final)
				{
					auto&& match_edge_result = RegexInterpretor::actions[current_edge->type](input, current_input_index, current_state, current_edge, *this, result);
					if(match_edge_result == true)
					{
						//进入下一个状态
						current_state = current_state->output[i]->target;
						break;
					}
					else
					{
						state_stack[stack_index].edge_index++;
						i++;
					}
				} 
				else
				{
					state_stack[stack_index].meet_final = true;
				}
			}
			if(state_stack[stack_index].meet_final == true)
			{
				return;
			}
		}
		
	}
}
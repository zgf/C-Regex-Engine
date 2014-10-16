#include "forward.h"
#include "ztl_regex_interpretor.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	RegexInterpretor::ActionType RegexInterpretor::actions = RegexInterpretor::InitActions();
	void RegexInterpretor::RightToLeft(wstring& input)
	{
		if(find(optional->begin(), optional->end(), RegexControl::RightToLeft) != optional->end())
		{
			reverse(input.begin(), input.end());
		}
	}
	RegexInterpretor::RegexInterpretor(const wstring& pattern, const Ptr<vector<RegexControl>>_optional)
	:char_table(65535)
	{
		RegexLex lexer(pattern);
		lexer.ParsingPattern();
		RegexParser parser(lexer);
		parser.RegexParsing();
		this->machine = make_shared<AutoMachine>(parser);
		machine->BuildOptimizeNFA();
		this->optional = _optional;
	
	}
	void RegexInterpretor::InitTable()
	{
	
	}
	RegexInterpretor::ActionType RegexInterpretor::InitActions()
	{
		RegexInterpretor::ActionType actions;
		//actions.insert({ Edge::EdgeType::Char, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	auto& current_char = input[index];
		//	//这里要查表才行
		//	if(any_cast<int>(current_edge->userdata) == interpretor.char_table[current_char])
		//	{
		//		index++;
		//		return true;
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//} });
		//actions.insert({ Edge::EdgeType::Loop, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		//actions.insert({ Edge::EdgeType::Capture, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		//actions.insert({ Edge::EdgeType::BackReference, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	auto&& find_result = interpretor.machine->captures->find(any_cast<wstring>(current_edge->userdata));
		//	if(find_result != interpretor.machine->captures->end())
		//	{
		//
		//		return 0;
		//	}
		//	else
		//	{
		//		return 0;
		//	}
		//} });
		//actions.insert({ Edge::EdgeType::Head, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	if(index == begin)
		//	{
		//		index++;
		//		return 0;
		//	}
		//	else
		//	{
		//		return 0;
		//	}
		//} });
		//actions.insert({ Edge::EdgeType::Tail, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	if (index == end)
		//	{
		//		index++;
		//		return true;
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//} });
		//actions.insert({ Edge::EdgeType::PositivetiveLookahead, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		//actions.insert({ Edge::EdgeType::PositiveLookbehind, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		//actions.insert({ Edge::EdgeType::NegativeLookahead, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		//actions.insert({ Edge::EdgeType::NegativeLookbehind, [](const wstring& input, const int begin, const int end, int& index, const State* current_state, const Edge* current_edge, RegexInterpretor& interpretor, RegexMatchResult& result)
		//{
		//	return 0;
		//} });
		return move(actions);
	}

	RegexMatchResult MatchFail()
	{
		RegexMatchResult result;
		result.success = false;
		return move(result);
	}

	//状态机这块代码太乱了- -

	//为了贪婪匹配 边需要重排序,final放到最后.遇到final后,直接跳出
	RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, const int start, const int end)
	{
		RegexMatchResult result;
		State* current_state = machine->nfa_expression->first;
		assert(!machine->nfa_expression->first->output.empty());
		auto current_input_index = start;
		size_t current_edge_index = 0;
		bool enter_next = false;
		while(current_input_index != end)
		{
			//先保存当前状态
			this->state_stack.emplace_back(SaveState(current_state, 0, current_input_index));
			auto stack_index = state_stack.size() - 1;
			for(current_edge_index = 0; current_edge_index < current_state->output.size();)
			{
				Edge* current_edge = current_state->output[current_edge_index];
				if(current_edge->type != Edge::EdgeType::Final)
				{
					auto&& new_input_index = RegexInterpretor::actions[current_edge->type]
						(input, start, end, current_input_index, current_state, current_edge, *this, result);
					if(new_input_index != -1)
					{
						//进入下一个状态
						state_stack.back().length = new_input_index - current_input_index;
						current_state = current_state->output[current_edge_index]->target;
						enter_next = true;
						current_input_index = new_input_index;
						break;
					}
					else
					{
						state_stack[stack_index].edge_index++;
						current_edge_index++;
					}
				}
				else
				{
					state_stack[stack_index].meet_final = true;
				}
			}

			if(enter_next == true)
			{
				enter_next = false;
	
			}//边全部执行完了并失败或者遇到过final.
			else if(state_stack[stack_index].meet_final == true)
			{
				result.success = true;
				//填充result;
				return move(result);
			}
			else if(state_stack.size() > 1)
			{
				//弹出当前状态,执行上一状态的下一条边
				state_stack.pop_back();

				//回退到上一状态
				current_state = state_stack.back().states;
				current_edge_index = state_stack.back().edge_index;
				current_input_index = state_stack.back().input_index;
				state_stack.pop_back();
			}
			else
			{
				//如果是空.说明匹配失败,没有状态能匹配
				return MatchFail();
			}
		}
		//跳出了就说明失败了
		result.success = false;
		//填充result;
		return MatchFail();
	}
}
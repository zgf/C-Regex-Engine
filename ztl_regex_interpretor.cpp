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
		:optional(_optional)

	{
		RegexLex lexer(pattern, optional);
		lexer.ParsingPattern();
		RegexParser parser(lexer, optional);
		parser.RegexParsing();
		this->machine = make_shared<AutoMachine>(parser);
		machine->BuildOptimizeNFA();
	
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

	RegexMatchResult RegexInterpretor::MatchSucced()
	{
		return {};
	}

	RegexMatchResult RegexInterpretor::MatchFailed()
	{
		RegexMatchResult result;
		result.success = false;
		return move(result);
	}
	
	void RegexInterpretor::DFAMatch(const DFA& dfa,const wstring& input, const int start)
	{
		auto& table = dfa.dfa;
		auto& finalset = dfa.finalset;
		auto& save_state = this->state_stack.back();
		auto final_index = machine->table->range_table->size();
		//从0开始
		auto current_state_index = 0;
		auto current_input_index = start;
		while(find(finalset.begin(),finalset.end(),current_state_index)!=finalset.end())
		{
			if (table[current_state_index][current_input_index] != -1)
			{
				current_input_index++;
				current_state_index = table[current_state_index][current_input_index];
				save_state.length++;
			}
			else if(table[current_state_index][final_index] != -1)
			{
				current_state_index = table[current_state_index][final_index];
			}
			else
			{
				save_state.length = 0;
				break;
			}
		}
	}

	bool MatchEdge(Edge*& edge, const wstring& input, const int current_input_index, SaveState& save)
	{
		return {};
	}

	//边都排序一次,final放最后
	RegexMatchResult RegexInterpretor::NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, const int start)
	{
		RegexMatchResult result;
		bool is_new_state = true;
		const auto& finalset = nfa.second;
		auto current_state = nfa.first;
		auto current_input_index = start;
		auto current_edge_index = 0;
		state_stack.emplace_back(SaveState());
		while(current_state != nfa.second)
		{
		LoopStart:
			auto& save = state_stack.back();
			if (is_new_state == true)
			{//状态是第一次进入
				save.input_index = current_input_index;
				save.states = current_state;
				save.edge_index = -1;
				save.length = -1;
				current_edge_index = 0;
			}
			else
			{//状态不是第一次进入
				current_edge_index = save.edge_index + 1;
				current_input_index = save.input_index;
				current_state = save.states;
				save.length = -1;
			}
			
			for(; current_edge_index < current_state->output.size();current_edge_index++)
			{
				save.edge_index = current_edge_index;
				auto&& match_result = MatchEdge(current_state->output[current_edge_index], input, current_input_index, save);
				if(match_result == true)
				{
					current_state = current_state->output[current_edge_index]->target;
					state_stack.emplace_back(move(save));
					state_stack.emplace_back(SaveState());
					goto LoopStart;
				}
			}
			//所有边都匹配失败
			//弹出当前状态,执行上一状态的下一条边
			state_stack.pop_back();
			if (state_stack.empty())
			{
				//所有的都失败了.没有上一状态报告失败
				break;
			}
		}

		if (current_state == nfa.second)
		{
			return MatchSucced();
		}

		return MatchFailed();

	}
}
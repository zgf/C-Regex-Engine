#include "forward.h"
#include "ztl_regex_lex.h"
#include "ztl_regex_parser.h"
#include "ztl_regex_automachine.h"
#include "ztl_regex_interpretor.h"
namespace ztl
{

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
	RegexInterpretor::RegexInterpretor(const wstring& pattern, const Ptr<vector<RegexControl>>_optional)
		:optional(_optional)

	{
		RegexLex lexer(pattern, optional);
		lexer.ParsingPattern();
		RegexParser parser(lexer, optional);
		parser.RegexParsing();
		this->machine = make_shared<AutoMachine>(parser);
		machine->BuildOptimizeNFA();
		//anonymity_capture_value.resize(machine->subexpression->size());
	}
	RegexInterpretor::RegexInterpretor(const wstring& pattern) :optional(make_shared<vector<RegexControl>>())
	{
		RegexLex lexer(pattern);
		lexer.ParsingPattern();
		RegexParser parser(lexer);
		parser.RegexParsing();
		this->machine = make_shared<AutoMachine>(parser);
		machine->BuildOptimizeNFA();
	}

	bool Final( vector<SaveState>& save_stack)
	{
		save_stack.back().length = 0;
		return true;
	}
	template<typename iterator_type>
	bool Head(iterator_type begin,  iterator_type& input_index, vector<SaveState>& save_stack)
	{
		if(input_index == begin)
		{
			save_stack.back().length = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	template<typename iterator_type>
	bool Tail(iterator_type end, iterator_type& input_index,vector<SaveState>& save_stack)
	{
		if(input_index == end)
		{
			save_stack.back().length = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
	template<typename iterator_type>

	bool Char(iterator_type end, iterator_type& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack)
	{
		if(input_index == end)
		{
			return false;
		}
		auto expect_index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		auto char_index = interpretor.GetWCharIndex(*input_index);
		if(expect_index == char_index)
		{
			save_stack.back().length = 1;
			input_index += 1;
			return true;
		}
		else
		{
			return false;
		}
	}
	template<typename iterator_type>

	bool BackReference(const wstring& input, iterator_type begin, iterator_type end, iterator_type& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		auto&& name = any_cast<wstring>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		auto& expect_value = result.group[name].content;

		auto length = expect_value.size();
		auto&& real_value = input.substr(input_index - begin, length);

		if(real_value == expect_value)
		{
			save_stack.back().length = length;
			input_index += length;
			return true;
		}
		else
		{
			save_stack.back().length = 0;
			return false;
		}
	}
	template<typename iterator_type>

	bool AnonymityBackReference(const wstring& input, iterator_type begin, iterator_type end, iterator_type& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		auto&& name = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		name -= 1;
		auto& expect_value = result.anonymity_group[name].content;
		auto length = expect_value.size();
		auto&& real_value = input.substr(input_index - begin, length);

		if(real_value == expect_value)
		{
			save_stack.back().length = length;
			input_index += length;
			return true;
		}
		else
		{
			save_stack.back().length = 0;
			return false;
		}
	}
	template<typename iterator_type>

	bool Capture(const wstring& input, iterator_type begin, iterator_type end, iterator_type& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		auto name = any_cast<wstring>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_captures.find(name) != interpretor.machine->dfa_captures.end())
		{
			auto subdfa = interpretor.machine->dfa_captures[name];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, input_index, begin, end);
			if(find_result == true)
			{
				result.group[name].content = input.substr(save.input_index, save.length);
				result.group[name].position = save.input_index;
				result.group[name].length = save.length;
				input_index += save.length;
				save_stack.back().length = save.length;
				save_stack.back().input_index = save.input_index;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->captures[name];
			auto&& find_result = interpretor.NFAMatch(subexpression, input, input_index, begin, end);
			if(find_result.success == true)
			{
				result.group = move(find_result.group);
				result.anonymity_group = move(find_result.anonymity_group);

				result.group[name].content = find_result.matched;
				result.group[name].position = find_result.start;
				result.group[name].length = find_result.length;
				save_stack.back().length = find_result.length;
				input_index += find_result.length;

				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	template<typename iterator_type>

	bool AnonymityCapture(const wstring& input, iterator_type begin, iterator_type end, iterator_type& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		int name = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_anonymity_captures.find(name) != interpretor.machine->dfa_anonymity_captures.end())
		{
			auto subdfa = interpretor.machine->dfa_anonymity_captures[name];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, input_index, begin, end);

			if(find_result == true)
			{
				result.anonymity_group[name].content = input.substr(save.input_index, save.length);
				result.anonymity_group[name].position = save.input_index;
				result.anonymity_group[name].length = save.length;
				input_index += save.length;
				save_stack.back().length = save.length;
				save_stack.back().input_index = save.input_index;

				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->anonymity_captures[name];
			auto&& find_result = interpretor.NFAMatch(subexpression.first, input, input_index, begin, end);
			if(find_result.success == true)
			{
				result.group = move(find_result.group);

				result.anonymity_group = move(find_result.anonymity_group);
				result.anonymity_group[name].content = find_result.matched;
				result.anonymity_group[name].position = find_result.start;
				result.anonymity_group[name].length = find_result.length;
				save_stack.back().length = find_result.length;
				input_index += find_result.length;

				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	template<typename iterator_type>

	bool PositivetiveLookahead(const wstring& input, iterator_type begin, iterator_type end, iterator_type input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			auto current_input_index = input_index;
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, current_input_index, begin, end);
			if(find_result == true)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto current_input_index = input_index;
			auto&& find_result = interpretor.NFAMatch(subexpression, input, current_input_index, begin, end);
			if(find_result.success == true)
			{
				result.group = move(find_result.group);
				result.anonymity_group = move(find_result.anonymity_group);
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}

	bool PositiveLookbehind(const wstring& input, wstring::const_iterator begin, wstring::const_iterator end, wstring::const_iterator input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		wstring::const_reverse_iterator rindex(input_index);
		wstring::const_reverse_iterator rbegin(end);
		wstring::const_reverse_iterator rend(begin);
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, rindex, rbegin, rend);
			if(find_result == true)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto&& find_result = interpretor.NFAMatch(subexpression, input, rindex, rbegin, rend);
			if(find_result.success == true)
			{
				result.group = move(find_result.group);
				result.anonymity_group = move(find_result.anonymity_group);
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	bool PositiveLookbehind(const wstring& input, wstring::const_reverse_iterator begin, wstring::const_reverse_iterator end, wstring::const_reverse_iterator input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		wstring::const_iterator rindex(input_index.base());
		wstring::const_iterator rbegin(end.base());
		wstring::const_iterator rend(begin.base());
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, rindex, rbegin, rend);
			if(find_result == true)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto&& find_result = interpretor.NFAMatch(subexpression, input, rindex, rbegin, rend);
			if(find_result.success == true)
			{
				result.group = move(find_result.group);
				result.anonymity_group = move(find_result.anonymity_group);
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	
	bool NegativeLookbehind(const wstring& input, wstring::const_iterator begin, wstring::const_iterator end, wstring::const_iterator input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		wstring::const_reverse_iterator rindex(input_index);
		wstring::const_reverse_iterator rbegin(end);
		wstring::const_reverse_iterator rend(begin);
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, rindex, rbegin, rend);
			if(find_result == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto&& find_result = interpretor.NFAMatch(subexpression, input, rindex, rbegin, rend);
			if(find_result.success == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	bool NegativeLookbehind(const wstring& input, wstring::const_reverse_iterator begin, wstring::const_reverse_iterator end, wstring::const_reverse_iterator input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		wstring::const_iterator rindex(input_index.base());
		wstring::const_iterator rbegin(end.base());
		wstring::const_iterator rend(begin.base());
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, rindex, rbegin, rend);
			if(find_result == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto&& find_result = interpretor.NFAMatch(subexpression, input, rindex, rbegin, rend);
			if(find_result.success == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	template<typename iterator_type>

	bool NegativeLookahead(const wstring& input, iterator_type begin, iterator_type end, iterator_type input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
		if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
		{
			auto subdfa = interpretor.machine->dfa_subexpression[index];
			auto current_input_index = input_index;
			SaveState save;
			auto&& find_result = interpretor.DFAMatch(subdfa, save, input, current_input_index, begin, end);
			if(find_result == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
		else
		{
			auto& subexpression = interpretor.machine->subexpression[index];
			auto current_input_index = input_index;
			auto&& find_result = interpretor.NFAMatch(subexpression, input, current_input_index, begin, end);
			if(find_result.success == false)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				save_stack.back().length = 0;
				return false;
			}
		}
	}
	

	RegexMatchResult RegexInterpretor::MatchSucced(const wstring& input, vector<SaveState>& save_stack, RegexMatchResult& result)
	{
		save_stack.pop_back();
		assert(!save_stack.empty());
		auto sumlength = 0;
		for(auto&& element : save_stack)
		{
			sumlength += element.length;
		}
		if(sumlength != 0)
		{
			result.length = sumlength;
			result.start = save_stack.front().input_index;
			result.matched = input.substr(result.start, result.length);
			result.success = true;
			return result;
		}
		else
		{
			return MatchFailed();
		}
	}

	RegexMatchResult RegexInterpretor::MatchFailed()
	{
		RegexMatchResult result;
		result.success = false;
		return move(result);
	}
	inline int RegexInterpretor::GetWCharIndex(const wchar_t character)const
	{
		return (*machine->table->char_table)[character];
	}
	

	//根据是否是第一次进入,设置状态
	template<typename iterator_type>
	void SetState(bool is_new_state,  SaveState& save, int& current_edge_index, iterator_type begin, iterator_type&  current_input_index, State*& current_state)
	{
		if(is_new_state == true)
		{//状态是第一次进入
			save.input_index = current_input_index - begin;
			save.states = current_state;
			save.edge_index = -1;
			save.length = -1;
			current_edge_index = 0;
		}
		else
		{//状态不是第一次进入
			current_edge_index = save.edge_index + 1;
			current_input_index = begin + save.input_index;
			current_state = save.states;
			save.length = -1;
		}
	}
	template<typename iterator_type>
	RegexMatchResult& IsSucceedMatch(const int sumlength, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end, vector<SaveState>& state_stack, RegexMatchResult& result)
	{
		if(sumlength != 0)
		{
			//从第一个捕获>0开始
			result.length = sumlength;
			result.start = (*find_if(state_stack.begin(), state_stack.end(), [](const SaveState& save)
			{
				return save.length > 0;
			})).input_index;
			result.matched = input.substr(result.start, result.length);
			result.success = true;
			return result;
		}
		else if(input_index == end)
		{
			result.length = 0;
			result.start = input_index - start;
			result.success = true;
			return result;
		}
		else
		{
			return result;
		}
	}
	template<typename iterator_type>
	bool StateGoto(Edge::EdgeType edge_type, RegexInterpretor& itereptor, const wstring& input, iterator_type start, iterator_type end, iterator_type& current_input_index, vector<SaveState>& state_stack, RegexMatchResult& result)
	{
		bool match_result = false;
		switch(edge_type)
		{
			case Edge::EdgeType::Final:
				match_result = Final( state_stack);
				break;
			case Edge::EdgeType::Head:
				match_result = Head( start,  current_input_index, state_stack);

				break;
			case Edge::EdgeType::Tail:
				match_result = Tail( end, current_input_index,state_stack);

				break;
			case Edge::EdgeType::Char:
				match_result = Char(end, current_input_index, itereptor, state_stack);

				break;
			case Edge::EdgeType::Capture:
				match_result = Capture(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::AnonymityCapture:
				match_result = AnonymityCapture(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::BackReference:
				match_result = BackReference(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::AnonymityBackReference:
				match_result = AnonymityBackReference(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::PositiveLookbehind:
				match_result = PositiveLookbehind(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::PositivetiveLookahead:
				match_result = PositivetiveLookahead(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::NegativeLookbehind:
				match_result = NegativeLookbehind(input, start, end, current_input_index, itereptor, state_stack, result);

				break;
			case Edge::EdgeType::NegativeLookahead:
				match_result = NegativeLookahead(input, start, end, current_input_index, itereptor, state_stack, result);
				break;
			default:
				assert(false);
				break;
		}
		return match_result;
	}

	

	RegexMatchResult RegexInterpretor::Match(const wstring& input, const int start)
	{
		return Match(input, input.begin() + start);
	}
	RegexMatchResult 	RegexInterpretor::Match(const wstring& input, wstring::const_iterator start)
	{
		if(find(optional->begin(), optional->end(), RegexControl::RightToLeft) != optional->end())
		{
		
			return RegexMatchOne(input, wstring::const_reverse_iterator(input.end()), wstring::const_reverse_iterator(input.end()), wstring::const_reverse_iterator(start));
		}
		else
		{
			return RegexMatchOne(input, start, input.begin(), input.end());
		}
	}
	//从指定的起始位置开始，判断输入字符串中是否存在正则表达式的第一个匹配项
	bool RegexInterpretor::IsMatch(const wstring& input, const int start)
	{
		return IsMatch(input, input.begin() + start);
	}
	bool RegexInterpretor::IsMatch(const wstring& input, wstring::const_iterator start)
	{
		if(find(optional->begin(), optional->end(), RegexControl::RightToLeft) != optional->end())
		{
			return RegexMatchOne(input, wstring::const_reverse_iterator(input.end()), wstring::const_reverse_iterator(input.end()), wstring::const_reverse_iterator(start)).success;
		}
		else
		{
			return RegexMatchOne(input, start,input.begin(), input.end()).success;
		}
	}
	//从字符串中的指定起始位置开始，在指定的输入字符串中搜索正则表达式的所有匹配项。
	const vector<RegexMatchResult> RegexInterpretor::Matches(const wstring& input, int start)
	{
		return Matches(input, input.begin() + start);
	}
	const vector<RegexMatchResult> 	RegexInterpretor::Matches(const wstring& input, wstring::const_iterator start)
	{
		if(find(optional->begin(), optional->end(), RegexControl::RightToLeft) != optional->end())
		{
			return RegexMatchAll(input, wstring::const_reverse_iterator(input.end()), wstring::const_reverse_iterator(start));
		}
		else
		{
			return RegexMatchAll(input, start, input.end());
		}
	}
	wstring RegexInterpretor::Replace(const wstring& input, const wstring& repalce, int start)
	{
		return Replace(input, repalce, input.begin() + start);
	}
	wstring	RegexInterpretor::Replace(const wstring& input, const wstring& repalce, wstring::const_iterator start)
	{
		auto target = Matches(input, start);
		wstring result;
		auto begin = input.begin();
		auto next = begin;
		for(auto i = 0; i < target.size(); i++)
		{
			auto& element = target[i];
			result.insert(result.end(), next, begin + element.start);
			result.insert(result.end(), repalce.begin(), repalce.end());
			next = begin + element.start + element.length;
		}
		result.insert(result.end(), next, input.end());
		return result;
	}
	wstring LinearStringToRegexString(const wstring& literal)
	{
		wstring result;
		for(int i = 0; i < literal.size(); i++)
		{
			auto c = literal[i];
			switch(c)
			{
				case L'(':
				case L')':
				case L'{':
				case L'}':
				case L'[':
				case L']':
				case L'\\':
				case L'.':
				case L'|':
				case L'\n':
				case L'\t':
				case L'\r':
				case L'^':
				case L'$':
				case L'+':
				case L'*':
				case L'?':
					result += LR"(\)" + wstring(1,c);
					break;
				default:
					result+=c;
					break;
			}
		}
		return result;
	}
}
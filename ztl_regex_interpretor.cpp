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
		//anonymity_capture_value.resize(machine->subexpression->size());
	}
	bool RegexInterpretor::BackReferenceAction(const wstring& input, int& input_index, SaveState& save, const wstring& expect_value)
	{
		auto length = expect_value.size();
		auto&& real_value = input.substr(input_index, length);

		if(real_value == expect_value)
		{
			save.length = length;
			input_index += length;
			return true;
		}
		else
		{
			save.length = 0;
			return false;
		}
	}
	
	RegexInterpretor::ActionType RegexInterpretor::InitActions()
	{
		RegexInterpretor::ActionType actions;

		/*
					Capture,
					BackReference,
					//Loop, Loop被展开成JUmp E Char Edge
					Char,
					Head,
					Tail,
					PositivetiveLookahead,
					NegativeLookahead,
					PositiveLookbehind,
					NegativeLookbehind,
					Final, //边后面是终结状态
					AnonymityCapture,
					AnonymityBackReference,
					*/

		actions.insert({ Edge::EdgeType::Final, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			save_stack.back().length = 0;
			return true;
		} });

		actions.insert({ Edge::EdgeType::Head, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			if(input_index == start)
			{
				save_stack.back().length = 0;
				return true;
			}
			else
			{
				return false;
			}
		} });
		actions.insert({ Edge::EdgeType::Tail, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
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
		} });
		actions.insert({ Edge::EdgeType::Char, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto expect_index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			auto char_index = (*interpretor.machine->table->char_table)[input[input_index]];
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
		} });
		actions.insert({ Edge::EdgeType::BackReference, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto&& name = any_cast<wstring>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			auto& expect_value = result.group[name].content;

			return BackReferenceAction(input, input_index, save_stack.back(), expect_value);
		} });
		actions.insert({ Edge::EdgeType::AnonymityBackReference, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto&& name = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			name -= 1;
			auto& expect_value = result.anonymity_group[name].content;
			return BackReferenceAction(input, input_index, save_stack.back(), expect_value);
		} });
		actions.insert({ Edge::EdgeType::Capture, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto name = any_cast<wstring>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_captures.find(name) != interpretor.machine->dfa_captures.end())
			{
				auto subdfa = interpretor.machine->dfa_captures[name];
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), input, input_index, end);
				if(find_result == true)
				{
					result.group[name].content = input.substr(save_stack.back().input_index, save_stack.back().length);
					result.group[name].position = save_stack.back().input_index;
					result.group[name].length = save_stack.back().length;
					input_index += save_stack.back().length;
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
				auto&& find_result = interpretor.NFAMatch(subexpression, input, input_index, end);
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
		} });
		actions.insert({ Edge::EdgeType::AnonymityCapture, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			int name = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_anonymity_captures.find(name) != interpretor.machine->dfa_anonymity_captures.end())
			{
				auto subdfa = interpretor.machine->dfa_anonymity_captures[name];
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), input, input_index, end);
				if(find_result == true)
				{
					result.anonymity_group[name].content = input.substr(save_stack.back().input_index, save_stack.back().length);
					result.anonymity_group[name].position = save_stack.back().input_index;
					result.anonymity_group[name].length = save_stack.back().length;
					input_index += save_stack.back().length;
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
				auto&& find_result = interpretor.NFAMatch(subexpression.first, input, input_index, end);
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
		} });
		/*
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,

		*/

		actions.insert({ Edge::EdgeType::PositivetiveLookahead, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
			{
				auto subdfa = interpretor.machine->dfa_subexpression[index];
				auto current_input_index = input_index;
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), input, current_input_index, end);
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
				auto&& find_result = interpretor.NFAMatch(subexpression, input, current_input_index, end);
				if(find_result.success == true)
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
		} });
		actions.insert({ Edge::EdgeType::PositiveLookbehind, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			wstring temp_input = input.substr(start, input_index);
			reverse(temp_input.begin(), temp_input.end());
			auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
			{
				auto subdfa = interpretor.machine->dfa_subexpression[index];
				auto current_input_index = 0;
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), temp_input, current_input_index, end);
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
				auto current_input_index = 0;
				auto&& find_result = interpretor.NFAMatch(subexpression, temp_input, current_input_index, end);
				if(find_result.success == true)
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
		} });
		actions.insert({ Edge::EdgeType::NegativeLookbehind, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			wstring temp_input = input.substr(start, input_index);
			reverse(temp_input.begin(), temp_input.end());
			auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
			{
				auto subdfa = interpretor.machine->dfa_subexpression[index];
				auto current_input_index = 0;
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), temp_input, current_input_index, end);
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
				auto current_input_index = 0;
				auto&& find_result = interpretor.NFAMatch(subexpression, temp_input, current_input_index, end);
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
		} });
		actions.insert({ Edge::EdgeType::NegativeLookahead, [](const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)
		{
			auto index = any_cast<int>(save_stack.back().states->output[save_stack.back().edge_index]->userdata);
			if(interpretor.machine->dfa_subexpression.find(index) != interpretor.machine->dfa_subexpression.end())
			{
				auto subdfa = interpretor.machine->dfa_subexpression[index];
				auto current_input_index = input_index;
				auto&& find_result = interpretor.DFAMatch(subdfa, save_stack.back(), input, current_input_index, end);
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
				auto&& find_result = interpretor.NFAMatch(subexpression, input, current_input_index, end);
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
		} });
		return move(actions);
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
	int RegexInterpretor::GetWCharIndex(const wchar_t character)const
	{
		return (*machine->table->char_table)[character];
	}
	bool RegexInterpretor::DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, const int start, const int end)
	{
		auto& table = dfa.dfa;
		auto& finalset = dfa.finalset;
		auto final_index = machine->table->range_table->size();
		//从0开始
		auto current_state_index = 0;
		auto current_input_index = start;
		save_state.length = 0;
		save_state.input_index = start;
		//追踪含final边的DFA 第一个参数是DFA状态号,第二个参数是当时的input_index
		//vector<pair<int, int>> tracks;
		while(current_state_index != finalset)
		{
			auto index = GetWCharIndex(input[current_input_index]);
			if(table[current_state_index][index] != -1)
			{
				//如果index到达end就匹配失败
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
		//有可能DFA会不捕获字符, a* 匹配 bbbaaa因为a*可以匹配空串,所以bbbaaa会匹配成功
		if(save_state.length == 0)
		{
			return false;
		}
		return true;
	}



	//根据是否是第一次进入,设置状态
	void SetState(bool is_new_state, SaveState& save, int& current_edge_index, int& current_input_index, State*& current_state)
	{
		if(is_new_state == true)
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
	}

	//边都排序一次,final放最后,不移动start,只匹配一次.
	RegexMatchResult RegexInterpretor::NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, const int start, const int end)
	{
		RegexMatchResult result;
		bool is_new_state = true;
		//	const auto& finalset = nfa.second;
		auto current_state = nfa.first;
		auto current_input_index = start;
		auto current_edge_index = 0;
		//设置进入时栈深度
		vector<SaveState> state_stack;
		state_stack.emplace_back(SaveState());
		result.anonymity_group.resize(machine->anonymity_captures.size());
		while(true)
		{
			if(current_state == nfa.second)
			{
				//assert(current_state == nfa.second);

				state_stack.pop_back();
				//assert(!state_stack.empty());
				auto sumlength = 0;
				for(auto&& element : state_stack)
				{
					sumlength += element.length;
				}
				if(sumlength != 0)
				{
					result.length = sumlength;
					result.start = state_stack.front().input_index;
					result.matched = input.substr(result.start, result.length);
					result.success = true;
					return result;
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
			SetState(is_new_state, save, current_edge_index, current_input_index, current_state);

			for(; current_edge_index < current_state->output.size(); current_edge_index++)
			{
				//遇到final,final如果不是最后把final放到最后去
				//PutFinalInListEnd(current_state, current_edge_index);

				save.edge_index = current_edge_index;

				auto&& match_result = actions[current_state->output[current_edge_index]->type](input, start, end, current_input_index, *this, state_stack, result);
				if(match_result == true)
				{
					current_state = current_state->output[current_edge_index]->target;
					//	state_stack.push_back(save);
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

	RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, const int start, const int end)
	{
		RegexMatchResult result;
		auto current_input_index = start;
		if(machine->dfa_expression != nullptr)
		{
			while(result.success == false && current_input_index < end)
			{
				SaveState save;
				if(DFAMatch(*machine->dfa_expression, save, input, current_input_index, end))
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
			while(result.success == false && current_input_index < end)
			{
				result = NFAMatch(*machine->nfa_expression, input, current_input_index, end);
				current_input_index++;
			}
		}
		return result;
	}
	//在start到end范围内寻找正则的全部匹配
	vector<RegexMatchResult> RegexInterpretor::RegexMatchAll(const wstring& input, const int start, const int end)
	{
		vector<RegexMatchResult> result;
		auto next_start_index = start;
		while(next_start_index < input.size())
		{
			auto match_result = RegexMatchOne(input, next_start_index, end);
			if(match_result.success == true)
			{
				next_start_index = result.back().start + result.back().length;
				result.emplace_back(move(match_result));
			}
			else
			{
				next_start_index += 1;
			}
		}
		return result;
	}

	RegexMatchResult RegexInterpretor::Match(const wstring& input, const int start )
	{
		if (find(optional->begin(),optional->end(),RegexControl::Multiline)!= optional->end())
		{
			wstring reverse_input;
			std::reverse_copy(input.cbegin(), input.cend(), inserter(reverse_input,reverse_input.begin()));
			return RegexMatchOne(reverse_input, start, reverse_input.size());

		}
		else
		{
			return RegexMatchOne(input, start, input.size());
		}
	}
	//从指定的起始位置开始，判断输入字符串中是否存在正则表达式的第一个匹配项
	bool RegexInterpretor::IsMatch(const wstring& input, const int start )
	{
		if(find(optional->begin(), optional->end(), RegexControl::Multiline) != optional->end())
		{
			wstring reverse_input;
			std::reverse_copy(input.cbegin(), input.cend(), inserter(reverse_input, reverse_input.begin()));
			return RegexMatchOne(reverse_input, start, reverse_input.size()).success;

		}
		else
		{
			return RegexMatchOne(input, start, input.size()).success;
		}
	}
	//从字符串中的指定起始位置开始，在指定的输入字符串中搜索正则表达式的所有匹配项。
	const vector<RegexMatchResult> RegexInterpretor::Matches(const wstring& input, int start )
	{
		if(find(optional->begin(), optional->end(), RegexControl::Multiline) != optional->end())
		{
			wstring reverse_input;
			std::reverse_copy(input.cbegin(), input.cend(), inserter(reverse_input, reverse_input.begin()));
			return RegexMatchAll(reverse_input, start, reverse_input.size());

		}
		else
		{
			return RegexMatchAll(input, start, input.size());
		}
	}
	wstring RegexInterpretor::Replace(const wstring& input, const wstring& repalce, int start)
	{
		auto target = Matches(input, start);
		vector<int> positions;
		//计算串总长度
		positions.emplace_back(0);
		auto sum = 0;
		auto count = 0;
		for (auto&& element : target)
		{
			count++;
			sum += element.length;
			positions.push_back(element.start);
			positions.push_back(element.start + element.length);
		}
		sum = input.size() - sum + count*repalce.size();
		wstring result;
		result.reserve(sum);
		auto des = result.begin();
		for(auto i = 0; i < positions.size();i+=2)
		{
			auto left =input.begin() + positions[i];
			auto right =input.begin()+ positions[i + 1];
			if (left !=right)
			{
				des = copy(left, right, des);
				des = copy(repalce.begin(), repalce.end(), des);
			}
			
		}
		return result;
	}

}
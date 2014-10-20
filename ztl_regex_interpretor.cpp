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
		anonymity_capture_value.resize(machine->subexpression->size());
	}
	bool BackReferenceAction(const wstring& input, int& input_index, SaveState& save, const wstring& expect_value)
	{
		auto length = expect_value.size();
		auto&& real_value = input.substr(input_index, length);
		if(expect_value == real_value)
		{
			save.length = length;
			input_index += length;
			return true;
		}
		else
		{
			return false;
		}
	}
	template<typename dfa_container_type,typename dfa_container_key>
	bool UsingDFAMatch(bool reverse,const wstring& input, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save, dfa_container_type& container, dfa_container_key& key)
	{
		auto& subfa = (container)[key];
		SaveState dfasave;
		auto&& result = interpretor.DFAMatch(subfa, dfasave, input, input_index, end);
		if(result == reverse)
		{
			save.length = 0;
			return true;
		}
		else
		{
			save.length = 0;
			return false;
		}
	}
	template<typename dfa_container_type, typename dfa_container_key>
	bool UsingNFAMatch(bool reverse,const wstring& input, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save, dfa_container_type& container, dfa_container_key& key)
	{
		auto& subexpression = container[key];
		auto&& result = interpretor.NFAMatch(subexpression, input, input_index, end);
		if(result.success == reverse)
		{
			save.length = 0;
			return true;
		}
		else
		{
			save.length = 0;
			return false;
		}
		
	}
	template<typename dfa_container_type, typename dfa_container_key>
	bool LookAroundAction(bool reverse, const wstring& input, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save, dfa_container_type& container, dfa_container_key)
	{
		auto index = any_cast<dfa_container_key>(save.states->output[save.edge_index]->userdata);
		if(container.find(index) != container.end())
		{
			return UsingDFAMatch(reverse, input, end, input_index, interpretor, save, *interpretor.machine->dfa_subexpression, index);
		}
		else
		{
			return UsingNFAMatch(reverse, input, end, input_index, interpretor, save, *interpretor.machine->subexpression, index);
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
					Jump,//无条件转移边和final差不多,只是不会匹配时被放到最后
					JumpByTime,//通过测试Time次 any pair<int,int> index,number
					JumpByTest,//通过测试  any int index
					*/

		actions.insert({ Edge::EdgeType::Final, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			save.length = 0;
			return true;
		} });
		actions.insert({ Edge::EdgeType::Jump, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			save.length = 0;
			return true;
		} });
		actions.insert({ Edge::EdgeType::JumpByTest, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			
			auto index = any_cast<int>(save.states->output[save.edge_index]->userdata);
			if(interpretor.machine->dfa_subexpression->find(index) != interpretor.machine->dfa_subexpression->end())
			{
				auto subdfa = (*interpretor.machine->dfa_subexpression)[index];
				SaveState dfasave;
				auto&& result = interpretor.DFAMatch(subdfa, dfasave, input, input_index, end);
				if(result == true)
				{

					save.length = dfasave.length;
					input_index += save.length;
					return true;
				}
				else
				{
					save.length = 0;
					return false;
				}
			}
			else
			{
				auto& subexpression = (*interpretor.machine->subexpression)[index];
				auto&& result = interpretor.NFAMatch(subexpression, input, input_index, end);
				if(result.success == true)
				{
					//interpretor.anonymity_capture_value[index].content = result.matched;
					//interpretor.anonymity_capture_value[index].position = result.start;
					//interpretor.anonymity_capture_value[index].length = result.length;
					interpretor.anonymity_capture_value.insert(interpretor.anonymity_capture_value.end(), result.anonymity_group.begin(), result.anonymity_group.end());
					save.length = result.length;
					input_index += save.length;
					return true;
				}
				else
				{
					save.length = 0;
					return false;
				}
			}
		} });
		actions.insert({ Edge::EdgeType::JumpByTime, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{

			auto user_pair = any_cast<pair<int,int>>(save.states->output[save.edge_index]->userdata);
			auto index = user_pair.first;
			auto number = user_pair.second;
			if(interpretor.machine->dfa_subexpression->find(index) != interpretor.machine->dfa_subexpression->end())
			{
				auto subdfa = (*interpretor.machine->dfa_subexpression)[index];
				auto end_index = input_index;
				for(auto i = 0; i < number;i++)
				{
					SaveState dfasave;
					auto&& result = interpretor.DFAMatch(subdfa, dfasave, input, end_index, end);
					if(result == true)
					{
						end_index += dfasave.length;
					}
					else
					{
						dfasave.length = 0;
						return false;
					}
				}
				save.length = end_index - input_index;
				input_index = end_index;
				return true;
				
			}
			else
			{
				auto& subexpression = (*interpretor.machine->subexpression)[index];
				vector<GroupIterm> temp;
				auto sum_length = 0;
				auto end_index = input_index;
				for(auto i = 0; i < number;i++)
				{ 
					auto&& result = interpretor.NFAMatch(subexpression, input, input_index, end);
					if(result.success == true)
					{
						temp[index].content = result.matched;
						temp[index].position = result.start;
						temp[index].length = result.length;
						temp.insert(temp.end(), result.anonymity_group.begin(), result.anonymity_group.end());
						sum_length += result.length;
					}
					else
					{
						save.length = 0;
						return false;
					}
				}
				interpretor.anonymity_capture_value.insert(interpretor.anonymity_capture_value.end(), temp.begin(), temp.end());
				input_index += sum_length;
				save.length = sum_length;
				return true;
			}
		} });
		actions.insert({ Edge::EdgeType::Head, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			if(input_index == begin)
			{
				save.length = 0;
				return true;
			}
			else
			{
				return false;
			}
		} });
		actions.insert({ Edge::EdgeType::Tail, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			if(input_index == end)
			{
				save.length = 0;
				return true;
			}
			else
			{
				return false;
			}
		} });
		actions.insert({ Edge::EdgeType::Char, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			auto expect_index = any_cast<int>(save.states->output[save.edge_index]->userdata);
			auto char_index = (*interpretor.machine->table->char_table)[input[input_index]];
			if(expect_index == char_index)
			{
				save.length = 1;
				input_index += 1;
				return true;
			}
			else
			{
				return false;
			}
		} });
		actions.insert({ Edge::EdgeType::BackReference, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			auto&& name = any_cast<wstring>(save.states->output[save.edge_index]->userdata);
			auto& expect_value = interpretor.capture_value[name].content;
			return BackReferenceAction(input, input_index, save, expect_value);
		} });
		actions.insert({ Edge::EdgeType::AnonymityBackReference, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			auto index = any_cast<int>(save.states->output[save.edge_index]->userdata);
			auto& expect_value = interpretor.anonymity_capture_value[index].content;
			return BackReferenceAction(input, input_index, save, expect_value);
		} });
		actions.insert({ Edge::EdgeType::Capture, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			auto name = any_cast<wstring>(save.states->output[save.edge_index]->userdata);

			if(interpretor.machine->dfa_captures->find(name) != interpretor.machine->dfa_captures->end())
			{
				auto subdfa = (*interpretor.machine->dfa_captures)[name];
				SaveState dfasave;
				auto&& result = interpretor.DFAMatch(subdfa, dfasave, input, input_index, end);
				if(result == true)
				{
					interpretor.capture_value[name].content = input.substr(dfasave.input_index, dfasave.length);
					interpretor.capture_value[name].position = dfasave.input_index;
					interpretor.capture_value[name].length = dfasave.length;
					save.length = dfasave.length;
					input_index += dfasave.length;

					return true;
				}
				else
				{
					save.length = 0;
					return false;
				}
			}
			else
			{
				auto& subexpression = (*interpretor.machine->captures)[name];
				auto&& result = interpretor.NFAMatch(subexpression, input, input_index, end);
				if(result.success == true)
				{
					interpretor.capture_value[name].content = result.matched;
					interpretor.capture_value[name].position = result.start;
					interpretor.capture_value[name].length = result.length;
					interpretor.capture_value.insert(result.group.begin(), result.group.end());
					save.length = result.length;
					input_index += result.length;

					return true;
				}
				else
				{
					save.length = 0;
					return false;
				}
			}
		} });
		actions.insert({ Edge::EdgeType::AnonymityCapture, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			auto index = any_cast<int>(save.states->output[save.edge_index]->userdata);
			if(interpretor.machine->dfa_anonymity_captures->find(index) != interpretor.machine->dfa_anonymity_captures->end())
			{
				auto subdfa = (*interpretor.machine->dfa_anonymity_captures)[index];
				SaveState dfasave;
				auto&& result = interpretor.DFAMatch(subdfa, dfasave, input, input_index, end);
				if(result == true)
				{
					interpretor.anonymity_capture_value[index].content = input.substr(dfasave.input_index, dfasave.length);
					interpretor.anonymity_capture_value[index].position = dfasave.input_index;
					interpretor.anonymity_capture_value[index].length = dfasave.length;
					save.length = dfasave.length;
					input_index += save.length;
					return true;
				}
				else
				{
					save.length = 0;
					return false;
				}
			}
			else
			{
				auto& subexpression = (*interpretor.machine->anonymity_captures)[index];
				auto&& result = interpretor.NFAMatch(subexpression, input, input_index, end);
				if(result.success == true)
				{
					interpretor.anonymity_capture_value[index].content = result.matched;
					interpretor.anonymity_capture_value[index].position = result.start;
					interpretor.anonymity_capture_value[index].length = result.length;
					interpretor.anonymity_capture_value.insert(interpretor.anonymity_capture_value.end(), result.anonymity_group.begin(), result.anonymity_group.end());
					save.length = result.length;
					input_index += save.length;
					return true;
				}
				else
				{
					save.length = 0;
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

		

		
		actions.insert({ Edge::EdgeType::PositivetiveLookahead, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			return LookAroundAction(true, input, end, input_index, interpretor, save, *interpretor.machine->dfa_subexpression,int());
		} });
		actions.insert({ Edge::EdgeType::PositiveLookbehind, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			wstring temp = input.substr(begin,input_index);
			reverse(temp.begin(), temp.end());
			SaveState new_save;
			new_save.input_index = save.input_index;
			new_save.edge_index = save.edge_index;
			new_save.states = save.states;
			int new_input_index = 0;
			return LookAroundAction(true, temp, temp.size(), new_input_index, interpretor, new_save, *interpretor.machine->dfa_subexpression, int());
		} });
		actions.insert({ Edge::EdgeType::NegativeLookbehind, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			wstring temp = input.substr(begin, input_index);
			reverse(temp.begin(), temp.end());
			SaveState new_save;
			new_save.input_index = save.input_index;
			new_save.edge_index = save.edge_index;
			new_save.states = save.states;
			int new_input_index = 0;
			return LookAroundAction(false, temp, temp.size(), new_input_index, interpretor, new_save, *interpretor.machine->dfa_subexpression, int());
		} });
		actions.insert({ Edge::EdgeType::NegativeLookahead, [](const wstring& input, const int begin, const int end, int& input_index, RegexInterpretor& interpretor, SaveState& save)
		{
			return LookAroundAction(false, input, end, input_index, interpretor, save, *interpretor.machine->dfa_subexpression, int());
		} });
		return move(actions);
	}

	RegexMatchResult RegexInterpretor::MatchSucced(const wstring& input)
	{
		RegexMatchResult result;
		state_stack.pop_back();
		assert(!state_stack.empty());
		auto sumlength = 0;
		for(auto&& element : state_stack)
		{
			sumlength += element.length;
		}
		if (sumlength !=0 )
		{
			result.length = sumlength;
			result.start = state_stack.front().input_index;
			result.matched = input.substr(result.start, result.length);
			result.success = true;
			result.group = move(capture_value);
			result.anonymity_group = move(anonymity_capture_value);
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
		state_stack.clear();
		this->anonymity_capture_value.clear();
		this->capture_value.clear();
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
		while(current_state_index != finalset)
		{
			if(table[current_state_index][GetWCharIndex(input[current_input_index])] != -1)
			{
				//如果index到达end就匹配失败
				if(current_input_index + 1 > end)
				{
					save_state.length = -1;
					return false;
				}
				else
				{
					current_state_index = table[current_state_index][GetWCharIndex(input[current_input_index])];
					current_input_index++;
					save_state.length++;
				}
			}
			else if(table[current_state_index][final_index] != -1)
			{
				current_state_index = table[current_state_index][final_index];
			}
			else
			{
				save_state.length = -1;
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

	void PutFinalInListEnd(State*& current_state, const int& current_edge_index)
	{
		//遇到final,final如果不是最后把final放到最后去
		if(current_state->output[current_edge_index]->type == Edge::EdgeType::Final)
		{
			if(current_edge_index != current_state->output.size() - 1)
			{
				swap(current_state->output.back(), current_state->output[current_edge_index]);
			}
		}
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

		auto deep = state_stack.size();
		state_stack.emplace_back(SaveState());
		while(current_state != nfa.second)
		{
			auto& save = state_stack.back();
			//设置save和各个变量状态
			SetState(is_new_state, save, current_edge_index, current_input_index, current_state);

			for(; current_edge_index < current_state->output.size(); current_edge_index++)
			{
				//遇到final,final如果不是最后把final放到最后去
				PutFinalInListEnd(current_state, current_edge_index);

				save.edge_index = current_edge_index;

				auto&& match_result = actions[current_state->output[current_edge_index]->type](input, start, end, current_input_index, *this, save);
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

			if(state_stack.size() == deep)
			{
				//所有的都失败了.没有上一状态报告失败
				return MatchFailed();
			}
		LoopEnd:;
		}

		assert(current_state == nfa.second);
		
		return MatchSucced(input);
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
					result.start = current_input_index;
					result.length = save.length;
					result.matched = input.substr(current_input_index, save.length);
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
			//如果匹配成功长度是0 就后移一位重新匹配
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

		auto&& match_result = RegexMatchOne(input, next_start_index, end);
		while(match_result.success == true)
		{
			result.emplace_back(move(match_result));
			next_start_index = result.back().start + result.back().length;
			match_result = RegexMatchOne(input, next_start_index, end);
		}
		return result;
	}
}
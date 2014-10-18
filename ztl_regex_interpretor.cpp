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
		//	//����Ҫ������
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
	int RegexInterpretor::GetWCharIndex(const wchar_t character)const
	{
		return (*machine->table->char_table)[character];
	}
	bool RegexInterpretor::DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, const int start, const int end)
	{
		auto& table = dfa.dfa;
		auto& finalset = dfa.finalset;
		auto final_index = machine->table->range_table->size();
		//��0��ʼ
		auto current_state_index = 0;
		auto current_input_index = start;
		save_state.length = 0;
		save_state.input_index = start;
		while(current_state_index != finalset)
		{
			if(table[current_state_index][GetWCharIndex(input[current_input_index])] != -1)
			{
				//���index����end��ƥ��ʧ��
				if (current_input_index+1 > end)
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
		//�п���DFA�᲻�����ַ�, a* ƥ�� bbbaaa��Ϊa*����ƥ��մ�,����bbbaaa��ƥ��ɹ�
		if (save_state.length == 0)
		{
			return false;
		}
		return true;
	}

	bool MatchEdge(Edge*& edge, const wstring& input, int current_input_index,const int end, SaveState& save)
	{
		return {};
	}
	void PutFinalInListEnd(State*& current_state,const int& current_edge_index)
	{
		//����final,final�����������final�ŵ����ȥ
		if(current_state->output[current_edge_index]->type == Edge::EdgeType::Final)
		{
			if(current_edge_index != current_state->output.size() - 1)
			{
				swap(current_state->output.back(), current_state->output[current_edge_index]);
			}
		}

	}

	//�����Ƿ��ǵ�һ�ν���,����״̬
	void SetState(bool is_new_state, SaveState& save, int& current_edge_index, int& current_input_index, State*&current_state)
	{
		if(is_new_state == true)
		{//״̬�ǵ�һ�ν���
			save.input_index = current_input_index;
			save.states = current_state;
			save.edge_index = -1;
			save.length = -1;
			current_edge_index = 0;
		}
		else
		{//״̬���ǵ�һ�ν���
			current_edge_index = save.edge_index + 1;
			current_input_index = save.input_index;
			current_state = save.states;
			save.length = -1;
		}
	}

	//�߶�����һ��,final�����
	RegexMatchResult RegexInterpretor::NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, const int start, const int end)
	{
		RegexMatchResult result;
		bool is_new_state = true;
	//	const auto& finalset = nfa.second;
		auto current_state = nfa.first;
		auto current_input_index = start;
		auto current_edge_index = 0;
		state_stack.emplace_back(SaveState());
		while(current_state != nfa.second)
		{
			auto& save = state_stack.back();
			//����save�͸�������״̬
			SetState(is_new_state, save,current_edge_index, current_input_index, current_state);

			for(; current_edge_index < current_state->output.size(); current_edge_index++)
			{
				//����final,final�����������final�ŵ����ȥ
				PutFinalInListEnd(current_state, current_edge_index);

				save.edge_index = current_edge_index;
				auto&& match_result = MatchEdge(current_state->output[current_edge_index], input, current_input_index,end,save);
				if(match_result == true)
				{
					current_state = current_state->output[current_edge_index]->target;
					state_stack.emplace_back(move(save));
					state_stack.emplace_back(SaveState());
					is_new_state = true;
					goto LoopEnd;
				}
			}
			//���б߶�ƥ��ʧ��
			//����Ҫ������û��������final
			//����final��ʱ�����һ����.Ȼ��ֱ������while
			//������ǰ״̬,ִ����һ״̬����һ����
			state_stack.pop_back();
			is_new_state = false;

			if(state_stack.empty())
			{
				//���еĶ�ʧ����.û����һ״̬����ʧ��
				return MatchFailed();
			}
		LoopEnd:;
		}

		assert(current_state == nfa.second);
		return MatchSucced();
	}

	RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, const int start, const int end)
	{
		RegexMatchResult result;
		auto current_input_index = start;
		if (machine->dfa_expression != nullptr)
		{
			while(result.success == false && current_input_index < end)
			{
				SaveState save;
				if(DFAMatch(*machine->dfa_expression, save, input, current_input_index, end))
				{
					//�ɹ�ƥ��
					result.success = true;
					result.start = current_input_index;
					result.end = result.start + save.length;
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
			while(result.success == false&&current_input_index < end)
			{
				result = NFAMatch(*machine->nfa_expression, input, current_input_index,end);
				current_input_index++;
			}
		}
		return result;

	}
	//��start��end��Χ��Ѱ�������ȫ��ƥ��
	vector<RegexMatchResult> RegexInterpretor::RegexMatchAll(const wstring& input, const int start, const int end)
	{
		vector<RegexMatchResult> result;
		auto next_start_index = start;

		auto&& match_result = RegexMatchOne(input, next_start_index, end);
		while(match_result.success == true)
		{
			result.emplace_back(move(match_result));
			next_start_index = result.back().end;
			match_result = RegexMatchOne(input, next_start_index, end);
		}
		return result;
	}
}
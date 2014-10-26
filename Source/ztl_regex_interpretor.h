#pragma once
#include "forward.h"
#include "ztl_regex_automachine.h"

/*
���:
//�Ӹ�Ԥ�����׶�
//����EC Ԥ����pattern ���� (�����滻����(?:
//����IC ����һ���﷨��,��char���滻һ��
//MT $ ^ Ԥ����pattern �滻�ɶ�Ӧ���������
//RTL ���ҵ���ƥ���ı�
//ODFA �滻��������,��ʹ�ò���
//
enum class RegexControl
{
ExplicitCapture,//����������
IgnoreCase,//��Сд�����е�ƥ��
Multiline,// $^ ƥ���н�β�Ϳ�ͷ
RightToLeft,//
Singleline,//.ƥ�������ַ�
OnlyDFA,//�������������ǲ���,��ʹ����������,��ʹ�ø߼�����
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
		//��������ʽƥ��ɹ���õ���������
		unordered_map<wstring, GroupIterm>  group;
		vector<GroupIterm>					anonymity_group;

		//����ʽƥ����
		wstring								matched;
		//ƥ���ڴ���λ��
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
		//��ǰ״̬ͨ����ǰ�����ĵ��ַ�����
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
		//��ָ������ʼλ�ÿ�ʼ���������ַ����������������ʽ�ĵ�һ��ƥ������ҽ�����ָ���������ַ���
		RegexMatchResult 				Match(const wstring& input, const int start = 0);
		RegexMatchResult 				Match(const wstring& input, wstring::const_iterator start);
		//��ָ������ʼλ�ÿ�ʼ���ж������ַ������Ƿ�����������ʽ�ĵ�һ��ƥ����
		bool							IsMatch(const wstring& input, const int start = 0);
		bool							IsMatch(const wstring& input, wstring::const_iterator start);

		//���ַ����е�ָ����ʼλ�ÿ�ʼ����ָ���������ַ����������������ʽ������ƥ���
		const vector<RegexMatchResult> 	Matches(const wstring& input, int start = 0);
		const vector<RegexMatchResult> 	Matches(const wstring& input, wstring::const_iterator start);

		wstring							Replace(const wstring& input, const wstring& repalce, int start = 0);
		wstring							Replace(const wstring& input, const wstring& repalce, wstring::const_iterator start);
	private:
		//��start��end��Χ��Ѱ������ĵ�һ��ƥ��
		template<typename iterator_type>
		RegexMatchResult RegexMatchOne(const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);
		//��start��end��Χ��Ѱ�������ȫ��ƥ��
		template<typename iterator_type>
		vector<RegexMatchResult> RegexMatchAll(const wstring& input, iterator_type start, iterator_type end);
			//DFA ƥ��,��start��ʼ,���ƶ�start,���ܷ񵽴��ս�״̬
		//���������save_stack.back()��
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
					//�ɹ�ƥ��
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

	//��start��end��Χ��Ѱ�������ȫ��ƥ��
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
		//��0��ʼ
		auto current_state_index = 0;
		auto current_input_index = input_index;
		save_state.length = 0;
		save_state.input_index = input_index - start;
		//׷�ٺ�final�ߵ�DFA ��һ��������DFA״̬��,�ڶ��������ǵ�ʱ��input_index
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
		//�п���DFA�᲻�����ַ�, a* ƥ�� bbbaaa��Ϊa*����ƥ��մ�,����bbbaaa��ƥ��ɹ�

		return save_state.length != 0;
	}

	//NFA ƥ��,��start��ʼ,���ƶ�start,���ܷ񵽴��ս�״̬
	//�߶�����һ��,final�����,���ƶ�start,ֻƥ��һ��.
	template<typename iterator_type>
	RegexMatchResult RegexInterpretor::NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
	{
		RegexMatchResult result;
		bool is_new_state = true;
		//	const auto& finalset = nfa.second;
		auto current_state = nfa.first;
		auto current_input_index = input_index;

		auto current_edge_index = 0;
		//���ý���ʱջ���
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
			//����save�͸�������״̬
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
	}
}
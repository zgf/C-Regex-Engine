#pragma once
#include "forward.h"
#include "ztl_regex_automachine.h"

/*
���:
//�Ӹ�Ԥ����׶�
//����EC Ԥ����pattern ���� (�����滻����(?:
//����IC ����һ���﷨��,��char���滻һ��
//MT $ ^ Ԥ����pattern �滻�ɶ�Ӧ��������
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
		//�������ʽƥ��ɹ���õ���������
		unordered_map<wstring, GroupIterm>  group;
		vector<GroupIterm>					anonymity_group;

		//���ʽƥ����
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
		int	             length=0;
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
		//��ָ������ʼλ�ÿ�ʼ���������ַ���������������ʽ�ĵ�һ��ƥ������ҽ�����ָ���������ַ���
		RegexMatchResult 				Match(const wstring& input, const int start = 0);
		RegexMatchResult 				Match(const wstring& input, wstring::const_iterator start);
		//��ָ������ʼλ�ÿ�ʼ���ж������ַ������Ƿ����������ʽ�ĵ�һ��ƥ����
		bool							IsMatch(const wstring& input, const int start = 0);
		bool							IsMatch(const wstring& input, wstring::const_iterator start );

		//���ַ����е�ָ����ʼλ�ÿ�ʼ����ָ���������ַ���������������ʽ������ƥ���
		const vector<RegexMatchResult> 	Matches(const wstring& input, int start = 0);
		const vector<RegexMatchResult> 	Matches(const wstring& input, wstring::const_iterator start);

		wstring							Replace(const wstring& input, const wstring& repalce, int start=0);
		wstring							Replace(const wstring& input, const wstring& repalce, wstring::const_iterator start);
	private:
		//��start��end��Χ��Ѱ������ĵ�һ��ƥ��
		RegexMatchResult RegexInterpretor::RegexMatchOne(const wstring& input, wstring::const_iterator input_index, wstring::const_iterator start, wstring::const_iterator end);
			//��start��end��Χ��Ѱ�������ȫ��ƥ��
		vector<RegexMatchResult> RegexMatchAll(const wstring& input, wstring::const_iterator start, wstring::const_iterator end);
		//DFA ƥ��,��start��ʼ,���ƶ�start,���ܷ񵽴��ս�״̬
		//���������save_stack.back()��
		RegexMatchResult MatchSucced(const wstring& input, vector<SaveState>& save_stack, RegexMatchResult& result);
		RegexMatchResult MatchFailed();
		int GetWCharIndex(const wchar_t character)const;
	public:
		template<typename iterator_type>
		bool DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
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
		RegexMatchResult NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end)
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
					if(sumlength != 0)
					{
						//�ӵ�һ������>0��ʼ
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
				SetState(is_new_state, save, current_edge_index, start,current_input_index, current_state);

				for(; current_edge_index < current_state->output.size(); current_edge_index++)
				{
					save.edge_index = current_edge_index;
					bool match_result = true;
					//auto&& match_result = actions[current_state->output[current_edge_index]->type](input, start, end, current_input_index, *this, state_stack, result);
					switch(current_state->output[current_edge_index]->type)
					{
						case Edge::EdgeType::Final:
							match_result = Final(input, start, end, current_input_index, *this, state_stack, result);
							break;
						case Edge::EdgeType::Head:
							match_result = Head(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::Tail:
							match_result = Tail(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::Char:
							match_result = Char(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::Capture:
							match_result = Capture(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::AnonymityCapture:
							match_result = AnonymityCapture(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::BackReference:
							match_result = BackReference(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::AnonymityBackReference:
							match_result = AnonymityBackReference(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::PositiveLookbehind:
							match_result = PositiveLookbehind(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::PositivetiveLookahead:
							match_result = PositivetiveLookahead(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::NegativeLookbehind:
							match_result = NegativeLookbehind(input, start, end, current_input_index, *this, state_stack, result);

							break;
						case Edge::EdgeType::NegativeLookahead:
							match_result = NegativeLookahead(input, start, end, current_input_index, *this, state_stack, result);

							break;
						default:
							assert(false);
							break;
					}


					if(match_result == true)
					{
						current_state = current_state->output[current_edge_index]->target;
						//	state_stack.push_back(save);
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

		
	};
}
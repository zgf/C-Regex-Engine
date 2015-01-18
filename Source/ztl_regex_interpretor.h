#pragma once

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
	class State;
	enum class RegexControl:int;
	class AutoMachine;
	class DFA;
	class CharRange;
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
	public:
		int Length() const
		{
			return length;
		}
		wstring Content() const
		{
			return matched;
		}
		pair<int,int> Position()const
		{
			return { start, length };
		}
		bool Success() const
		{
			return success;
		}

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
		shared_ptr<vector<RegexControl>>   optional;
		shared_ptr<AutoMachine>			machine;
	public:
		RegexInterpretor() = delete;
		RegexInterpretor(const wstring& pattern, const shared_ptr<vector<RegexControl>>_optional);
		RegexInterpretor(const wstring& pattern);

	public:
		//��ָ������ʼλ�ÿ�ʼ���������ַ���������������ʽ�ĵ�һ��ƥ������ҽ�����ָ���������ַ���
		RegexMatchResult 				Match(const wstring& input, const int start = 0);
		RegexMatchResult 				Match(const wstring& input, wstring::const_iterator start);
		//��ָ������ʼλ�ÿ�ʼ���ж������ַ������Ƿ����������ʽ�ĵ�һ��ƥ����
		bool							IsMatch(const wstring& input, const int start = 0);
		bool							IsMatch(const wstring& input, wstring::const_iterator start);

		//���ַ����е�ָ����ʼλ�ÿ�ʼ����ָ���������ַ���������������ʽ������ƥ���
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
		RegexMatchResult NFAMatch(const pair<State*,State*>& nfa, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);

		template<typename iterator_type>
		bool DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, iterator_type input_index, iterator_type start, iterator_type end);
	};
	wstring LinearStringToRegexString(const wstring& literal);
}
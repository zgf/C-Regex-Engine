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
		int	             length;
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
		using ActionType = unordered_map < Edge::EdgeType, function<bool(const wstring& input, const int start, const int end, int& input_index, RegexInterpretor& interpretor, vector<SaveState>& save_stack, RegexMatchResult& result)> > ;
	private:
		static ActionType actions;
	public:
		RegexInterpretor() = delete;
		RegexInterpretor(const wstring& pattern, const Ptr<vector<RegexControl>>_optional);
		RegexInterpretor(const wstring& pattern);
	public:
		static  ActionType InitActions();
	public:
		//��ָ������ʼλ�ÿ�ʼ���������ַ���������������ʽ�ĵ�һ��ƥ������ҽ�����ָ���������ַ���
		RegexMatchResult 				Match(const wstring& input, const int start = 0);
		//��ָ������ʼλ�ÿ�ʼ���ж������ַ������Ƿ����������ʽ�ĵ�һ��ƥ����
		bool							IsMatch(const wstring& input, const int start = 0);
		//���ַ����е�ָ����ʼλ�ÿ�ʼ����ָ���������ַ���������������ʽ������ƥ���
		const vector<RegexMatchResult> 	Matches(const wstring& input, int start = 0);
		wstring							Replace(const wstring& input, const wstring& repalce, int start=0);
	private:
		//��start��end��Χ��Ѱ������ĵ�һ��ƥ��
		RegexMatchResult RegexMatchOne(const wstring& input, const int start, const int end);
		//��start��end��Χ��Ѱ�������ȫ��ƥ��
		vector<RegexMatchResult> RegexMatchAll(const wstring& input, const int start, const int end);
		static bool BackReferenceAction(const wstring& input, int& input_index, SaveState& save, const wstring& expect_value);
		//DFA ƥ��,��start��ʼ,���ƶ�start,���ܷ񵽴��ս�״̬
		//���������save_stack.back()��
		bool DFAMatch(const DFA& dfa, SaveState& save_state, const wstring& input, const int start, const int end);
		//NFA ƥ��,��start��ʼ,���ƶ�start,���ܷ񵽴��ս�״̬
		RegexMatchResult NFAMatch(const AutoMachine::StatesType& nfa, const wstring& input, const int start, const int end);

		RegexMatchResult MatchSucced(const wstring& input, vector<SaveState>& save_stack, RegexMatchResult& result);
		RegexMatchResult MatchFailed();
		int GetWCharIndex(const wchar_t character)const;
	public:
	};
}
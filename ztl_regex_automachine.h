#pragma once
#include "forward.h"
#include "ztl_regex_data.h"
#include "ztl_regex_expression.h"

namespace ztl
{
	//�Զ���
	//����ǿ�NFA.
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		Ptr<Expression>							ast;
		unordered_map<wstring, StatesType> captures;
		unordered_map<wstring, DFA>		dfa_captures;
		unordered_map<int, DFA>			dfa_anonymity_captures;

		vector<StatesType>			subexpression;//���ڼ���lookaround��
		unordered_map<int, DFA>			dfa_subexpression;

		Ptr<CharTable>							table;
		vector<Ptr<State>>					states;
		vector<Ptr<Edge>>					edges;
		unordered_map<wstring, StatesType>		macro_expression;//����ʽ
		int										capture_count = 0;//���������
		Ptr<AutoMachine::StatesType>			nfa_expression;
		Ptr<DFA>								dfa_expression;
		vector<pair<AutoMachine::StatesType, int>> anonymity_captures;//������������
		//������״̬���ӱ��ʽ������DFA�Ż�
		vector<State*> non_dfa;
	public:
		AutoMachine() = delete;
		AutoMachine(RegexParser& parser);
	public:

		AutoMachine::StatesType NewEpsilonStates();
		AutoMachine::StatesType NewCharStates(const CharRange& range);
		AutoMachine::StatesType NewCharSetStates(const bool reverse, const vector<CharRange>& range);
		AutoMachine::StatesType NewAlterStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewBeinAndEndStates(const Edge::EdgeType& type);
		AutoMachine::StatesType NewCaptureStates(StatesType& substates, const wstring& name, const int index);
		AutoMachine::StatesType NewBackReferenceStates(const wstring& name);
		AutoMachine::StatesType NewAnonymityBackReferenceBackReferenceStates(const int& index);
		AutoMachine::StatesType NewRegexMacroStates(const wstring& name, StatesType& substates);
		AutoMachine::StatesType NewMacroReferenceStates(const wstring& name);
		AutoMachine::StatesType NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type);
		AutoMachine::StatesType NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end);
		AutoMachine::StatesType NewFinalStates(StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewChooseClourseStates(bool greedy, StatesType& target);
		AutoMachine::StatesType NewPositiveClourseStates(bool greedy, StatesType& target);
		AutoMachine::StatesType NewKleenClourseStates(bool greedy, StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& target, int number);

		vector<AutoMachine::StatesType> NewStateSequence(StatesType& target, int number);

		AutoMachine::StatesType AutoMachine::ConnectLoopChain(bool greedy, AutoMachine::StatesType& loop_head, int number);
		AutoMachine::StatesType AutoMachine::LoopIncludeInFinite(bool greedy, int number, AutoMachine::StatesType& substates);
		//����ͬ��ͼ
		AutoMachine::StatesType NewIsomorphicGraph(StatesType& target);

		//���Final���Ҵ���DFA
		void AddFinalAndCreatDFA(AutoMachine::StatesType& subexpress);
		//�����ɵ���Target�ڵ�Ľڵ㼯��
		unordered_set<State*> FindReachTargetStateSet(State* start, State* target);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		void AutoMachine::BuildOptimizeNFA();
		void SetAnonymityCaptures();
		AutoMachine::StatesType EpsilonNFAtoNFA(const AutoMachine::StatesType& target);
		void AutoMachine::CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, vector< unordered_set<State*>>& edge_nfa_map, const int final_index);
		
	private:
		void GetTableIndex(const CharRange& target, vector<int>& range)const;
		int GetTableIndex(const int& target)const;
		AutoMachine::StatesType NewStates();
		State* NewOneState();

		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	public:
		//��鵱ǰ��ͼ�ǲ���pure����� Ҳ����ֻ�� char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);

		//NFA�ڵ�ͨ��nullת���ɴ�Ľڵ㼯��
		unordered_set<State*> EpsilonNFASet(State* target);

		unordered_set<State*> EpsilonNFASet(unordered_set<State*>& target);
		//NFA to DFA
		DFA AutoMachine::NfaToDfa(AutoMachine::StatesType& expression);

		//����target������
		int Find(const State*& target);
		//�Ż��ӱ��ʽ
		void OptimizeSubexpress();
	private:
		Edge::EdgeType GetEdgeType(int index) const;
	};

	//�����Զ���
}
namespace std
{
	template<>
	struct hash < ztl::Edge >
	{	// hash functor for Edge
	public:
		size_t operator()(const ztl::Edge& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			return (std::_Hash_seq((const unsigned char *)&_Keyval, sizeof(ztl::Edge)));
		}
	};
	template<>
	struct hash < unordered_set<ztl::State*> >
	{	// hash functor for Edge
	public:
		//���hash�������д��- -...Ŀ��Ч�ʲ���̫��- -....
		size_t operator()(const unordered_set<ztl::State*>& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			vector<size_t> result(_Keyval.size());
			size_t i = 0;
			for(auto&& iter : _Keyval)
			{
				result[i++] = _Hash_seq((const unsigned char *)(iter), sizeof(ztl::State*));
			}
			return	accumulate(result.begin(), result.end(), 0);
		}
	};
}
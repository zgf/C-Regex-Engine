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
	public:/*�������*/
		using StatesType = pair < State*, State* >;
		unordered_map<wstring, StatesType>				captures;
		vector<pair<AutoMachine::StatesType, int>>		anonymity_captures;//����������
		vector<StatesType>								subexpression;//���ڼ���lookaround��
		unordered_map<wstring, StatesType>				macro_expression;//����ʽ

	
		
		Ptr<Expression>									ast;
		Ptr<CharTable>									table;
		vector<Ptr<State>>								states;
		vector<Ptr<Edge>>								edges;
		Ptr<AutoMachine::StatesType>					nfa_expression;
		int												capture_count = 0;//���������
	public:/*�Ż����*/
		unordered_map<wstring, DFA>						dfa_captures;
		unordered_map<int, DFA>							dfa_anonymity_captures;
		unordered_map<int, DFA>							dfa_subexpression;
		Ptr<DFA>										dfa_expression;
		//������״̬���ӱ��ʽ������DFA�Ż�
		vector<State*>									non_dfa;
	public:
		AutoMachine() = delete;
		AutoMachine(RegexParser& parser);
	public: /*��AST�����й�API*/

		AutoMachine::StatesType NewEpsilonStates();
		AutoMachine::StatesType NewNormalCharStates(const CharRange& range);
		AutoMachine::StatesType NewCharSetStates(const bool reverse, const vector<CharRange>& range);
		AutoMachine::StatesType NewAlterStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewBeinAndEndStates(const Edge::EdgeType& type);
		AutoMachine::StatesType NewCaptureStates(StatesType& substates, const wstring& name);
		AutoMachine::StatesType NewAnonymitCaptureStates(StatesType& substates, const int index);
		AutoMachine::StatesType NewBackReferenceStates(const wstring& name);
		AutoMachine::StatesType NewAnonymityBackReferenceBackReferenceStates(const int& index);
		AutoMachine::StatesType NewRegexMacroStates(const wstring& name, StatesType& substates);
		AutoMachine::StatesType NewMacroReferenceStates(const wstring& name);
		AutoMachine::StatesType NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type);
		AutoMachine::StatesType NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end);
		AutoMachine::StatesType NewFinalStates(StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewSequenceStates(StatesType& target, int number);
	public:/*�Ż����*/
		void							BuildOptimizeNFA();
	private: /*��AST�����й�API*/
		
		//����ͬ��ͼ
		AutoMachine::StatesType			NewIsomorphicGraph(StatesType& target);

		AutoMachine::StatesType			NewStates();
		Edge*							NewEdge();
		State*							NewOneState();
		vector<AutoMachine::StatesType> CreateTheSameStateSequence(StatesType& target, int number);
		void							ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void							ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		void							ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void							ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);


	private: /*��NFA�Ż����*/
		AutoMachine::StatesType			EpsilonNFAtoNFA(const AutoMachine::StatesType& target);
		int								GetSubexpressionIndex(const StatesType& substates);
		void							SortAnonymityCaptures();
		vector<Edge*>					GetSortedReachNoneEpsilonEdge(State* target, unordered_map<State*, State*>& state_map, State* front, unordered_set<State*>& sign);
		vector<Edge*>					GetSortedReachNoneEpsilonEdge(State* target, unordered_map<State*, State*>& state_map, State* front);
		void							CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, vector< unordered_set<State*>>& edge_nfa_map, const int final_index);
		State*							NewReverseGraph(State* target);

		//��ȡĿ�귶Χ�ڱ��ϵ�������Χ
		void							GetRangeTableIndex(const CharRange& target, vector<int>& range_table)const;
		
		//��鵱ǰ��ͼ�ǲ���pure����� Ҳ����ֻ�� char e,
		bool							CheckPure(const AutoMachine::StatesType& expression);
		void							ClearEdgeNfaMap(vector<int>& need_clear, vector<unordered_set<State*>>& edge_nfa_map);
		//NFA to DFA
		DFA								NfaToDfa(AutoMachine::StatesType& expression);
		//�Ż��ӱ��ʽ
		void							OptimizeSubexpress();
		bool							IsLookBehindEdge(Edge* target);
		bool							IsLookAroundEge(Edge* target);
		//��lookbehind�������ӱ��ʽȫ����תͼ
		void							ReverseLookBehindGraph(AutoMachine::StatesType& target);
		AutoMachine::StatesType&		HaveSubGraph(State* target,const int index);
		bool							NoneSubGraph(State* target, const int index);
		void							AllNFAAddTheFinal();
		void							AllEpsilonNFAtoNFA();
		void							AllChangeNFAToDFA();
		void							AllFinalMoveToEnd();
		void							ChangeLookAroundDirect(Edge* target);
	};

	//�����Զ���
}
namespace std
{
	
	template<>
	struct hash < unordered_set<ztl::State*> >
	{	// hash functor for State* set
	public:
		//���hash�������д��- -...Ŀ��Ч�ʲ���̫��- -....
		size_t operator()(const unordered_set<ztl::State*>& _Keyval) const
		{	
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
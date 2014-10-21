#pragma once
#include "forward.h"
#include "ztl_regex_data.h"
#include "ztl_regex_expression.h"

namespace ztl
{
	

	//自动机
	//构造非空NFA.
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		Ptr<Expression>							ast;
		Ptr<unordered_map<wstring, StatesType>> captures;
		Ptr<unordered_map<wstring, DFA>>		dfa_captures;
		Ptr<vector<StatesType>>					anonymity_captures;//匿名捕获组
		Ptr<unordered_map<int, DFA>>			dfa_anonymity_captures;

		Ptr<vector<StatesType>>					subexpression;//用在几个lookaround上
		Ptr<unordered_map<int, DFA>>			dfa_subexpression;

		Ptr<CharTable>							table;
		Ptr<vector<Ptr<State>>>					states;
		Ptr<vector<Ptr<Edge>>>					edges;
		unordered_map<wstring, StatesType>		macro_expression;//宏表达式
		int										capture_count = 0;//捕获组计数
		Ptr<AutoMachine::StatesType>			nfa_expression;
		Ptr<DFA>								dfa_expression;
	public:
		AutoMachine() = delete;
		AutoMachine(RegexParser& parser);
	public:

		AutoMachine::StatesType NewEpsilonStates();
		AutoMachine::StatesType NewCharStates(const CharRange& range);
		AutoMachine::StatesType NewCharSetStates(const bool reverse, const vector<CharRange>& range);
		AutoMachine::StatesType NewAlterStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewBeinAndEndStates(const Edge::EdgeType& type);
		AutoMachine::StatesType NewCaptureStates(StatesType& substates, const wstring& name);
		AutoMachine::StatesType NewBackReferenceStates(const wstring& name);
		AutoMachine::StatesType NewAnonymityBackReferenceBackReferenceStates(const int& index);
		AutoMachine::StatesType NewRegexMacroStates(const wstring& name, StatesType& substates);
		AutoMachine::StatesType NewMacroReferenceStates(const wstring& name);
		AutoMachine::StatesType NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type);
		AutoMachine::StatesType NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end);
		AutoMachine::StatesType NewFinalStates(StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewChooseClourseStates(bool greedy,StatesType& target);
		AutoMachine::StatesType NewPositiveClourseStates(bool greedy, StatesType& target);
		AutoMachine::StatesType NewKleenClourseStates(bool greedy, StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& target, int number);

		vector<AutoMachine::StatesType> NewStateSequence(StatesType& target, int number);

		AutoMachine::StatesType AutoMachine::ConnectLoopChain(bool greedy, AutoMachine::StatesType& loop_head, int number);
		AutoMachine::StatesType AutoMachine::LoopIncludeInFinite(bool greedy, int number,AutoMachine::StatesType& substates);
		//创建同构图
		AutoMachine::StatesType NewIsomorphicGraph(StatesType& target);

		//添加Final并且创建DFA
		void AddFinalAndCreatDFA(AutoMachine::StatesType& subexpress);
		//搜索可到达Target节点的节点集合
		unordered_set<State*> FindReachTargetStateSet(State* start,State* target);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		void AutoMachine::BuildOptimizeNFA();
		AutoMachine::StatesType EpsilonNFAtoNFA(const AutoMachine::StatesType& target);
		void AutoMachine::CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, vector< unordered_set<State*>>& edge_nfa_map, const int final_index);
		void CreatDFAStateByEdgeToNFAMap(unordered_map<int, unordered_set<State*>>& edge_nfa_map, unordered_map<unordered_set<State*>, int>& nfa_dfa_map, int& front, vector<vector<int>>& dfa_table, vector<unordered_set<State*>>& dfa_nfa_map, const int edge_sum, deque<int>& dfaqueue);
	private:
		void GetTableIndex(const CharRange& target,vector<int>& range)const;
		int GetTableIndex(const int& target)const;
		AutoMachine::StatesType NewStates();
		State* NewOneState();

		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	public:
		//检查当前子图是不是pure正则的 也就是只有 char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);

		//NFA节点通过null转换可达的节点集合
		unordered_set<State*> EpsilonNFASet(State* target);

		unordered_set<State*> EpsilonNFASet(unordered_set<State*>& target);
		//NFA to DFA
		DFA AutoMachine::NfaToDfa(AutoMachine::StatesType& expression);

		//查找target的索引
		int Find(const State*& target);
		//优化子表达式
		void OptimizeSubexpress();
	private:
		Edge::EdgeType GetEdgeType(int index) const;
	};

	//正则自动机
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
		//这个hash函数随便写的- -...目测效率不是太高- -....
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
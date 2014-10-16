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
		Ptr<Expression> ast;
		Ptr<unordered_map<wstring, StatesType>> captures;
		Ptr<vector<StatesType>>					anonymity_captures;//����������
		Ptr<vector<StatesType>>					subexpression;//���ڼ���lookaround��
		Ptr<vector<CharRange>> table;
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
		unordered_map<wstring, StatesType> macro_expression;//����ʽ
		int capture_count = 0;//���������
	public:
		AutoMachine() = delete;
		AutoMachine(const Ptr<vector<CharRange>>& _table)
			:table(_table), states(make_shared<vector<Ptr<State>>>()), edges(make_shared<vector<Ptr<Edge>>>()), captures(make_shared<unordered_map<wstring, StatesType>>()), subexpression(make_shared<vector<StatesType>>()), macro_expression()
		{
		}
		AutoMachine(RegexParser& parser)
			:table(nullptr), states(make_shared<vector<Ptr<State>>>()), edges(make_shared<vector<Ptr<Edge>>>()), captures(make_shared<unordered_map<wstring, StatesType>>()), subexpression(make_shared<vector<StatesType>>()), ast(nullptr), macro_expression()
		{
			table = parser.GetCharTable();
			ast = parser.GetExpressTree();
		}
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

		//����ͬ��ͼ
		AutoMachine::StatesType NewIsomorphicGraph(StatesType& target);


		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		AutoMachine::StatesType BuildOptimizeNFA();
	private:
		int GetTableIndex(const CharRange& target)const;
		AutoMachine::StatesType NewStates();
		State* NewOneState();

		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	public:
		//��鵱ǰ��ͼ�ǲ���pure����� Ҳ����ֻ�� char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);
		//NFA to DFA
		void NfaToDfa(AutoMachine::StatesType& expression);
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
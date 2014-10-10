#pragma once
#include "forward.h"
#include "ztl_regex_expression.h"
namespace ztl
{
	class State;
	class Edge
	{
	public:
		//边
		enum class EdgeType
		{
			Epsilon,
			Capture,
			BackReference,
			Loop,
			Char,
			Head,
			Tail,
			PositivetiveLookahead,
			NegativeLookahead,
			PositiveLookbehind,
			NegativeLookbehind,
			Final, //边后面是终结状态

		};
		struct LoopUserData
		{
			int index;
			int begin;
			int end;
			bool greedy;
		public:
			LoopUserData() = default;
			LoopUserData(const int& _index, const int& _begin, const int& _end, const bool _greedy) :
				index(_index), begin(_begin), end(_end), greedy(_greedy)
			{

			}
		};
	public:
		EdgeType type;
		State* srouce;
		State* target;
		any userdata;
		/*
		char
		这里是table index. int
		Capture
		这里是subexpression 编号和bool 是否是pure subexpress pair<int,bool>
		Loop
		这里是Loop subexpress 编号 是否是pure. pair<int,bool>
		Head userdata不需要
		Tail也不需要.
		Final也不需要
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		都是 subexpression index
		BackReference
		这里是wstring name
		NameSubexprssion
		这里是name.
		*/
	};

	//状态
	class State
	{
	public:
		vector<Edge*> input;
		vector<Edge*> output;
	};
	
	

	//自动机
	//构造非空NFA.
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		Ptr<unordered_map<wstring, StatesType>> captures;
		Ptr<vector<StatesType>>				 subexpression;//用在几个lookaround上
		Ptr<vector<CharRange>> table;
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
	public:
		AutoMachine() = delete;
		AutoMachine(const Ptr<vector<CharRange>>& _table) 
			:table(_table), states(make_shared<vector<Ptr<State>>>()), edges(make_shared<vector<Ptr<Edge>>>()), captures(make_shared<unordered_map<wstring, StatesType>>()), subexpression(make_shared<vector<StatesType>>())
		{

		}
	public:


		AutoMachine::StatesType NewEpsilonStates();
		AutoMachine::StatesType NewCharStates(const CharRange& range);
		AutoMachine::StatesType NewCharSetStates(const bool reverse,const vector<CharRange>& range);
		AutoMachine::StatesType NewAlterStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewBeinAndEndStates(const Edge::EdgeType& type);
		AutoMachine::StatesType NewCaptureStates(StatesType& substates,const wstring& name);
		AutoMachine::StatesType NewBackReferenceStates(const wstring& name);
		AutoMachine::StatesType NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type);
		AutoMachine::StatesType NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end);
		AutoMachine::StatesType NewFinalStates(StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& left, StatesType& right);

		
		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		AutoMachine::StatesType BuildNFA(const Ptr<Expression>& expression);
	private:
		int GetTableIndex(const CharRange& target)const;
		AutoMachine::StatesType NewStates();
		State* NewOneState();

		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	};

	//正则自动机
	
}
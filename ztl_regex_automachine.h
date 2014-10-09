#pragma once
#include "forward.h"
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
		这里是Loop subexpress 编号和bool 是否是pure. pair<int,bool>
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
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		vector<CharRange> table;
		State* start;
		State* end;
		unordered_map<wstring, pair<State*, State*>> captures;
		vector<pair<State*, State*>>				 subexpression;//用在几个lookaround上
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
	public:
		int GetTableIndex(const CharRange& target)const;
		AutoMachine::StatesType NewEpsilonStates();
		AutoMachine::StatesType NewCharStates(int index);
		AutoMachine::StatesType NewCharSetStates(const vector<int>& range);
		AutoMachine::StatesType NewAlterStates(StatesType& left, StatesType& right);
		AutoMachine::StatesType NewBeinAndEndStates(const Edge::EdgeType& type);
		AutoMachine::StatesType NewCaptureStates(StatesType& substates,const wstring& name);
		AutoMachine::StatesType NewBackReferenceStates(const wstring& name);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);

	private:
		AutoMachine::StatesType NewStates();
		Edge* NewEdge();

	};
}
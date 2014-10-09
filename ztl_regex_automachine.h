#pragma once
#include "forward.h"
namespace ztl
{
	class State;
	class Edge
	{
	public:
		//��
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
			Final, //�ߺ������ս�״̬

		};
	public:
		EdgeType type;
		State* srouce;
		State* target;
		any userdata;
		/*
		char
		������table index. int
		Capture
		������subexpression ��ź�bool �Ƿ���pure subexpress pair<int,bool>
		Loop
		������Loop subexpress ��ź�bool �Ƿ���pure. pair<int,bool>
		Head userdata����Ҫ
		TailҲ����Ҫ.
		FinalҲ����Ҫ
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		���� subexpression index
		BackReference
		������wstring name
		NameSubexprssion
		������name.
		*/
	};

	//״̬
	class State
	{
	public:
		vector<Edge*> input;
		vector<Edge*> output;
	};
	
	

	//�Զ���
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		vector<CharRange> table;
		State* start;
		State* end;
		unordered_map<wstring, pair<State*, State*>> captures;
		vector<pair<State*, State*>>				 subexpression;//���ڼ���lookaround��
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
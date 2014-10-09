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
		������table index. int
		Capture
		������subexpression ��ź�bool �Ƿ���pure subexpress pair<int,bool>
		Loop
		������Loop subexpress ��� �Ƿ���pure. pair<int,bool>
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
		unordered_map<wstring, StatesType> captures;
		vector<StatesType>				 subexpression;//���ڼ���lookaround��
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
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

		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);

	private:
		int GetTableIndex(const CharRange& target)const;
		AutoMachine::StatesType NewStates();
		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	};
}
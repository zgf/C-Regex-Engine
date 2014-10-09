#pragma once
#include "forward.h"
namespace ztl
{

	//状态
	class State
	{
		Ptr<vector<weak_ptr<Edge>>> input;
		Ptr<vector<weak_ptr<Edge>>> output;
	};
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
		End,//Capture end, loop end,
	};
	class Edge
	{
		EdgeType type;
		weak_ptr<Edge> srouce;
		weak_ptr<Edge> target;
		any userdata;
	};

	//自动机
	class AutoMachine
	{
	public:
		State* start;
		State* end;
		unordered_map<wstring, pair<State*, State*>> captures;
		vector<pair<State*, State*>>				 subexpression;

		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;

	};
}
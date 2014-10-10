#pragma once
#include "ztl_regex_automachine.h"
//接受一个NFA做输入,一个MACHINE做输入
//转换subexpression 和capture尽可能多的转换为DFA
namespace ztl
{
	class RegexOptimizer
	{
	public:
		Ptr<unordered_map<wstring, AutoMachine::StatesType>> captures;
		Ptr<vector<AutoMachine::StatesType>>				 subexpression;//用在几个lookaround上
		Ptr<vector<CharRange>> table;
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
		AutoMachine::StatesType nfa;
	public:
		RegexOptimizer() = delete;
		RegexOptimizer( const AutoMachine& _machine, const AutoMachine::StatesType& _nfa)
			:captures(_machine.captures), subexpression(_machine.subexpression),table(_machine.table)
			,states(_machine.states),edges(_machine.edges), nfa(_nfa)
		{
			
		}
		//检查当前子图是不是pure正则的 也就是只有 char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);
		//NFA to DFA
		void NfaToDfa(AutoMachine::StatesType& expression);
		//查找target的索引
		int Find(const State*& target);
	};
}
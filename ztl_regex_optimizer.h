#pragma once
#include "ztl_regex_automachine.h"
//����һ��NFA������,һ��MACHINE������
//ת��subexpression ��capture�����ܶ��ת��ΪDFA
namespace ztl
{
	class RegexOptimizer
	{
	public:
		Ptr<unordered_map<wstring, AutoMachine::StatesType>> captures;
		Ptr<vector<AutoMachine::StatesType>>				 subexpression;//���ڼ���lookaround��
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
		//��鵱ǰ��ͼ�ǲ���pure����� Ҳ����ֻ�� char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);
		//NFA to DFA
		void NfaToDfa(AutoMachine::StatesType& expression);
		//����target������
		int Find(const State*& target);
	};
}
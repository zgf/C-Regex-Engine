#include "forward.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	int AutoMachine::GetTableIndex(const CharRange& target)const
	{
		return distance(table->begin(), find(table->begin(), table->end(), target));
	}
	void AutoMachine::ConnetWith(State*& start, State*& end, const Edge::EdgeType& type)
	{
		auto&& edge = NewEdge();
		edge->type = type;
		start->output.push_back(edge);
		end->input.push_back(edge);
		edge->srouce = start;
		edge->target = end;
		assert(type != Edge::EdgeType::Epsilon);

	}
	void AutoMachine::ConnetWith(StatesType& target, const Edge::EdgeType& type)
	{
		ConnetWith(target.first, target.second, type);
	}
	
	void AutoMachine::ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata)
	{
		auto&& edge = NewEdge();
		edge->type = type;
		edge->userdata = userdata;
		start->output.push_back(edge);
		end->input.push_back(edge);
		edge->srouce = start;
		edge->target = end;
		assert(type != Edge::EdgeType::Epsilon);

	}
	void AutoMachine::ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata)
	{
		ConnetWith(target.first, target.second, type, userdata);
	}
	
	AutoMachine::StatesType AutoMachine::NewStates()
	{
		AutoMachine::StatesType result;
		this->states->emplace_back(make_shared<State>());
		result.first = states->back().get();
		this->states->emplace_back(make_shared<State>());
		result.second = states->back().get();
		return move(result);
	}
	State* AutoMachine::NewOneState()
	{
		this->states->emplace_back(make_shared<State>());
		return states->back().get();
	}
	Edge* AutoMachine::NewEdge()
	{
		this->edges->emplace_back(make_shared<Edge>());
		return edges->back().get();
	}
	
	AutoMachine::StatesType AutoMachine::NewEpsilonStates()
	{
		auto result = NewStates();
		ConnetWith(result);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCharStates(const CharRange& range)
	{
		auto&& result = NewStates();
		auto&& index = GetTableIndex(range);
		ConnetWith(result, Edge::EdgeType::Char, index);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCharSetStates(const bool reverse, const vector<CharRange>& range)
	{
		vector<int> result;
		vector<int> final;
		for(auto iter : range)
		{
			result.emplace_back(GetTableIndex(iter));
		}
		if(reverse == true)
		{
			vector<int> sum(table->size());
			std::iota(sum.begin(), sum.end(), 0);
			sort(result.begin(), result.end());
			set_difference(sum.begin(), sum.end(), result.begin(), result.end(), inserter(final, final.begin()));
		}
		else
		{
			final = move(result);
		}

		auto&& states = NewStates();
		for(auto&& iter : final)
		{
			ConnetWith(states, Edge::EdgeType::Char, iter);
		}
		return move(states);
	}
	AutoMachine::StatesType AutoMachine::NewAlterStates(StatesType& left, StatesType& right)
	{
		
		assert(left.first->input.empty());
		assert(right.first->input.empty());
		assert(left.second->output.empty());
		assert(right.second->output.empty());
		auto&& result = NewStates();
		/*ConnetWith(result.first,left.first);
		ConnetWith(result.first, right.first);
		ConnetWith(left.second,result.second);
		ConnetWith(right.second,result.second);*/
		result.first->output = left.first->output;
		//left.first->input empty 所以left节点的output的值可以随便修改,因为没有其他节点可达left.
		result.first->output.insert(result.first->output.end(), right.first->output.begin(), right.first->output.end());
		//修改输出边的源节点
		for(auto&& edge : result.first->output)
		{
			edge->srouce = result.first;
		}
		result.second->input = left.second->input;
		result.second->input.insert(result.second->input.end(), right.second->input.begin(), right.second->input.end());
		//修改输入边到新节点
		for (auto&& edge : result.second->input)
		{
			edge->target = result.second;
		}
		
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewSequenceStates(StatesType& left, StatesType& right)
	{
		//ConnetWith(left.second, right.first);
		assert(left.second->output.empty());
		assert(right.first->input.empty());
		left.second->output = right.first->output;
		return { left.first, right.second };
	}

	AutoMachine::StatesType AutoMachine::NewBeinAndEndStates(const Edge::EdgeType& type)
	{
		auto&& result = NewStates();
		ConnetWith(result, type);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCaptureStates(StatesType& substates, const wstring& name)
	{
		captures->insert({ name, substates });
		auto&& result = NewStates();
		ConnetWith(result, Edge::EdgeType::Capture, name);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewBackReferenceStates(const wstring& name)
	{
		auto&& result = NewStates();
		ConnetWith(result, Edge::EdgeType::BackReference, name);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type)
	{
		auto&& result = NewStates();
		auto&& index = GetSubexpressionIndex(substates);
		ConnetWith(result, type, index);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end)
	{
		auto&& result = NewStates();
		auto&& index = GetSubexpressionIndex(substates);
		ConnetWith(result, Edge::EdgeType::Loop, Edge::LoopUserData(index,begin,end,greedy));
		return move(result);
	}
	int AutoMachine::GetSubexpressionIndex(const StatesType& substates)
	{
		this->subexpression->emplace_back(substates);
		return subexpression->size() - 1;
	}
	AutoMachine::StatesType AutoMachine::NewFinalStates(StatesType& target)
	{
		auto&& end = NewOneState();
		ConnetWith(target.second, end, Edge::EdgeType::Final);
		return {target.first,end};
	}
	AutoMachine::StatesType AutoMachine::BuildOptimizeNFA()
	{
		auto nfa = ast->BuildNFA(this);
		//优化子表达式, DFA化
		OptimizeSubexpress();

		if (CheckPure(nfa) == true)
		{
			this->NfaToDfa(nfa);
		}
		return move(nfa);
	}

}
namespace ztl
{
	void AutoMachine::DFS(const AutoMachine::StatesType& expression)
	{
		unordered_set<State*> marks;
		function<void(State* element)> functor;
		functor = [this, &functor, &marks](State* element)
		{
			if(marks.find(element) == marks.end())
			{
				marks.insert(element);

				for(auto&& iter : element->output)
				{

					functor(iter->target);
				}
			}
		};
		functor(expression.first);
		//肯定查看到了尾部
		assert(marks.find(expression.second) != marks.end());
	}
	bool AutoMachine::CheckPure(const AutoMachine::StatesType& expression)
	{
		unordered_set<State*> marks;
		function<void(State* element)> functor;
		bool result = true;
		functor = [this, &result, &functor, &marks](State* element)
		{
			if(marks.find(element) == marks.end())
			{
				marks.insert(element);
				for(auto&& iter : element->output)
				{
					if(iter->type != Edge::EdgeType::Char &&iter->type != Edge::EdgeType::Final)
					{
						result = false;
						return;
					}
					functor(iter->target);
					if(result == false)
					{
						return;
					}
				}
			}
		};
		functor(expression.first);
		return move(result);
	}
	//现存的边 
	//Char 消耗字符
	//BackRefer 消耗字符
	//Capture 消耗字符
	//LookAround 不消耗字符
	//Begin 不消耗字符
	//End 不消耗字符
	//Final 不消耗字符
	//Loop 消耗字符
	//
	/*
	算法:
	子表达式的纯正则表达式转DFA
	*/
	Edge::EdgeType AutoMachine::GetEdgeType(int index)const
	{
		if (index <=65535)
		{
			return Edge::EdgeType::Char;
		}
		else
		{
			return Edge::EdgeType::Final;
		}
	}
	void AutoMachine::NfaToDfa(AutoMachine::StatesType& expression)
	{
		//前提条件.NFA只有 Char Final
		
		deque<State*> dfaqueue;
		//DFA到组成DFA的NFA状态的映射.
		//Final看成第65536号字符
		//DFA到NFA状态集合的映射
		unordered_map<State*, unordered_set<State*>> dfa_nfa_map;
		//不同值的边到NFA状态集合的映射
		unordered_map<int, unordered_set<State*>> edge_nfa_map;
		//NFA状态集合到DFA节点映射
		unordered_map<unordered_set<State*>,State*>nfa_dfa_map;
		//DFA的终结状态
		//unordered_set<State*> finalset;
		State* finalset;

		//初始化
		assert(expression.first->input.empty());
		auto dfa_start = NewOneState();
		dfaqueue.push_back(dfa_start);
		dfa_nfa_map.insert({ dfa_start, unordered_set<State*>({expression.first}) });
		nfa_dfa_map.insert({ unordered_set<State*>({ expression.first }), dfa_start });
		while(!dfaqueue.empty())
		{
			auto front = dfaqueue.front();
			dfaqueue.pop_front();
			//查看是否是DFA的终结状态
			assert(dfa_nfa_map.find(front) != dfa_nfa_map.end());
			if(dfa_nfa_map[front].find(expression.second)!=dfa_nfa_map[front].end())
			{
				finalset = front;
			}

			//收集边到nfa集合的映射
			for(auto&& range = dfa_nfa_map[front].begin(); range != dfa_nfa_map[front].end(); ++range)
			{
				for(auto&& edge : (*range)->output)
				{
					assert(edge->type == Edge::EdgeType::Char || edge->type == Edge::EdgeType::Final);
					if(edge->type == Edge::EdgeType::Char)
					{
						auto&& index = any_cast<int>(edge->userdata);
						if (edge_nfa_map.find(index)==edge_nfa_map.end())
						{
							edge_nfa_map.insert({ index, unordered_set<State*>() });
						}
						edge_nfa_map[index].insert(edge->target);
						
					}
					else
					{
						if(edge_nfa_map.find(table->size()) == edge_nfa_map.end())
						{
							edge_nfa_map.insert({ 65536, unordered_set<State*>() });
						}
						edge_nfa_map[table->size()].insert(edge->target);
					}
				}
			}

			for(auto&& key_iter = edge_nfa_map.begin(); key_iter != edge_nfa_map.end();++key_iter)
			{
				//获取 边的nfa集合,然后查看dfa集合 有没有状态一致的dfa.有的话,建立边
				//没有的话,新建dfa节点再建立边,并把dfa节点加入队列
				auto&& nfaset = key_iter->second;
				auto&& find_result = nfa_dfa_map.find(nfaset);
				if(find_result == nfa_dfa_map.end())
				{
					auto dfa_node = NewOneState();
					nfa_dfa_map.insert({ nfaset, dfa_node });
					dfa_nfa_map.insert({ dfa_node, nfaset });
					if(key_iter->first <= table->size())
					{
						ConnetWith(front, dfa_node, Edge::EdgeType::Char, key_iter->first);
					}
					else
					{
						ConnetWith(front, dfa_node, Edge::EdgeType::Final);
					}
					dfaqueue.push_back(dfa_node);
				}
				else if(key_iter->first <= table->size())
				{
					ConnetWith(front, find_result->second, Edge::EdgeType::Char, key_iter->first);
				}
				else
				{
					ConnetWith(front, find_result->second, Edge::EdgeType::Final);
				}

			}
			//清除边的映射
			edge_nfa_map.clear();
		}

		//assert(finalset.size() == 1);
		//auto& dfa_end = *finalset.begin();
		expression.first = move(dfa_start);
		expression.second = move(finalset);
		//expression.second = move(dfa_end);
	}
	
	
	void  AutoMachine::OptimizeSubexpress()
	{

		for(auto&& iter = captures->begin(); iter != captures->end();++iter)
		{
			auto& subexpress = iter->second;
			if (CheckPure(subexpress) == true)
			{
				NfaToDfa(subexpress);
			}
		}
		for(size_t i = 0; i < this->subexpression->size();++i)
		{
			auto&& subexpress = subexpression->at(i);
			if(CheckPure(subexpress) == true)
			{
				NfaToDfa(subexpress);
			}
		}
	}

}
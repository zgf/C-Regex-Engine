#include "forward.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	AutoMachine::AutoMachine(RegexParser& parser)
		:table(parser.GetCharTable()),
		ast(parser.GetExpressTree()),
		nfa_expression(nullptr),
		dfa_expression(nullptr)

	{
	}

	void AutoMachine::GetRangeTableIndex(const CharRange& target, vector<int>& range)const
	{
		auto&& min_index = (*table->char_table)[target.min];
		auto&& max_index = (*table->char_table)[target.max];
		for(auto i = min_index; i <= max_index; i++)
		{
			//'\0'不应该参与匹配,遇到就终结
			if(i != 0)
			{
				range.emplace_back(i);
			}
		}
	}

	void AutoMachine::ConnetWith(State*& start, State*& end, const Edge::EdgeType& type)
	{
		auto edge = NewEdge();
		edge->type = type;
		start->output.push_back(edge);
		//end->input.push_back(edge);
		//edge->srouce = start;
		edge->target = end;
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

		edge->target = end;
	}
	void AutoMachine::ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata)
	{
		ConnetWith(target.first, target.second, type, userdata);
	}

	AutoMachine::StatesType AutoMachine::NewStates()
	{
		AutoMachine::StatesType result;
		states.emplace_back(make_shared<State>());
		result.first = states.back().get();
		states.emplace_back(make_shared<State>());
		result.second = states.back().get();
		return move(result);
	}
	State* AutoMachine::NewOneState()
	{
		states.emplace_back(make_shared<State>());
		return states.back().get();
	}
	Edge* AutoMachine::NewEdge()
	{
		edges.emplace_back(make_shared<Edge>());
		return edges.back().get();
	}

	AutoMachine::StatesType AutoMachine::NewEpsilonStates()
	{
		auto result = NewStates();
		ConnetWith(result);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewNormalCharStates(const CharRange& range)
	{
		auto&& result = NewStates();
		assert(range.min == range.max);
		auto&& index = table->GetTableIndex(range.min);
		ConnetWith(result, Edge::EdgeType::Char, index);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCharSetStates(const bool reverse, const vector<CharRange>& range)
	{
		vector<int> result;
		vector<int> final;
		for(auto iter : range)
		{
			GetRangeTableIndex(iter, result);
		}

		if(reverse == true)
		{
			vector<int> sum(table->range_table->size());
			std::iota(sum.begin(), sum.end(), 1);
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
		auto&& result = NewStates();
		ConnetWith(result.first, left.first);
		ConnetWith(result.first, right.first);
		ConnetWith(left.second, result.second);
		ConnetWith(right.second, result.second);

		return result;
	}
	AutoMachine::StatesType AutoMachine::NewSequenceStates(StatesType& left, StatesType& right)
	{
		ConnetWith(left.second, right.first);
		//assert(left.second->output.empty());
		//assert(right.first->input.empty());
		//left.second->output = right.first->output;
		return { left.first, right.second };
	}
	vector<AutoMachine::StatesType> AutoMachine::CreateTheSameStateSequence(StatesType& target, int number)
	{
		vector<StatesType> stores(number);
		for(auto& element : stores)
		{
			element = this->NewIsomorphicGraph(target);
		}
		return stores;
	}

	AutoMachine::StatesType AutoMachine::NewSequenceStates(StatesType& target, int number)
	{
		if(number == 0)
		{
			auto reuslt = NewStates();
			ConnetWith(reuslt);
			return reuslt;
		}
		else if(number == 1)
		{
			auto stores = CreateTheSameStateSequence(target, number);
			return stores.front();
		}
		else
		{
			auto stores = CreateTheSameStateSequence(target, number);
			for(auto i = 1; i < stores.size(); i++)
			{
				auto& left = stores[i - 1];
				auto& right = stores[i];
				ConnetWith(left.second, right.first);
			}
			return { stores.front().first, stores.back().second };
		}
	}
	AutoMachine::StatesType AutoMachine::NewBeinAndEndStates(const Edge::EdgeType& type)
	{
		auto&& result = NewStates();
		ConnetWith(result, type);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCaptureStates(StatesType& substates, const wstring& name)
	{
		auto&& result = NewStates();

		captures.insert({ name, substates });
		ConnetWith(result, Edge::EdgeType::Capture, name);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewAnonymitCaptureStates(StatesType& substates, const int index)
	{
		auto result = NewStates();
		anonymity_captures.push_back({ substates, index });
		ConnetWith(result, Edge::EdgeType::AnonymityCapture, index);

		return move(result);
	}

	AutoMachine::StatesType AutoMachine::NewBackReferenceStates(const wstring& name)
	{
		auto&& result = NewStates();
		ConnetWith(result, Edge::EdgeType::BackReference, name);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewAnonymityBackReferenceBackReferenceStates(const int& index)
	{
		auto&& result = NewStates();
		ConnetWith(result, Edge::EdgeType::AnonymityBackReference, index);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewRegexMacroStates(const wstring& name, StatesType& substates)
	{
		macro_expression.insert({ name, substates });
		auto&& result = NewStates();
		ConnetWith(result);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewMacroReferenceStates(const wstring& name)
	{
		//构造NFA时,宏引用要扩展开来
		if(macro_expression.find(name) != macro_expression.end())
		{
			return NewIsomorphicGraph(macro_expression[name]);
		}
		else
		{
			throw exception("Can't find this name.Loop reference?");
		}
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
		auto& target = substates;
		auto begin_state = NewSequenceStates(target, begin == 0 ? 0 : begin - 1);
		auto&& end_state = NewOneState();
		//阻止DFA优化
		if(greedy == false)
		{
			none_dfa.emplace_back(end_state);
			//需要在ENFA-NFA转换时调整保存的节点
		}
		auto number = -1;

		if(end == -1)
		{
			number = 1;
		}
		else if(begin == 0)
		{
			number = end;
		}
		else if(begin != 0)
		{
			number = end - begin + 1;
		}

		auto chain = CreateTheSameStateSequence(target, number);

		for(auto i = 1; i < chain.size(); i++)
		{
			if(greedy == true)
			{
				ConnetWith(chain[i - 1].second, chain[i].first);
				ConnetWith(chain[i - 1].second, end_state);
			}
			else
			{
				ConnetWith(chain[i - 1].second, end_state);
				ConnetWith(chain[i - 1].second, chain[i].first);
			}
		}

		if(greedy == true)
		{
			ConnetWith(begin_state.second, chain.front().first);
			if(begin == 0)
			{
				ConnetWith(begin_state.second, end_state);
			}
		}
		else
		{
			if(begin == 0)
			{
				ConnetWith(begin_state.second, end_state);
			}
			ConnetWith(begin_state.second, chain.front().first);
		}

		if(end != -1)
		{
			ConnetWith(chain.back().second, end_state);
		}
		else
		{
			if(greedy == true)
			{
				ConnetWith(chain.back().second, chain.back().first);
				ConnetWith(chain.back().second, end_state);
			}
			else
			{
				ConnetWith(chain.back().second, end_state);
				ConnetWith(chain.back().second, chain.back().first);
			}
		}

		return { begin_state.first, end_state };
	}

	int AutoMachine::GetSubexpressionIndex(const StatesType& substates)
	{
		subexpression.emplace_back(substates);
		return subexpression.size() - 1;
	}
	AutoMachine::StatesType AutoMachine::NewFinalStates(StatesType& target)
	{
		auto&& end = NewOneState();
		ConnetWith(target.second, end, Edge::EdgeType::Final);
		return { target.first, end };
	}
}
namespace ztl
{
	AutoMachine::StatesType AutoMachine::NewIsomorphicGraph(StatesType& target)
	{
		//旧图到新图节点的映射
		unordered_map<State*, State*> sign;
		deque<State*> queue;
		sign.insert({ target.first, NewOneState() });
		queue.emplace_back(target.first);
		while(!queue.empty())
		{
			auto&& front = queue.front();
			for(size_t i = 0; i < front->output.size(); i++)
			{
				auto& current_edge = front->output[i];

				if(sign.find(current_edge->target) == sign.end())
				{
					//说明新节点第一次发现
					auto&& new_node = NewOneState();
					sign.insert({ current_edge->target, new_node });
					queue.push_back(current_edge->target);
				}

				ConnetWith(sign[front], sign[current_edge->target], current_edge->type, current_edge->userdata);
			}
			queue.pop_front();
		}

		return { sign[target.first], sign[target.second] };
	}
	bool AutoMachine::IsLookAroundEge(Edge* target)
	{
		return target->type == Edge::EdgeType::NegativeLookahead ||
			target->type == Edge::EdgeType::PositivetiveLookahead ||
			target->type == Edge::EdgeType::NegativeLookbehind ||
			target->type == Edge::EdgeType::PositiveLookbehind;
	}
	AutoMachine::StatesType&	AutoMachine::HaveSubGraph(State* target, const int index)
	{
		if(target->output[index]->type == Edge::EdgeType::Capture)
		{
			auto name = any_cast<wstring>(target->output[index]->userdata);
			return captures[name];
		}
		else if(target->output[index]->type == Edge::EdgeType::AnonymityCapture)
		{
			auto name = any_cast<int>(target->output[index]->userdata);
			return anonymity_captures[name].first;
		}
		else if(IsLookAroundEge(target->output[index]))
		{
			auto name = any_cast<int>(target->output[index]->userdata);
			return subexpression[name];
		}
		throw exception("Can't find SubGraph");
	}
	bool AutoMachine::NoneSubGraph(State* target, const int index)
	{
		return target->output[index]->type == Edge::EdgeType::Char ||
			target->output[index]->type == Edge::EdgeType::BackReference ||
			target->output[index]->type == Edge::EdgeType::Epsilon ||
			target->output[index]->type == Edge::EdgeType::Final ||
			target->output[index]->type == Edge::EdgeType::Tail ||
			target->output[index]->type == Edge::EdgeType::Head;
	}
	void AutoMachine::ChangeLookAroundDirect(Edge* target)
	{
		if(target->type == Edge::EdgeType::PositiveLookbehind)
		{
			target->type = Edge::EdgeType::PositivetiveLookahead;
		}
		else if(target->type == Edge::EdgeType::PositivetiveLookahead)
		{
			target->type = Edge::EdgeType::PositiveLookbehind;
		}
		else if(target->type == Edge::EdgeType::NegativeLookbehind)
		{
			target->type = Edge::EdgeType::NegativeLookahead;
		}
		else if(target->type == Edge::EdgeType::NegativeLookahead)
		{
			target->type = Edge::EdgeType::NegativeLookbehind;
		}
	}

	//将target指向的图反转
	State* AutoMachine::NewReverseGraph(State* target)
	{
		deque<pair<State*, int>>queue;
		unordered_set<State*> sign;
		queue.push_back({ target, target->output.size() });
		sign.insert(target);

		while(!queue.empty())
		{
			auto front = queue.front();
			for(auto i = 0; i < front.second; i++)
			{
				auto edge = front.first->output[i];
				auto next_node = edge->target;
				if(!NoneSubGraph(front.first, i))
				{
					auto& subexpress = HaveSubGraph(front.first, i);
					NewReverseGraph(subexpress.first);
					swap(subexpress.first, subexpress.second);
					//在逆图中,断言方向需要反向才能嵌套断言
					ChangeLookAroundDirect(edge);
				}
				//如果下一节点没看到过先保存下一节点的数据
				if(sign.find(next_node) == sign.end())
				{
					sign.insert(next_node);
					queue.push_back({ next_node, next_node->output.size() });
				}
				edge->target = front.first;
				next_node->output.push_back(edge);
			}
			front.first->output.erase(front.first->output.begin(), front.first->output.begin() + front.second);
			queue.pop_front();
		}
		return target;
	}
	void AutoMachine::SortAnonymityCaptures()
	{
		//构建完nfa_express就做这个
		assert(nfa_expression != nullptr);
		sort(anonymity_captures.begin(), anonymity_captures.end(), [](const pair<AutoMachine::StatesType, int>& left, const pair<AutoMachine::StatesType, int>& right)
		{
			return left.second < right.second;
		});
	}
	void AutoMachine::BuildOptimizeNFA()
	{
		nfa_expression = make_shared<AutoMachine::StatesType>(ast->BuildNFA(this));

		SortAnonymityCaptures();

		assert(std::is_sorted(anonymity_captures.begin(), anonymity_captures.end(), [](const pair<AutoMachine::StatesType, int>& left, const pair<AutoMachine::StatesType, int>& right)
		{
			return left.second < right.second;
		}));

		*nfa_expression = EpsilonNFAtoNFA(*nfa_expression);
		ReverseLookBehindGraph(*nfa_expression);
		//对逆向lookaround需要的逆图进行处理
		if(CheckPure(*nfa_expression) == true)
		{
			dfa_expression = make_shared<DFA>(this->NfaToDfa(*nfa_expression));
		}
		//优化子表达式, DFA化
		OptimizeSubexpress();
	}
	void	AutoMachine::ReverseLookBehindGraph(AutoMachine::StatesType& target)
	{
		deque<State*> queue;
		//待处理边表

		unordered_set<State*> sign;
		queue.push_back(target.first);
		sign.insert(target.first);
		while(!queue.empty())
		{
			auto& front = queue.front();

			////处理nfa的每一条边
			for(auto&& edge : front->output)
			{
				//在之前对NFARoot调用了ENFA2NFA
				assert(edge->type != Edge::EdgeType::Epsilon);
				if(IsLookBehindEdge(edge))
				{
					auto index = any_cast<int>(edge->userdata);
					NewReverseGraph(subexpression[index].first);
					swap(subexpression[index].first, subexpression[index].second);
				}
				if(sign.find(edge->target) == sign.end())
				{
					//新的未处理过的节点
					queue.push_back(edge->target);
					sign.insert(edge->target);
				}
			}
			queue.pop_front();
		}
	}
	bool AutoMachine::CheckPure(const AutoMachine::StatesType& target)
	{
		deque<State*> queue;
		//待处理边表

		unordered_set<State*> sign;
		queue.push_back(target.first);
		sign.insert(target.first);
		while(!queue.empty()&& target.first!=target.second)
		{
			auto& front = queue.front();
			if (find(none_dfa.begin(),none_dfa.end(),front)!=none_dfa.end())
			{
				return false;
			}
			////处理nfa的每一条边
			for(auto&& edge : front->output)
			{
				//在之前对NFARoot调用了ENFA2NFA
				assert(edge->type != Edge::EdgeType::Epsilon);
				if(edge->type != Edge::EdgeType::Char &&edge->type != Edge::EdgeType::Final)
				{
					return false;
				}

				if(sign.find(edge->target) == sign.end())
				{
					//新的未处理过的节点
					queue.push_back(edge->target);
					sign.insert(edge->target);
				}
			}
			queue.pop_front();
		}
		return true;
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

	void AutoMachine::CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, vector< unordered_set<State*>>& edge_nfa_map, const int final_index)
	{
		//收集边到nfa集合的映射
		for(auto&& range = dfa_nfa_map[front].begin(); range != dfa_nfa_map[front].end(); ++range)
		{
			for(auto&& edge : (*range)->output)
			{
				assert(edge->type == Edge::EdgeType::Char || edge->type == Edge::EdgeType::Final);
				if(edge->type == Edge::EdgeType::Char)
				{
					auto&& index = any_cast<int>(edge->userdata);

					edge_nfa_map[index].insert(edge->target);
				}
				else if(edge->type == Edge::EdgeType::Final)
				{
					edge_nfa_map[final_index].insert(edge->target);
				}
			}
		}
	}

	void AutoMachine::ClearEdgeNfaMap(vector<int>& need_clear, vector<unordered_set<State*>>& edge_nfa_map)
	{
		for(auto& element : need_clear)
		{
			edge_nfa_map[element].clear();
		}
		need_clear.clear();
	}
	DFA AutoMachine::NfaToDfa(AutoMachine::StatesType& expression)
	{
		//前提条件.NFA只有 Char Final E
		//表的值-1表示不接受状态,表值final_index表示接受状态
		//第一维是当前节点编号,第二维是边index,值是下一个节点号
		DFA result;
		int final_index = table->range_table->size();
		int edge_sum = final_index + 1;
		vector<vector<int>> dfa_table;
		//编号所代表的dfa到nfa集合的映射
		vector<unordered_set<State*>> dfa_nfa_map;
		//nfa到dfa的集合的映射
		unordered_map<unordered_set<State*>, int> nfa_dfa_map;
		//待处理的dfa队列
		deque<int> dfaqueue;

		int final_dfa = -1;
		//	//不同值的边到NFA状态集合的映射
		//这里可以改vector
		vector<unordered_set<State*>> edge_nfa_map(edge_sum);
		dfa_table.emplace_back(vector<int>(edge_sum, -1));

		dfa_nfa_map.emplace_back(unordered_set<State*>({ expression.first })/*EpsilonNFASet(*/)/*)*/;
		nfa_dfa_map.insert({ dfa_nfa_map.back(), dfa_nfa_map.size() - 1 });
		//初始化队列
		dfaqueue.emplace_back(0);
		vector<int> need_clear;
		need_clear.reserve(8);
		while(!dfaqueue.empty())
		{
			auto&& front = dfaqueue.front();
			if(dfa_nfa_map[front].find(expression.second) != dfa_nfa_map[front].end())
			{
				final_dfa = front;
			}

			CollecteEdgeToNFAMap(dfa_nfa_map, front, edge_nfa_map, final_index);

			for(auto&& i = 0; i < edge_nfa_map.size(); ++i)
			{
				//获取 边的nfa集合,然后查看dfa集合 有没有状态一致的dfa.有的话,建立边
				//没有的话,新建dfa节点再建立边,并把dfa节点加入队列
				if(!edge_nfa_map[i].empty())
				{
					need_clear.emplace_back(i);
					auto&& nfaset = edge_nfa_map[i];
					//auto&& enfaset = EpsilonNFASet(nfaset);

					auto&& find_result = nfa_dfa_map.find(nfaset);
					if(find_result == nfa_dfa_map.end())
					{
						//新节点
						dfa_table.emplace_back(vector<int>(edge_sum, -1));
						dfa_nfa_map.emplace_back(nfaset);
						auto&& dfa_node = dfa_table.size() - 1;
						nfa_dfa_map.insert({ nfaset, dfa_node });
						dfa_table[front][i] = dfa_node;
						dfaqueue.emplace_back(dfa_node);
					}
					else
					{
						dfa_table[front][i] = find_result->second;
					}
				}
			}

			ClearEdgeNfaMap(need_clear, edge_nfa_map);
			//清除边的映射

			dfaqueue.pop_front();
		}
		//assert(final_dfa.size() == 1);
		result.dfa = move(dfa_table);
		result.finalset = final_dfa;
		return move(result);
	}
	/*void	AutoMachine::AllNFAAddTheFinal()
	{
		for(auto&& iter = captures.begin(); iter != captures.end(); ++iter)
		{
			auto& subexpress = iter->second;
			subexpress = NewFinalStates(subexpress);
		}
		for(size_t i = 0; i < subexpression.size(); ++i)
		{
			auto&& subexpress = subexpression[i];
			subexpress = NewFinalStates(subexpress);
		}
		for(size_t i = 0; i < this->anonymity_captures.size(); ++i)
		{
			auto&& subexpress = anonymity_captures[i].first;
			subexpress = NewFinalStates(subexpress);
		}
	}
	void	AutoMachine::AllEpsilonNFAtoNFA()
	{
		for(auto&& iter = captures.begin(); iter != captures.end(); ++iter)
		{
			auto& subexpress = iter->second;
			subexpress = EpsilonNFAtoNFA(subexpress);
		}
		for(size_t i = 0; i < subexpression.size(); ++i)
		{
			auto&& subexpress = subexpression[i];
			subexpress = EpsilonNFAtoNFA(subexpress);
		}
		for(size_t i = 0; i < this->anonymity_captures.size(); ++i)
		{
			auto&& subexpress = anonymity_captures[i].first;
			subexpress = EpsilonNFAtoNFA(subexpress);
		}
	}
	void	AutoMachine::AllChangeNFAToDFA()
	{
		for(auto&& iter = captures.begin(); iter != captures.end(); ++iter)
		{
			auto& subexpress = iter->second;
			if(CheckPure(subexpress))
			{
				dfa_captures.insert({ iter->first, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < subexpression.size(); ++i)
		{
			auto&& subexpress = subexpression[i];
			if(CheckPure(subexpress))
			{
				dfa_subexpression.insert({ i, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < this->anonymity_captures.size(); ++i)
		{
			auto&& subexpress = anonymity_captures[i].first;
			if(CheckPure(subexpress))
			{
				dfa_anonymity_captures.insert({ i, NfaToDfa(subexpress) });
			}
		}
	}*/
	void  AutoMachine::OptimizeSubexpress()
	{
		for(auto&& iter = captures.begin(); iter != captures.end(); ++iter)
		{
			auto& subexpress = iter->second;
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);
			if(CheckPure(subexpress) == true)
			{
				dfa_captures.insert({ iter->first, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < subexpression.size(); ++i)
		{
			auto&& subexpress = subexpression[i];
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);

			if(CheckPure(subexpress) == true)
			{
				dfa_subexpression.insert({ i, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < this->anonymity_captures.size(); ++i)
		{
			auto&& subexpress = anonymity_captures[i].first;
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);
			if(CheckPure(subexpress) == true)
			{
				dfa_anonymity_captures.insert({ i, NfaToDfa(subexpress) });
			}
		}
	}
	bool AutoMachine::IsLookBehindEdge(Edge* target)
	{
		return 	target->type == Edge::EdgeType::PositiveLookbehind ||
			target->type == Edge::EdgeType::NegativeLookbehind;
	}
	//按顺序获取从target节点可达的所有非空边
	vector<Edge*> AutoMachine::GetSortedReachNoneEpsilonEdge(State* target, unordered_map<State*, State*>& state_map, State* front, unordered_set<State*>& sign)
	{
		state_map.insert({ target, front });
		vector<int> eposition;
		eposition.emplace_back(-1);
		vector<Edge*>result;
		vector<vector<Edge*>>save;

		for(auto i = 0; i < target->output.size(); i++)
		{
			auto& element = target->output[i];
			if(element->type == Edge::EdgeType::Epsilon)
			{
				if(sign.find(element->target) == sign.end())
				{
					sign.insert(element->target);
					auto temp = GetSortedReachNoneEpsilonEdge(element->target, state_map, front, sign);
					save.emplace_back(move(temp));
					eposition.emplace_back(i);
				}
				else
				{
					save.emplace_back(vector<Edge*>({}));
					eposition.emplace_back(i);
				}
			}
		}

		for(auto i = 0; i < eposition.size() - 1; i++)
		{
			if(target->output[eposition[i + 1]]->type == Edge::EdgeType::Epsilon)
			{
				result.insert(result.end(), next(target->output.begin(), eposition[i] + 1), next(target->output.begin(), eposition[i + 1]));
			}
			result.insert(result.end(), save[i].begin(), save[i].end());
		}
		result.insert(result.end(), next(target->output.begin(), eposition.back() + 1), target->output.end());
		return result;
	}

	vector<Edge*> AutoMachine::GetSortedReachNoneEpsilonEdge(State* target, unordered_map<State*, State*>& state_map, State* front)
	{
		unordered_set<State*> sign;
		return GetSortedReachNoneEpsilonEdge(target, state_map, front, sign);
	}
	//从ENFA到普通NFA
	AutoMachine::StatesType AutoMachine::EpsilonNFAtoNFA(const AutoMachine::StatesType& target)
	{
		//从旧状态到新状态的映射
		unordered_map<State*, State*> state_map;
		//状态映射,还未处理的状态队列
		deque<State*> queue;
		//待处理边表
		vector<Edge*> edge_queue;
		unordered_set<State*> sign;
		queue.emplace_back(target.first);
		sign.insert(target.first);
		while(!queue.empty())
		{
			auto& front = queue.front();

			front->output = move(GetSortedReachNoneEpsilonEdge(front, state_map, front));
			////处理enfaset的每一条边

			for(auto&& edge : front->output)
			{
				assert(edge->type != Edge::EdgeType::Epsilon);

				if(state_map.find(edge->target) != state_map.end())
				{
					edge->target = state_map[edge->target];
				}
				else
				{
					if(sign.find(edge->target) == sign.end())
					{
						//新的未处理过的节点
						sign.insert(edge->target);
						queue.emplace_back(edge->target);
						edge_queue.emplace_back(edge);
					}
				}
			}
			queue.pop_front();
		}

		for(auto&& edge : edge_queue)
		{
			assert(state_map.find(edge->target) != state_map.end());
			edge->target = state_map[edge->target];
		}

		//处理一下non_dfa
		for(auto& element : none_dfa)
		{
			element = state_map[element];
		}
		//新的开始和结束节点
		return { state_map[target.first], state_map[target.second] };
	}
}
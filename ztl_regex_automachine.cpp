#include "forward.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	AutoMachine::AutoMachine(RegexParser& parser)
		:table(parser.GetCharTable()),
		states(make_shared<vector<Ptr<State>>>()),
		edges(make_shared<vector<Ptr<Edge>>>()),
		captures(make_shared<unordered_map<wstring, StatesType>>()),
		subexpression(make_shared<vector<StatesType>>()),
		anonymity_captures(make_shared<vector<StatesType>>()),
		ast(parser.GetExpressTree()),
		dfa_anonymity_captures(make_shared<unordered_map<int, DFA>>()),
		dfa_captures(make_shared<unordered_map<wstring, DFA>>()),
		dfa_subexpression(make_shared<unordered_map<int, DFA>>()),
		nfa_expression(nullptr),
		dfa_expression(nullptr)

	{
	}

	void AutoMachine::GetTableIndex(const CharRange& target, vector<int>& range)const
	{
		auto&& min_index = (*table->char_table)[target.min];
		auto&& max_index = (*table->char_table)[target.max];
		for(auto i = min_index; i <= max_index; i++)
		{
			range.emplace_back(i);
		}
	}
	int AutoMachine::GetTableIndex(const int& target)const
	{
		return (*table->char_table)[target];
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
		//end->input.push_back(edge);
		//edge->srouce = start;
		edge->target = end;
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
		assert(range.min == range.max);
		auto&& index = GetTableIndex(range.min);
		ConnetWith(result, Edge::EdgeType::Char, index);
		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewCharSetStates(const bool reverse, const vector<CharRange>& range)
	{
		vector<int> result;
		vector<int> final;
		for(auto iter : range)
		{
			GetTableIndex(iter, result);
		}

		if(reverse == true)
		{
			vector<int> sum(table->range_table->size());
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
		//assert(left.first->input.empty());
		//assert(right.first->input.empty());
		//��Ϊ���ڽڵ�����ս�ڵ�ָ�ؿ�ʼ�ڵ�����,��������Ķ��Բ�����Ҫ
		/*assert(left.second->output.empty());
		assert(right.second->output.empty());*/
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
	vector<AutoMachine::StatesType> AutoMachine::NewStateSequence(StatesType& target, int number)
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
			auto stores = NewStateSequence(target, number);
			return stores.front();
		}
		else
		{
			auto stores = NewStateSequence(target, number);
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
		if(name.empty())
		{
			capture_count++;
			anonymity_captures->emplace_back(substates);
			ConnetWith(result, Edge::EdgeType::AnonymityCapture, capture_count);
		}
		else
		{
			captures->insert({ name, substates });
			ConnetWith(result, Edge::EdgeType::Capture, name);
		}
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
		//����NFAʱ,������Ҫ��չ����
		return NewIsomorphicGraph(macro_expression[name]);
	}
	AutoMachine::StatesType AutoMachine::NewLookAroundStates(StatesType& substates, const Edge::EdgeType& type)
	{
		auto&& result = NewStates();
		auto&& index = GetSubexpressionIndex(substates);
		ConnetWith(result, type, index);
		return move(result);
	}
	pair<Edge::EdgeType, Edge::EdgeType > SetPriority(bool greedy)
	{
		auto height = Edge::EdgeType::Final;
		auto low = Edge::EdgeType::Epsilon;
		if(greedy == true)
		{
			height = Edge::EdgeType::Epsilon;
		}
		return { height, low };
	}
	AutoMachine::StatesType AutoMachine::ConnectLoopChain(bool greedy, AutoMachine::StatesType& loop_head, int number)
	{
		auto priority = SetPriority(greedy);
		auto height = priority.first;
		auto low = priority.second;
		vector<AutoMachine::StatesType> chain;
		chain.emplace_back(loop_head);
		auto temp = NewStateSequence(loop_head, number);
		chain.insert(chain.end(), temp.begin(), temp.end());
		auto end_state = NewOneState();
		assert(chain.size() > 1);
		for(auto i = 1; i < chain.size(); i++)
		{
			ConnetWith(chain[i - 1].second, chain[i].first, height);
			ConnetWith(chain[i - 1].second, end_state, low);
		}

		ConnetWith(chain.back().second, end_state, Edge::EdgeType::Epsilon);
		return { loop_head.first, end_state };
	}

	AutoMachine::StatesType AutoMachine::LoopIncludeInFinite(bool greedy, int number, AutoMachine::StatesType& substates)
	{
		assert(number > 1);
		auto end_node = NewOneState();
		auto priority = SetPriority(greedy);
		auto height = priority.first;
		auto low = priority.second;
		if(number > 1)
		{
			auto loop_head = NewSequenceStates(substates, number - 1);
			auto middle = NewIsomorphicGraph(substates);
			auto end_node = NewOneState();
			ConnetWith(loop_head.second, middle.first);
			ConnetWith(middle.second, middle.first, height);
			ConnetWith(middle.second, end_node, low);
			return { loop_head.first, end_node };
		}
	}

	AutoMachine::StatesType AutoMachine::NewChooseClourseStates(bool greedy, StatesType& target)
	{
		auto&& new_state = NewStates();
		ConnetWith(new_state.first, target.first);
		ConnetWith(target.second, new_state.second);
		if(greedy == true)
		{
			ConnetWith(new_state);
		}
		else
		{
			ConnetWith(new_state, Edge::EdgeType::Final);
		}
		return new_state;
	}
	AutoMachine::StatesType AutoMachine::NewPositiveClourseStates(bool greedy, StatesType& target)
	{
		auto head = NewIsomorphicGraph(target);
		auto kleen = NewKleenClourseStates(greedy, target);

		ConnetWith(head.second, kleen.first);

		return { head.first, kleen.second };
	}
	AutoMachine::StatesType AutoMachine::NewKleenClourseStates(bool greedy, StatesType& target)
	{
		auto&& result = NewStates();
		if(greedy == true)
		{
			ConnetWith(result.second, target.first);
		}
		else
		{
			ConnetWith(result.second, target.first, Edge::EdgeType::Final);
		}
		ConnetWith(target.second, result.first);
		ConnetWith(result.first, result.second);
		return result;
	}
	AutoMachine::StatesType AutoMachine::NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end)
	{
		//

		//

		//if(begin == 0 && end == 1)
		//{
		//	auto&& new_state = NewStates();
		//	ConnetWith(new_state.first, substates.first);
		//	ConnetWith(substates.second, new_state.second);
		//	ConnetWith(new_state);
		//	return new_state;
		//	//return NewChooseClourseStates(greedy,substates);
		//}
		//else if(begin == 0 && end == -1)
		//{
		//	ConnetWith(substates.second, substates.first);
		//	ConnetWith(substates.first, substates.second);
		//	return substates;
		//	//return NewKleenClourseStates(greedy,substates);
		//}
		//else if(begin == 1 && end == -1)
		//{
		//	ConnetWith(substates.second, substates.first);
		//	return substates;
		//	//return NewPositiveClourseStates(greedy,substates);
		//}
		//else if(begin == end)
		//{
		//
		//	if(begin == 1)
		//	{
		//		return substates;
		//	}
		//	else
		//	{
		//		auto stores = NewStateSequence(substates, begin);
		//		for(auto i = 1; i < stores.size(); i++)
		//		{
		//			auto& left = stores[i - 1];
		//			auto& right = stores[i];
		//			ConnetWith(left.second, right.first);
		//		}
		//		return { stores.front().first, stores.back().second };
		//	}
		//}

		auto& target = substates;
		auto begin_state = NewSequenceStates(target, begin == 0 ? 0 : begin - 1);
		auto&& end_state = NewOneState();

		auto priority = SetPriority(greedy);
		auto height = priority.first;
		auto low = priority.second;
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

		auto chain = NewStateSequence(target, number);

		for(auto i = 1; i < chain.size(); i++)
		{
			ConnetWith(chain[i - 1].second, chain[i].first, height);
			ConnetWith(chain[i - 1].second, end_state, low);
		}

		if(end != -1)
		{
			ConnetWith(begin_state.second, chain.front().first);
			ConnetWith(chain.back().second, end_state);
		}
		else
		{
			ConnetWith(begin_state.second, chain.front().first);
			ConnetWith(chain.back().second, end_state, low);
			ConnetWith(chain.back().second, chain.back().first, height);
		}

		if(begin == 0)
		{
			ConnetWith(begin_state.second, end_state);
		}
		return { begin_state.first, end_state };
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
		return { target.first, end };
	}
	void AutoMachine::BuildOptimizeNFA()
	{
		nfa_expression = make_shared<AutoMachine::StatesType>(ast->BuildNFA(this));
		//�Ż��ӱ��ʽ, DFA��
		OptimizeSubexpress();

		if(CheckPure(*nfa_expression) == true)
		{
			dfa_expression = make_shared<DFA>(this->NfaToDfa(EpsilonNFAtoNFA(*nfa_expression)));
		}
		else
		{
			*nfa_expression = EpsilonNFAtoNFA(*nfa_expression);
		}
	}
}
namespace ztl
{
	unordered_set<State*> AutoMachine::FindReachTargetStateSet(State* start, State* target)
	{
		unordered_set<State*> result;
		unordered_set<State*> marks;
		function<void(State* element)> functor;
		functor = [this, &functor, &target, &result, &marks](State* element)
		{
			if(marks.find(element) == marks.end())
			{
				marks.insert(element);

				for(auto&& iter : element->output)
				{
					if(iter->target == target)
					{
						result.insert(element);
					}
					functor(iter->target);
				}
			}
		};
		functor(start);
		//�϶��鿴����β��
		return result;
	}
	AutoMachine::StatesType AutoMachine::NewIsomorphicGraph(StatesType& target)
	{
		//��ͼ����ͼ�ڵ��ӳ��
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
					//˵���½ڵ��һ�η���
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
		//�϶��鿴����β��
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
					if(iter->type != Edge::EdgeType::Char &&iter->type != Edge::EdgeType::Final&&iter->type != Edge::EdgeType::Epsilon)
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
	//�ִ�ı�
	//Char �����ַ�
	//BackRefer �����ַ�
	//Capture �����ַ�
	//LookAround �������ַ�
	//Begin �������ַ�
	//End �������ַ�
	//Final �������ַ�
	//Loop �����ַ�
	//
	/*
	�㷨:
	�ӱ��ʽ�Ĵ�������ʽתDFA
	*/
	Edge::EdgeType AutoMachine::GetEdgeType(int index)const
	{
		if(index <= 65535)
		{
			return Edge::EdgeType::Char;
		}
		else
		{
			return Edge::EdgeType::Final;
		}
	}
	void AutoMachine::CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, vector< unordered_set<State*>>& edge_nfa_map, const int final_index)
	{
		//�ռ��ߵ�nfa���ϵ�ӳ��
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
	void AutoMachine::CreatDFAStateByEdgeToNFAMap(unordered_map<int, unordered_set<State*>>& edge_nfa_map, unordered_map<unordered_set<State*>, int>& nfa_dfa_map, int& front, vector<vector<int>>& dfa_table, vector<unordered_set<State*>>& dfa_nfa_map, const int edge_sum, deque<int>& dfaqueue)
	{
	}
	void ClearEdgeNfaMap(vector<int>& need_clear, vector<unordered_set<State*>>& edge_nfa_map)
	{
		for(auto& element : need_clear)
		{
			edge_nfa_map[element].clear();
		}
		need_clear.clear();
	}
	DFA AutoMachine::NfaToDfa(AutoMachine::StatesType& expression)
	{
		//ǰ������.NFAֻ�� Char Final E
		//���ֵ-1��ʾ������״̬,��ֵfinal_index��ʾ����״̬
		//��һά�ǵ�ǰ�ڵ���,�ڶ�ά�Ǳ�index,ֵ����һ���ڵ��
		DFA result;
		int final_index = table->range_table->size();
		int edge_sum = final_index + 1;
		vector<vector<int>> dfa_table;
		//����������dfa��nfa���ϵ�ӳ��
		vector<unordered_set<State*>> dfa_nfa_map;
		//nfa��dfa�ļ��ϵ�ӳ��
		unordered_map<unordered_set<State*>, int> nfa_dfa_map;
		//�������dfa����
		deque<int> dfaqueue;

		int final_dfa = -1;
		//	//��ֵͬ�ıߵ�NFA״̬���ϵ�ӳ��
		//������Ը�vector
		vector<unordered_set<State*>> edge_nfa_map(edge_sum);
		dfa_table.emplace_back(vector<int>(edge_sum, -1));
		//unordered_set<State*>temp;
		//temp.insert();

		dfa_nfa_map.emplace_back(unordered_set<State*>({ expression.first })/*EpsilonNFASet(*/)/*)*/;
		nfa_dfa_map.insert({ dfa_nfa_map.back(), dfa_nfa_map.size() - 1 });
		//��ʼ������
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
				//��ȡ �ߵ�nfa����,Ȼ��鿴dfa���� ��û��״̬һ�µ�dfa.�еĻ�,������
				//û�еĻ�,�½�dfa�ڵ��ٽ�����,����dfa�ڵ�������
				if(!edge_nfa_map[i].empty())
				{
					need_clear.emplace_back(i);
					auto&& nfaset = edge_nfa_map[i];
					//auto&& enfaset = EpsilonNFASet(nfaset);

					auto&& find_result = nfa_dfa_map.find(nfaset);
					if(find_result == nfa_dfa_map.end())
					{
						//�½ڵ�
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
			//����ߵ�ӳ��

			dfaqueue.pop_front();
		}
		//assert(final_dfa.size() == 1);
		result.dfa = move(dfa_table);
		result.finalset = final_dfa;
		return move(result);
	}

	unordered_set<State*> AutoMachine::EpsilonNFASet(State* target)
	{
		unordered_set<State*> result;
		result.insert(target);
		deque<State*> queue;
		queue.emplace_back(target);
		while(!queue.empty())
		{
			State*& front = queue.front();

			for(Edge*& element : front->output)
			{
				if(element->type == Edge::EdgeType::Epsilon)
				{
					if(find(result.begin(), result.end(), element->target) == result.end())
					{
						result.insert(element->target);
						queue.push_back(element->target);
					}
				}
			}
			queue.pop_front();
		}
		return result;
	}
	unordered_set<State*> AutoMachine::EpsilonNFASet(unordered_set<State*>& target)
	{
		unordered_set<State*> result;
		for(auto&& Iter : target)
		{
			auto&& temp = EpsilonNFASet(Iter);
			result.insert(temp.begin(), temp.end());
		}
		return result;
	}
	void  AutoMachine::OptimizeSubexpress()
	{
		for(auto&& iter = captures->begin(); iter != captures->end(); ++iter)
		{
			auto& subexpress = iter->second;
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);
			if(CheckPure(subexpress) == true)
			{
				dfa_captures->insert({ iter->first, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < this->subexpression->size(); ++i)
		{
			auto&& subexpress = subexpression->at(i);
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);

			if(CheckPure(subexpress) == true)
			{
				dfa_subexpression->insert({ i, NfaToDfa(subexpress) });
			}
		}
		for(size_t i = 0; i < this->anonymity_captures->size(); ++i)
		{
			auto&& subexpress = anonymity_captures->at(i);
			subexpress = NewFinalStates(subexpress);
			subexpress = EpsilonNFAtoNFA(subexpress);
			if(CheckPure(subexpress) == true)
			{
				dfa_anonymity_captures->insert({ i, NfaToDfa(subexpress) });
			}
		}
	}
	//��ȡ��target�ڵ�ɴ�����зǿձ�
	vector<Edge*> GetReachNoneEEdge(State* target, unordered_map<State*, State*>& state_map,State* front)
	{
		state_map.insert({ target, front });
		int last = -1;
		vector<Edge*>result;
		for(auto i = 0; i < target->output.size(); i++)
		{
			auto& element = target->output[i];
			if(element->type == Edge::EdgeType::Epsilon)
			{
				auto temp = GetReachNoneEEdge(element->target,state_map,front);
				
				result.insert(result.end(), next(target->output.begin(),last + 1), next(target->output.begin(), i));
				last = i;
				result.insert(result.end(), temp.begin(), temp.end());
			}
		}
		result.insert(result.end(), next(target->output.begin(), last + 1), target->output.end());
		return move(result);
	}
	//��ENFA����ͨNFA
	AutoMachine::StatesType AutoMachine::EpsilonNFAtoNFA(const AutoMachine::StatesType& target)
	{
		//�Ӿ�״̬����״̬��ӳ��
		unordered_map<State*, State*> state_map;
		//״̬ӳ��,��δ�����״̬����
		deque<State*> queue;
		//������߱�
		vector<Edge*> edge_queue;
		unordered_set<State*> sign;
		queue.emplace_back(target.first);
		sign.insert(target.first);
		while(!queue.empty())
		{
			auto& front = queue.front();

			front->output = move(GetReachNoneEEdge(front, state_map, front));
			//auto&& enfaset = EpsilonNFASet(front);
			////enfaset����ӳ���
			//for(auto& element : enfaset)
			//{
			//	state_map.insert({ element, front });
			//}
			////enfaset��ÿһ���߷ǿձ߽���front,front�Ŀձ�ɾ��
			//vector<Edge*> new_front_output;
			//for(auto& element : enfaset)
			//{
			//	for(auto&& edge : element->output)
			//	{
			//		if(edge->type != Edge::EdgeType::Epsilon)
			//		{
			//			new_front_output.emplace_back(edge);
			//		}
			//	}
			//}
			//front->output = move(new_front_output);
			////����enfaset��ÿһ����

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
						//�µ�δ������Ľڵ�
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
		//�µĿ�ʼ�ͽ����ڵ�
		return { state_map[target.first], state_map[target.second] };
	}
}
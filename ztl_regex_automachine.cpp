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
		auto&& edge = NewEdge();
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
		ConnetWith(left.second,result.second);
		ConnetWith(right.second,result.second);
	

		return move(result);
	}
	AutoMachine::StatesType AutoMachine::NewSequenceStates(StatesType& left, StatesType& right)
	{
		ConnetWith(left.second, right.first);
		//assert(left.second->output.empty());
		//assert(right.first->input.empty());
		//left.second->output = right.first->output;
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
	AutoMachine::StatesType AutoMachine::NewLoopStates(StatesType& substates, const bool greedy, const int begin, const int end)
	{
		//�޸Ķ��� *+?��̰��ƥ��,ֱ����չ����
		if(greedy == true && begin == 0 && end == 1)
		{
			auto&& new_state = NewStates();
			ConnetWith(new_state);
			ConnetWith(new_state.first, substates.first);
			ConnetWith(substates.second, new_state.second);
			return new_state;
		}
		else if(greedy == true && begin == 0 && end == -1)
		{
			auto&& result = NewStates();
			ConnetWith(result);
			ConnetWith(result.second, substates.first);
			ConnetWith(substates.second, result.first);
			return move(result);
		}
		else if(greedy == true && begin == 1 && end == -1)
		{
			ConnetWith(substates.second, substates.first);
			return substates;
		}
		else if(greedy == true && begin == end)
		{
			assert(begin != 0);
			vector<AutoMachine::StatesType> result(begin);
			result[0] = substates;
			for(auto i = 1; i < result.size(); i++)
			{
				result[i] = this->NewIsomorphicGraph(substates);
			}
			for(auto i = 0; i < result.size() - 1; i++)
			{
				auto&& left = result[i];
				auto&& right = result[i + 1];
				ConnetWith(left.second, right.first);
			}
			return { result[0].first, result.back().second };
		}
		else
		{
			auto&& result = NewStates();
			auto&& index = GetSubexpressionIndex(substates);
			ConnetWith(result, Edge::EdgeType::Loop, Edge::LoopUserData(index, begin, end, greedy));
			return move(result);
		}
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
					if (iter->target == target)
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

				/*Edge* new_dege = NewEdge();
				new_dege->type = current_edge->type;
				new_dege->userdata = current_edge->userdata;
				new_dege->target = sign[current_edge->target];*/
				//new_dege->srouce = sign[front];
				ConnetWith(sign[front], sign[current_edge->target], current_edge->type, current_edge->userdata);
				//sign[front]->output.push_back(new_dege);
				//sign[current_edge->target]->input.emplace_back(new_dege);
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
	void AutoMachine::CollecteEdgeToNFAMap(vector<unordered_set<State*>>& dfa_nfa_map, int& front, unordered_map<int, unordered_set<State*>>& edge_nfa_map,const int final_index)
	{
		//�ռ��ߵ�nfa���ϵ�ӳ��
		for(auto&& range = dfa_nfa_map[front].begin(); range != dfa_nfa_map[front].end(); ++range)
		{
			for(auto&& edge : (*range)->output)
			{
				assert(edge->type == Edge::EdgeType::Char || edge->type == Edge::EdgeType::Final );
				if(edge->type == Edge::EdgeType::Char)
				{
					auto&& index = any_cast<int>(edge->userdata);
					if(edge_nfa_map.find(index) == edge_nfa_map.end())
					{
						edge_nfa_map.insert({ index, unordered_set<State*>() });
					}
					edge_nfa_map[index].insert(edge->target);
				}
				else if(edge->type == Edge::EdgeType::Final)
				{
					if(edge_nfa_map.find(table->range_table->size()) == edge_nfa_map.end())
					{
						edge_nfa_map.insert({ final_index, unordered_set<State*>() });
					}
					edge_nfa_map[table->range_table->size()].insert(edge->target);
				}
			}
		}
		
	}
	void AutoMachine::CreatDFAStateByEdgeToNFAMap(unordered_map<int, unordered_set<State*>>& edge_nfa_map, unordered_map<unordered_set<State*>, int>& nfa_dfa_map, int& front, vector<vector<int>>& dfa_table, vector<unordered_set<State*>>& dfa_nfa_map, const int edge_sum, deque<int>& dfaqueue)
	{
		
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
		unordered_map<int, unordered_set<State*>> edge_nfa_map;
		dfa_table.emplace_back(vector<int>(edge_sum, -1));

		unordered_set<State*>temp;
		temp.insert(expression.first);
		
		
		dfa_nfa_map.emplace_back(temp/*EpsilonNFASet(*/)/*)*/;
		nfa_dfa_map.insert({ dfa_nfa_map.back(), dfa_nfa_map.size() - 1 });
		//��ʼ������
		dfaqueue.emplace_back(0);
		while(!dfaqueue.empty())
		{
			
			auto&& front = dfaqueue.front();
			if(dfa_nfa_map[front].find(expression.second) != dfa_nfa_map[front].end())
			{
				final_dfa = front;
			}
			
			CollecteEdgeToNFAMap(dfa_nfa_map, front, edge_nfa_map, final_index);

			for(auto&& key_iter = edge_nfa_map.begin(); key_iter != edge_nfa_map.end(); ++key_iter)
			{
				//��ȡ �ߵ�nfa����,Ȼ��鿴dfa���� ��û��״̬һ�µ�dfa.�еĻ�,������
				//û�еĻ�,�½�dfa�ڵ��ٽ�����,����dfa�ڵ�������
				auto&& nfaset = key_iter->second;
				//auto&& enfaset = EpsilonNFASet(nfaset);
				
				auto&& enfaset = nfaset;
				
				auto&& find_result = nfa_dfa_map.find(enfaset);
				if(find_result == nfa_dfa_map.end())
				{
					//�½ڵ�
					dfa_table.emplace_back(vector<int>(edge_sum, -1));
					dfa_nfa_map.emplace_back(enfaset);
					auto&& dfa_node = dfa_table.size() - 1;
					nfa_dfa_map.insert({ enfaset, dfa_node });
					dfa_table[front][key_iter->first] = dfa_node;
					dfaqueue.emplace_back(dfa_node);
				}
				else
				{
					dfa_table[front][key_iter->first] = find_result->second;
				}
			}
			//����̫��,�ϵ�����
		//	CreatDFAStateByEdgeToNFAMap(edge_nfa_map, nfa_dfa_map, front, dfa_table, dfa_nfa_map, edge_sum, dfaqueue);
			
			//����ߵ�ӳ��
			edge_nfa_map.clear();
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
					if(find(result.begin(), result.end(),element->target) == result.end())
					{
						result.insert(element->target);
						queue.push_back(element->target);
					}
				}
			}
			queue.pop_front();
		}
		return result ;
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
			auto&& enfaset = EpsilonNFASet(front);
			//enfaset����ӳ���
			for(auto& element : enfaset)
			{
				state_map.insert({ element, front });
			}
			//enfaset��ÿһ���߷ǿձ߽���front,front�Ŀձ�ɾ��
			vector<Edge*> new_front_output;
			for(auto& element : enfaset)
			{
				for(auto&& edge : element->output)
				{
					if(edge->type != Edge::EdgeType::Epsilon)
					{
						new_front_output.emplace_back(edge);
					}
				}
			}
			front->output = move(new_front_output);
			//����enfaset��ÿһ����

			for(auto&& edge : front->output)
			{
				if(edge->type != Edge::EdgeType::Epsilon)
				{
					if(state_map.find(edge->target) != state_map.end())
					{
						edge->target = state_map[edge->target];
					}
					else
					{
						if(sign.find(edge->target)==sign.end())
						{
							//�µ�δ������Ľڵ�
							sign.insert(edge->target);
							queue.emplace_back(edge->target);
							edge_queue.emplace_back(edge);
						}
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
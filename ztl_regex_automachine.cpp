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
	AutoMachine::StatesType AutoMachine::BuildNFA(const Ptr<Expression>& expression)
	{
		return expression->BuildNFA(this);
	}

}
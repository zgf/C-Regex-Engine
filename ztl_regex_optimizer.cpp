#include "forward.h"
#include "ztl_regex_optimizer.h"
namespace ztl
{
	void RegexOptimizer::DFS(const AutoMachine::StatesType& expression)
	{
		unordered_set<State*> marks;
		function<void(State* element)> functor;
		functor = [this,&functor, &marks](State* element)
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
		assert(marks.find(expression.second)!= marks.end());
	}
	bool RegexOptimizer::CheckPure(const AutoMachine::StatesType& expression)
	{
		unordered_set<State*> marks;
		function<void(State* element)> functor;
		bool result = true;
		functor = [this,&result, &functor, &marks](State* element)
		{
			if(marks.find(element) == marks.end())
			{
				marks.insert(element);
				for(auto&& iter : element->output)
				{
					if (iter->type != Edge::EdgeType::Char &&iter->type!= Edge::EdgeType::Final)
					{
						result = false;
						return;
					}
					functor(iter->target);
					if(result ==false)
					{
						return;
					}
				}
			}
		};
		functor(expression.first);
		return move(result);
	}
	void RegexOptimizer::NfaToDfa(AutoMachine::StatesType& expression)
	{

	}

}
#pragma once
#include "forward.h"
#include "ztl_regex_expression.h"

namespace ztl
{
	class State;
	class Edge
	{
	public:
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
			bool operator==(const LoopUserData& target)
			{
				return index == target.index && begin == target.begin && end == target.end && greedy == target.greedy;
			}
			bool operator!= (const LoopUserData& target)
			{
				return !this->operator==(target);
			}
			bool operator<(const LoopUserData& right)
			{
				if(index == right.index)
				{
					if(greedy == right.greedy)
					{
						if(begin == right.begin)
						{
							return end < right.end;
						}
						else
						{
							return begin < right.begin;
						}
					}
					else
					{
						return greedy < right.greedy;
					}
				}
				else
				{
					return index < right.index;
				}
			}
		};
	public:
		EdgeType type;
		State* srouce = 0;
		State* target = 0;
		any userdata;
		/*
		char
		这里是table index. int
		Capture
		这里是subexpression 编号和bool 是否是pure subexpress pair<int,bool>
		Loop
		这里是Loop subexpress 编号 是否是pure. pair<int,bool>
		Head userdata不需要
		Tail也不需要.
		Final也不需要
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		都是 subexpression index
		BackReference
		这里是wstring name
		NameSubexprssion
		这里是name.
		*/
	public:
		bool operator==( const Edge& right)
		{
			if(type == right.type && srouce == right.srouce&& target == right.target)
			{
				return CompareUserData(right.userdata);
			}
			else
			{
				return false;
			}
		}
		bool CompareEqualUserData(const any& right)
		{
			if(type == Edge::EdgeType::BackReference || type == Edge::EdgeType::Capture)
			{
				return any_cast<wstring>(userdata) == any_cast<wstring>(right);
			}
			else if(type == Edge::EdgeType::Loop)
			{
				return any_cast<Edge::LoopUserData>(userdata) == any_cast<Edge::LoopUserData>(right);
			}
			else if(type == EdgeType::NegativeLookahead || type == EdgeType::NegativeLookbehind || type == EdgeType::PositivetiveLookahead || type == EdgeType::PositiveLookbehind || type == EdgeType::Char)
			{
				return any_cast<int>(userdata) == any_cast<int>(right);
			}
			else
			{
				return true;
			}
		}
		bool CompareLessUserData(const any& right)
		{
			if(type == Edge::EdgeType::BackReference || type == Edge::EdgeType::Capture)
			{
				return any_cast<wstring>(userdata) < any_cast<wstring>(right);
			}
			else if(type == Edge::EdgeType::Loop)
			{
				return any_cast<Edge::LoopUserData>(userdata) < any_cast<Edge::LoopUserData>(right);
			}
			else if(type == EdgeType::NegativeLookahead || type == EdgeType::NegativeLookbehind || type == EdgeType::PositivetiveLookahead || type == EdgeType::PositiveLookbehind || type == EdgeType::Char)
			{
				return any_cast<int>(userdata) < any_cast<int>(right);
			}
			else
			{
				//undef==undef
				return false;
			}
		}
		bool CompareUserData(const any& right)
		{
			if(type == Edge::EdgeType::BackReference || type == Edge::EdgeType::Capture)
			{
				return any_cast<wstring>(userdata) == any_cast<wstring>(right);
			}
			else if(type == Edge::EdgeType::Loop)
			{
				return any_cast<Edge::LoopUserData>(userdata) == any_cast<Edge::LoopUserData>(right);
			}
			else if(type == EdgeType::NegativeLookahead || type == EdgeType::NegativeLookbehind || type == EdgeType::PositivetiveLookahead || type == EdgeType::PositiveLookbehind || type == EdgeType::Char)
			{
				return any_cast<int>(userdata)== any_cast<int>(right);
			}
			else
			{
				//undef==undef
				return false;
			}
		}
		bool operator!=(const Edge& right)
		{
			return !this->operator==(right);
		}
		//相同类型,相同userdata的放一起.
		bool operator<(const Edge& right)
		{
			if (type == right.type)
			{
				return CompareLessUserData(right.userdata);
			}
			else
			{
				return type < right.type;
			}
		}
	};
	
	//状态
	class State
	{
	public:
		vector<Edge*> input;
		vector<Edge*> output;
	};
	
	

	//自动机
	//构造非空NFA.
	class AutoMachine
	{
	public:
		using StatesType = pair < State*, State* > ;
		Ptr<Expression> ast;
		Ptr<unordered_map<wstring, StatesType>> captures;
		Ptr<vector<StatesType>>				 subexpression;//用在几个lookaround上
		Ptr<vector<CharRange>> table;
		Ptr<vector<Ptr<State>>> states;
		Ptr<vector<Ptr<Edge>>> edges;
		Ptr<unordered_map<wstring, StatesType>> macroexpression;//宏表达式
	public:
		AutoMachine() = delete;
		AutoMachine(const Ptr<vector<CharRange>>& _table) 
			:table(_table), states(make_shared<vector<Ptr<State>>>()), edges(make_shared<vector<Ptr<Edge>>>()), captures(make_shared<unordered_map<wstring, StatesType>>()), subexpression(make_shared<vector<StatesType>>()), macroexpression(make_shared <unordered_map<wstring, StatesType >> ())
		{

		}
		AutoMachine(RegexParser& parser)
			:table(nullptr), states(make_shared<vector<Ptr<State>>>()), edges(make_shared<vector<Ptr<Edge>>>()), captures(make_shared<unordered_map<wstring, StatesType>>()), subexpression(make_shared<vector<StatesType>>()), ast(nullptr), macroexpression(make_shared <unordered_map<wstring, StatesType >>())
		{
			table = parser.GetCharTable();
			ast = parser.GetExpressTree();
		}
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
		AutoMachine::StatesType NewFinalStates(StatesType& target);
		AutoMachine::StatesType NewSequenceStates(StatesType& left, StatesType& right);

		
		void ConnetWith(StatesType& target, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type = Edge::EdgeType::Epsilon);
		void ConnetWith(StatesType& target, const Edge::EdgeType& type, const any& userdata);
		void ConnetWith(State*& start, State*& end, const Edge::EdgeType& type, const any& userdata);
		AutoMachine::StatesType BuildOptimizeNFA();
	private:
		int GetTableIndex(const CharRange& target)const;
		AutoMachine::StatesType NewStates();
		State* NewOneState();

		Edge* NewEdge();
		int GetSubexpressionIndex(const StatesType& substates);
	public:
		//检查当前子图是不是pure正则的 也就是只有 char e,
		bool CheckPure(const AutoMachine::StatesType& expression);
		void DFS(const AutoMachine::StatesType& expression);
		//NFA to DFA
		void NfaToDfa(AutoMachine::StatesType& expression);
		//查找target的索引
		int Find(const State*& target);
		//优化子表达式
		void OptimizeSubexpress();
	private:
		Edge::EdgeType GetEdgeType(int index) const;
	};

	//正则自动机
	
}
namespace std
{
	template<>
	struct hash < ztl::Edge >
	{	// hash functor for Edge
	public:
		size_t operator()(const ztl::Edge& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			return (std::_Hash_seq((const unsigned char *)&_Keyval, sizeof(ztl::Edge)));
		}
	};
	template<>
	struct hash < unordered_set<ztl::State*> >
	{	// hash functor for Edge
	public:
		//这个hash函数随便写的- -...目测效率不是太高- -....
		size_t operator()(const unordered_set<ztl::State*>& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			vector<size_t> result(_Keyval.size());
			size_t i = 0;
			for(auto&& iter: _Keyval)
			{
				result[i++] = _Hash_seq((const unsigned char *)(iter), sizeof(ztl::State*));
			}
			return	accumulate(result.begin(), result.end(), 0);
		}
	};
}
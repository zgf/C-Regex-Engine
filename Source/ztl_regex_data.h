#pragma once
#include "forward.h"
namespace ztl
{
	//////////////////////////////////////////////////////////////////////////
	//  正则需要的数据结构声明
	//////////////////////////////////////////////////////////////////////////
	class CharRange
	{
	public:
		int min;
		int max;
	public:
		CharRange() :min(-1), max(-1)
		{
		}
		CharRange(const int& _min, const int& _max) : min(_min), max(_max)
		{
		}
		~CharRange() = default;
		bool operator== (const CharRange& target)const
		{
			return this->min == target.min && this->max == target.max;
		}
		bool operator!= (const CharRange& target)const
		{
			return !(*this == target);
		}
		bool operator<(const CharRange& target)
		{
			if(min == target.min)
			{
				return max < target.max;
			}
			else
			{
				return min < target.min;
			}
		}
	};
	enum class TokenType
	{
		//LexToken
		NormalChar,
		BackReference,
		AnonymityBackReference,
		Named,
		Number,
		InFinite,
		LoopBegin,
		LoopEndGreedy,
		LoopEnd,
		ChoseLoop,
		ChoseLoopGreedy,
		PositiveLoop,
		PositiveLoopGreedy,
		KleeneLoop,
		KleeneLoopGreedy,
		StringHead,
		StringTail,
		CharSet,
		CharSetReverse,
		CharSetEnd,
		CharSetW,
		CharSetw,
		CharSetS,
		CharSets,
		CharSetD,
		CharSetd,
		PositionB,
		Positionb,
		//通配符,匹配非\n
		GeneralMatch,
		//匹配所有字符
		MatchAllSymbol,
		Component,
		AnonymityCaptureBegin,
		CaptureBegin,
		CaptureEnd,
		NoneCapture,
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		Alternation,
		//新功能
		RegexMacro,//(?#<name>expression)
		MacroReference,//$<name>
		//匹配行开始
		LineBegin,
		//匹配行结束
		LineEnd,

		//ParserToken
		Unit,
		Factor,
		Express,
		Loop,
		Alter,
		CaptureRight,
	};
	class RegexToken
	{
	public:
		TokenType type;
		CharRange position;
	public:
		RegexToken() = default;
		~RegexToken() = default;
		RegexToken(const TokenType& _type, const CharRange& _position) :type(_type), position(_position)
		{
		}
		RegexToken(const TokenType& _type) :type(_type)
		{
		}
		RegexToken(const TokenType& _type, const int& _min, const int& _max) : type(_type), position(_min, _max)
		{
		}
		bool operator== (const RegexToken& target)const
		{
			return this->type == target.type && this->position == target.position;
		}
		bool operator!= (const RegexToken& target)const
		{
			return !(*this == target);
		}
	};
	enum class RegexControl
	{
		//MatchAll,//匹配全部才算成功
		//MatchPartical,//部分匹配算成功
		ExplicitCapture,//不使用捕获组功能
		IgnoreCase,//大小写不敏感的匹配
		Multiline,// $^ 匹配行结尾和开头
		RightToLeft,//
		Singleline,//
		OnlyMatch,//仅匹配,不捕获
	};
	class CharTable
	{
	public:
		Ptr<vector<CharRange>> range_table;//字母范围表
		Ptr<vector<unsigned short>> char_table;//总的字母表
		int GetTableIndex(const int& target)const
		{
			return (*char_table)[target];
		}

	};
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
			//Loop, Loop被展开成JUmp E Char Edge
			Char,
			Head,
			Tail,
			PositivetiveLookahead,
			NegativeLookahead,
			PositiveLookbehind,
			NegativeLookbehind,
			Final, //边后面是终结状态
			AnonymityCapture,
			AnonymityBackReference,
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
		};
	public:
		EdgeType type;
		//State* srouce = 0;
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
	};

	//状态
	class State
	{
	public:
		//vector<Edge*> input;
		vector<Edge*> output;
	};

	class DFA
	{
	public:
		vector<vector<int>> dfa;
		int					finalset;
	};
}
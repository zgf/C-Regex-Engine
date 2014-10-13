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
			if (min == target.min)
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
		Named,
		Number,
		InFinite,
		LoopBegin,
		LoopEndGreedy,
		LoopEnd,
		Comma,
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
		MatchAllSymbol,
		Component,
		CaptureBegin,
		CaptureEnd,
		NoneCapture,
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		LookaheadEnd,
		LookbehindEnd,
		Alternation,

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
}
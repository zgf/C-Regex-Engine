#pragma once
#include "forward.h"
namespace ztl
{
	//////////////////////////////////////////////////////////////////////////
	//  ������Ҫ�����ݽṹ����
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
		//ͨ���,ƥ���\n
		GeneralMatch,
		//ƥ�������ַ�
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
		//�¹���
		RegexMacro,//(?#<name>expression)
		MacroReference,//$<name>

		//ƥ���п�ʼ
		LineBegin,
		//ƥ���н���
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
		//MatchAll,//ƥ��ȫ������ɹ�
		//MatchPartical,//����ƥ����ɹ�
		ExplicitCapture,//��ʹ�ò����鹦��
		IgnoreCase,//��Сд�����е�ƥ��
		Multiline,// $^ ƥ���н�β�Ϳ�ͷ
		RightToLeft,//
		Singleline,//
		OnlyMatch,//��ƥ��,������
	};
	class CharTable
	{
	public:
		Ptr<vector<CharRange>> range_table;//��ĸ��Χ��
		Ptr<vector<int>> char_table;//�ܵ���ĸ��
	};
	class State;
	class Edge
	{
	public:
		//��
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
			Final, //�ߺ������ս�״̬
			AnonymityCapture,
			AnonymityBackReference,
			//MarcoReference,NFA�׶α���չ
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
		������table index. int
		Capture
		������subexpression ��ź�bool �Ƿ���pure subexpress pair<int,bool>
		Loop
		������Loop subexpress ��� �Ƿ���pure. pair<int,bool>
		Head userdata����Ҫ
		TailҲ����Ҫ.
		FinalҲ����Ҫ
		PositivetiveLookahead,
		NegativeLookahead,
		PositiveLookbehind,
		NegativeLookbehind,
		���� subexpression index
		BackReference
		������wstring name
		NameSubexprssion
		������name.
		*/
	public:
		bool operator==(const Edge& right)
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
				return any_cast<int>(userdata) == any_cast<int>(right);
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
		//��ͬ����,��ͬuserdata�ķ�һ��.
		bool operator<(const Edge& right)
		{
			if(type == right.type)
			{
				return CompareLessUserData(right.userdata);
			}
			else
			{
				return type < right.type;
			}
		}
	};

	//״̬
	class State
	{
	public:
		vector<Edge*> input;
		vector<Edge*> output;
	};
}
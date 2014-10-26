#pragma once
#include "ztl_regex_expression.h"
//手写正则表达式语法树 用来测试
namespace ztl
{
	class RegexParseTreeWriter
	{
	public:
		Ptr<Expression> expression;
	public:
		RegexParseTreeWriter() = default;
		RegexParseTreeWriter(const Ptr<Expression>& target) :expression(target)
		{
		}
	public:
		friend RegexParseTreeWriter operator+(const RegexParseTreeWriter& left, const RegexParseTreeWriter& right)
		{
			return make_shared<SequenceExpression>(left.expression, right.expression);
		}
		friend RegexParseTreeWriter operator|(const RegexParseTreeWriter& left, const RegexParseTreeWriter& right)
		{
			return make_shared<AlternationExpression>(left.expression, right.expression);
		}
		RegexParseTreeWriter LoopCreator(int min, int max, bool greedy)
		{
			return make_shared<LoopExpression>(expression, min, max, greedy);
		}
	};
	RegexParseTreeWriter CharSetCreator(bool reverse, const vector<CharRange>&range)
	{
		return make_shared<CharSetExpression>(reverse, range);
	}
	RegexParseTreeWriter CharSetw()
	{
		return CharSetCreator(false, { { L'a', L'a' }, { L'b', L'y' }, { L'z', L'z' }, { L'A', L'A' }, { L'B', L'Y' }, { L'Z', L'Z' }, { '0', '0' }, { '1', '8' }, { '9', '9' }, { '_', '_' }, { 0x4E00, 0x4E00 }, { 0x4E01, 0x9FA4 }, { 0x9FA5, 0x9FA5 }, { 0xF900, 0xF900 }, { 0xF901, 0xFA2D - 1 }, { 0xFA2D, 0xFA2D } }).expression;
	}
	RegexParseTreeWriter CharSetW()
	{
		return CharSetCreator(true, { { L'a', L'a' }, { L'b', L'y' }, { L'z', L'z' }, { L'A', L'A' }, { L'B', L'Y' }, { L'Z', L'Z' }, { '0', '0' }, { '1', '8' }, { '9', '9' }, { '_', '_' }, { 0x4E00, 0x4E00 }, { 0x4E01, 0x9FA4 }, { 0x9FA5, 0x9FA5 }, { 0xF900, 0xF900 }, { 0xF901, 0xFA2D - 1 }, { 0xFA2D, 0xFA2D } }).expression;
	}
	RegexParseTreeWriter NumberCreator(bool reverse)
	{
		vector<CharRange> range({ { L'0', L'9' } });
		return make_shared<CharSetExpression>(reverse, range);
	}
	RegexParseTreeWriter NormalCharCreator(const wstring& character)
	{
		assert(character.size() == 1);
		return make_shared<NormalCharExpression>(CharRange(character[0], character[0]));
	}
	RegexParseTreeWriter One(const wstring& character)
	{
		return NormalCharCreator(character);
	}
	RegexParseTreeWriter One(const wchar_t& character)
	{
		wstring temp;
		temp = character;
		return NormalCharCreator(temp);
	}
	RegexParseTreeWriter Capture( const RegexParseTreeWriter& expression)
	{
		return make_shared<AnonymityCaptureExpression>(0,expression.expression);
	}
	RegexParseTreeWriter NamedCapture(const wstring name, const RegexParseTreeWriter& expression)
	{
		return make_shared<CaptureExpression>(name, expression.expression);
	}
	RegexParseTreeWriter NoneCapture(const RegexParseTreeWriter& expression)
	{
		return make_shared<NoneCaptureExpression>(expression.expression);
	}
	RegexParseTreeWriter BackReference(const wstring name)
	{
		return make_shared<BackReferenceExpression>(name);
	}
	RegexParseTreeWriter AnonymityBackReference(const int index)
	{
		return make_shared<AnonymityBackReferenceExpression>(index);
	}
	RegexParseTreeWriter StringTail()
	{
		return make_shared<EndExpression>();
	}
	RegexParseTreeWriter StringHead()
	{
		return make_shared<BeginExpression>();
	}
	RegexParseTreeWriter PositiveLookbehind(const RegexParseTreeWriter& expression)
	{
		return make_shared<PositiveLookbehindExpression>(expression.expression);
	}
	RegexParseTreeWriter NegativeLookbehind(const RegexParseTreeWriter& expression)
	{
		return make_shared<NegativeLookbehindExpression>(expression.expression);
	}
	RegexParseTreeWriter NegativeLookahead(const RegexParseTreeWriter& expression)
	{
		return make_shared<NegativeLookaheadExpression>(expression.expression);
	}
	RegexParseTreeWriter PositivetiveLookahead(const RegexParseTreeWriter& expression)
	{
		return make_shared<PositivetiveLookaheadExpression>(expression.expression);
	}
}
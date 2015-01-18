#pragma once
#include "ztl_regex_expression.h"
//��д������ʽ�﷨�� ��������
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
		
		RegexParseTreeWriter RegexParseTreeWriter::LoopCreator(int min, int max, bool greedy);
	};
	
}
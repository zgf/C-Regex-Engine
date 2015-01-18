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
		
		RegexParseTreeWriter RegexParseTreeWriter::LoopCreator(int min, int max, bool greedy);
	};
	
}
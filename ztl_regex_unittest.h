#pragma once
#include "ztl_regex_writer.h"

namespace ztl
{
	void TestLexer()
	{
		auto ExpectEq = [](const wstring input, const vector<TokenType>& expect)->bool
		{
			RegexLex lexer(input);
			lexer.ParsingPattern();
			auto& result = lexer.GetTokens();
			int a = 0;
			return equal(result->begin(), result->end(), expect.begin(), [](auto&&left, auto&&right)->bool
			{
				return left.type == right;
			});
		};
		auto ExpectEqEx = [](const wstring input, const vector<RegexToken>& expect)->bool
		{
			RegexLex lexer(input);
			lexer.ParsingPattern();
			auto& result = lexer.GetTokens();
			int a = 0;
			return equal(result->begin(), result->end(), expect.begin());
		};
		//one char
		assert(ExpectEq(L"a", { TokenType::NormalChar }));
		//选择
		assert(ExpectEq(L"|", { TokenType::Alternation }));
		//注释
		assert(ExpectEq(L"(#zgf)", {}));
		//test 重复
		assert(ExpectEq(L"{0,2}", { TokenType::LoopBegin, TokenType::Number, /* { TokenType::Comma }, */ TokenType::Number, TokenType::LoopEndGreedy }));
		assert(ExpectEqEx(L"{0,4}?", { { TokenType::LoopBegin }, { TokenType::Number, 1, 2 },/* { TokenType::Comma }, */ { TokenType::Number, 3, 4 }, { TokenType::LoopEnd } }));
		assert(ExpectEqEx(L"{0,}?", { { TokenType::LoopBegin }, { TokenType::Number, 1, 2 },/* { TokenType::Comma }, */ { TokenType::InFinite }, { TokenType::LoopEnd } }));
		assert(ExpectEqEx(L"{0,}", { { TokenType::LoopBegin }, { TokenType::Number, 1, 2 },/* { TokenType::Comma }, */ { TokenType::InFinite }, { TokenType::LoopEndGreedy } }));
		//test 预定义重复
		assert(ExpectEq(L"*", { TokenType::KleeneLoopGreedy }));
		assert(ExpectEq(L"*?", { TokenType::KleeneLoop }));
		assert(ExpectEq(L"+", { TokenType::PositiveLoopGreedy }));
		assert(ExpectEq(L"+?", { TokenType::PositiveLoop }));
		assert(ExpectEq(L"?", { TokenType::ChoseLoopGreedy }));
		assert(ExpectEq(L"??", { TokenType::ChoseLoop }));

		assert(ExpectEq(L".", { TokenType::MatchAllSymbol }));
		assert(ExpectEq(L"\\.", { TokenType::NormalChar }));

		//test 捕获组
		assert(ExpectEq(L"(a)", { TokenType::CaptureBegin, TokenType::NormalChar, TokenType::CaptureEnd }));
		assert(ExpectEqEx(L"(<bbb>a)", { { TokenType::CaptureBegin }, { TokenType::Named, 2, 5 }, { TokenType::NormalChar, 6, 7 }, { TokenType::CaptureEnd } }));
		assert(ExpectEq(L"(?:a)", { TokenType::NoneCapture, TokenType::NormalChar, TokenType::CaptureEnd }));

		//test零宽断言
		assert(ExpectEq(L"(?<=ac)", { TokenType::PositiveLookbehind, TokenType::NormalChar, TokenType::NormalChar, TokenType::LookbehindEnd }));
		assert(ExpectEq(L"(?<!a)", { TokenType::NegativeLookbehind, TokenType::NormalChar, TokenType::LookbehindEnd }));
		assert(ExpectEq(L"(?=a)", { TokenType::PositivetiveLookahead, TokenType::NormalChar, TokenType::LookaheadEnd }));
		assert(ExpectEq(L"(?!a)", { TokenType::NegativeLookahead, TokenType::NormalChar, TokenType::LookaheadEnd }));
		//test 串首尾
		assert(ExpectEq(L"$", { TokenType::StringTail }));
		assert(ExpectEq(L"^", { TokenType::StringHead }));

		//test 字符集合
		assert(ExpectEq(L"[bc]", { TokenType::CharSet, TokenType::NormalChar, TokenType::NormalChar, TokenType::CharSetEnd }));
		assert(ExpectEq(L"[^bc]", { TokenType::CharSetReverse, TokenType::NormalChar, TokenType::NormalChar, TokenType::CharSetEnd }));
		assert(ExpectEq(L"[^_\\.]", { TokenType::CharSetReverse, TokenType::NormalChar, TokenType::NormalChar, TokenType::CharSetEnd }));

		//test 预定义字符集合
		assert(ExpectEq(L"\\s", { TokenType::CharSets }));
		assert(ExpectEq(L"\\S", { TokenType::CharSetS }));
		assert(ExpectEq(L"\\d", { TokenType::CharSetd }));
		assert(ExpectEq(L"\\D", { TokenType::CharSetD }));
		assert(ExpectEq(L"\\W", { TokenType::CharSetW }));
		assert(ExpectEq(L"\\w", { TokenType::CharSetw }));

		//后向引用
		assert(ExpectEq(L"(<one>ee)\\k<one>", { TokenType::CaptureBegin, TokenType::Named, TokenType::NormalChar, TokenType::NormalChar, TokenType::CaptureEnd, TokenType::BackReference, TokenType::Named }));
		assert(ExpectEq(L"(ee)\\1", { TokenType::CaptureBegin, TokenType::NormalChar, TokenType::NormalChar, TokenType::CaptureEnd, TokenType::BackReference, TokenType::Number }));
	}
	void TestParserUnCrash()
	{
		auto ExpectUnCrash = [](const wstring& input)
		{
			ztl::RegexLex lexer(input);
			lexer.ParsingPattern();
			ztl::RegexParser parser(input, lexer.GetTokens());
			parser.RegexParsing();
		};
		//普通字符 连接
		ExpectUnCrash(L"a");
		ExpectUnCrash(L"abv");

		//重复
		ExpectUnCrash(L"a{1,}");
		ExpectUnCrash(L"a{1,2}");
		ExpectUnCrash(L"a{1}");
		ExpectUnCrash(L"a{1,}?");
		ExpectUnCrash(L"a{1,2}?");
		ExpectUnCrash(L"a{1}?");
		ExpectUnCrash(L"a*");
		ExpectUnCrash(L"a*?");
		ExpectUnCrash(L"a+?");
		ExpectUnCrash(L"a+");
		ExpectUnCrash(L"a?");
		ExpectUnCrash(L"a??");
		
		//捕获组
		ExpectUnCrash(L"(avs)");
		ExpectUnCrash(L"(<one>a)");

		//非捕获组
		ExpectUnCrash(L"(?:avs)");

		//字符集合
		ExpectUnCrash(L"[avs]");
		ExpectUnCrash(L"[^avs]");
		ExpectUnCrash(L"[^avs]{3,4}");

		//预定义字符
		ExpectUnCrash(L"\\s");
		ExpectUnCrash(L"\\S");
		ExpectUnCrash(L"\\d");
		ExpectUnCrash(L"\\D");
		ExpectUnCrash(L"\\W");
		ExpectUnCrash(L"\\w");
		//后向引用
		ExpectUnCrash(L"(aa)\\1");
		ExpectUnCrash(L"(<one>aa)\\k<one>");


		//零宽断言
		ExpectUnCrash(L"(?<=aa)");
		ExpectUnCrash(L"(?<!aa)");
		ExpectUnCrash(L"(?=aa)");
		ExpectUnCrash(L"(?!aa)");

		//串首尾
		ExpectUnCrash(L"(aa)$");
		ExpectUnCrash(L"^(?<!aa)");

		//选择
		ExpectUnCrash(L"^(?<!aa)|(as[ad\\.]c(a|b(?<=aa)))");

	}

	void TestParserTree()
	{
		auto TestCase = [](const wstring input, RegexParseTreeWriter& expect)
		{
			ztl::RegexLex lexer(input);
			lexer.ParsingPattern();
			ztl::RegexParser parser(input, lexer.GetTokens());
			parser.RegexParsing();
			auto&& expression = parser.GetExpressTree();
			auto&& table = expression->GetCharSetTable();
			expression->SetTreeCharSetOrthogonal(table);
			auto result = expression->IsEqual(expect.expression);
			return result;
		};
		assert(TestCase(L"a", One(L'a')));
		assert(TestCase(L"abc", One(L'a')+(One(L'b')+One(L'c'))));
		
		assert(TestCase(L"a{1,}", One(L'a').LoopCreator(1,-1,true)));
		assert(TestCase(L"(<one>a)", Capture(L"one",One(L'a'))));
		assert(TestCase(L"(aa)", Capture(L"1", One(L'a')+One('a'))));

		assert(TestCase(L"(?:abc)", NoneCapture(One(L'a') + (One(L'b') + One(L'c')))));
		assert(TestCase(L"[^a-g]c", CharSetCreator(true, { { 'a', 'a' }, {'b','b'}, { 'c', 'c' }, { 'd', 'f' }, { 'g', 'g' } }) + One('c')));
		auto name_expression = Capture(L"1",One('a'));
		assert(TestCase(L"(a)\\1", name_expression + BackReference(L"1")));
		auto named_expression = Capture(L"one", One('a'));

		assert(TestCase(L"(<one>a)\\k<one>", named_expression + BackReference(L"one")));
		assert(TestCase(L"\\d", CharSetCreator(false, { { '0', '0' }, { '1', '8' }, {'9','9'} })));
		assert(TestCase(L"\\w", CharSetw()));
		assert(TestCase(L"\\W", CharSetW()));
		assert(TestCase(L".", CharSetCreator(false, { { 0, 0 }, { 1, 65534 }, { 65535, 65535 } })));
		//所以\b == ((?<=\\w)(?=\\W))|((?<=\\W)(?=\\w))

		assert(TestCase(L"\\b", (PositiveLookbehind(CharSetw()) + PositivetiveLookahead(CharSetW())) | (PositiveLookbehind(CharSetW()) + PositivetiveLookahead(CharSetw()))));
		assert(TestCase(L"\\B", (PositiveLookbehind(CharSetw()) + PositivetiveLookahead(CharSetw())) | (PositiveLookbehind(CharSetW()) + PositivetiveLookahead(CharSetW()))));

		//串首尾
		assert(TestCase(L"(a)$", Capture(L"1",One('a'))+StringTail()));
		auto aa = One('a') + One('a');
		//零宽断言
		assert(TestCase(L"^(?<!aa)", StringHead()+NegativeLookbehind(aa)));
		assert(TestCase(L"^(?<=aa)", StringHead() + PositiveLookbehind(aa)));
		assert(TestCase(L"^(?!aa)", StringHead() + NegativeLookahead(aa)));
		assert(TestCase(L"^(?=aa)", StringHead() + PositivetiveLookahead(aa)));
		//选择
		//|a(?<=aa)
		assert(TestCase(L"(?<!av)|(zh[^a-c]{3,4}(a|f(?<=sy)))", NegativeLookbehind(One('a') + One('v')) | Capture(L"1",
			One('z') + (One('h') + (
			CharSetCreator(true, { { 'a', 'a' }, { 'b', 'b' }, { 'c', 'c' } 
	}).LoopCreator(3, 4, true) +
			Capture(L"2", One('a') | 
			(One('f') + PositiveLookbehind(One('s') + One('y')))))))));
	
		

	}

	void TestAllComponent()
	{
		TestLexer();
		TestParserUnCrash();
		TestParserTree();
	}
}
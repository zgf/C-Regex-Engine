#pragma once
#include "forward.h"
#include "ztl_regex_writer.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	void TestLexer()
	{
		auto ExpectEq = [](const wstring input, const vector<TokenType>& expect)->bool
		{
			RegexLex lexer(input);
			lexer.ParsingPattern();
			auto&& result = lexer.GetTokens();
			return equal(result->begin(), result->end(), expect.begin(), [](const RegexToken& left, const TokenType& right)->bool
			{
				return left.type == right;
			});
		};
		auto ExpectEqEx = [](const wstring input, const vector<RegexToken>& expect)->bool
		{
			RegexLex lexer(input);
			lexer.ParsingPattern();
			auto&& result = lexer.GetTokens();
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

		assert(ExpectEq(L".", { TokenType::GeneralMatch }));
		assert(ExpectEq(L"\\.", { TokenType::NormalChar }));

		//test 捕获组
		assert(ExpectEq(L"(a)", { TokenType::CaptureBegin, TokenType::NormalChar, TokenType::CaptureEnd }));
		assert(ExpectEqEx(L"(<bbb>a)", { { TokenType::CaptureBegin }, { TokenType::Named, 2, 5 }, { TokenType::NormalChar, 6, 7 }, { TokenType::CaptureEnd } }));
		assert(ExpectEq(L"(?:a)", { TokenType::NoneCapture, TokenType::NormalChar, TokenType::CaptureEnd }));
		assert(ExpectEqEx(L"(?#<bbb>a)$<bbb>", { { TokenType::RegexMacro }, { TokenType::Named, 4, 7 }, { TokenType::NormalChar, 8, 9 }, { TokenType::CaptureEnd }, { TokenType::MacroReference }, { TokenType::Named, 12, 15 } }));

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
		assert(ExpectEq(L"(ee)\\1", { TokenType::CaptureBegin, TokenType::NormalChar, TokenType::NormalChar, TokenType::CaptureEnd, TokenType::AnonymityBackReference, TokenType::Number }));
	}
	void TestParserUnCrash()
	{
		auto ExpectUnCrash = [](const wstring& input)
		{
			ztl::RegexLex lexer(input);
			lexer.ParsingPattern();
			ztl::RegexParser parser(lexer);
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

	string ws2s(const wstring& ws)
	{
		string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";

		setlocale(LC_ALL, "chs");

		const wchar_t* _Source = ws.c_str();
		size_t _Dsize = 2 * ws.size() + 1;
		char *_Dest = new char[_Dsize];
		memset(_Dest, 0, _Dsize);
		wcstombs(_Dest, _Source, _Dsize);
		string result = _Dest;
		delete[]_Dest;

		setlocale(LC_ALL, curLocale.c_str());

		return result;
	}

	void TestParserTree()
	{
		auto TestCase = [](const wstring input, RegexParseTreeWriter&& expect)
		{
			ztl::RegexLex lexer(input);
			lexer.ParsingPattern();
			ztl::RegexParser parser(lexer);
			parser.RegexParsing();
			auto&& expression = parser.GetExpressTree();
			auto result = expression->IsEqual(expect.expression);
			return result;
		};
		assert(TestCase(L"a", One(L'a')));
		assert(TestCase(L"abc", One(L'a') + (One(L'b') + One(L'c'))));

		assert(TestCase(L"a{1,}", One(L'a').LoopCreator(1, -1, true)));
		assert(TestCase(L"(<one>a)", Capture(L"one", One(L'a'))));
		assert(TestCase(L"(aa)", Capture(L"", One(L'a') + One('a'))));

		assert(TestCase(L"(?:abc)", NoneCapture(One(L'a') + (One(L'b') + One(L'c')))));
		assert(TestCase(L"[^a-g]c", CharSetCreator(true, { { 'a', 'a' }, { 'b', 'b' }, { 'c', 'c' }, { 'd', 'f' }, { 'g', 'g' } }) + One('c')));
		auto name_expression = Capture(L"", One('a'));
		assert(TestCase(L"(a)\\1", name_expression + AnonymityBackReference(1)));
		auto named_expression = Capture(L"one", One('a'));

		assert(TestCase(L"(<one>a)\\k<one>", named_expression + BackReference(L"one")));
		assert(TestCase(L"\\d", CharSetCreator(false, { { '0', '0' }, { '1', '8' }, { '9', '9' } })));
		assert(TestCase(L"\\w", CharSetw()));
		assert(TestCase(L"\\W", CharSetW()));
		assert(TestCase(L".", CharSetCreator(true, { { L'\n', L'\n' } })));
		//所以\b == ((?<=\\w)(?=\\W))|((?<=\\W)(?=\\w))

		assert(TestCase(L"\\b", (PositiveLookbehind(CharSetw()) + PositivetiveLookahead(CharSetW())) | (PositiveLookbehind(CharSetW()) + PositivetiveLookahead(CharSetw()))));
		assert(TestCase(L"\\B", (PositiveLookbehind(CharSetw()) + PositivetiveLookahead(CharSetw())) | (PositiveLookbehind(CharSetW()) + PositivetiveLookahead(CharSetW()))));

		//串首尾
		assert(TestCase(L"(a)$", Capture(L"", One('a')) + StringTail()));
		auto aa = One('a') + One('a');
		//零宽断言
		assert(TestCase(L"^(?<!aa)", StringHead() + NegativeLookbehind(aa)));
		assert(TestCase(L"^(?<=aa)", StringHead() + PositiveLookbehind(aa)));
		assert(TestCase(L"^(?!aa)", StringHead() + NegativeLookahead(aa)));
		assert(TestCase(L"^(?=aa)", StringHead() + PositivetiveLookahead(aa)));
		////选择
		////|a(?<=aa)
		assert(TestCase(L"(?<!av)|(zh[^a-c]{3,4}(a|f(?<=sy)))", NegativeLookbehind(One('a') + One('v')) | Capture(L"",
			One('z') + (One('h') + (
			CharSetCreator(true, { { 'a', 'a' }, { 'b', 'b' }, { 'c', 'c' }
			}).LoopCreator(3, 4, true) +
			Capture(L"", One('a') |
			(One('f') + PositiveLookbehind(One('s') + One('y')))))))));
	}
	void PrintENFA(ofstream& output, unordered_map<int, std::string>& signmap, const wstring& input)
	{
		static int cast_index = 0;
		output << "TestCaseIndex:" << cast_index++ << endl;
		output << "Input:" << ws2s(input) << endl;
		RegexLex lexer(input);
		lexer.ParsingPattern();
		RegexParser parser(lexer);
		parser.RegexParsing();
		auto&& machine = make_shared<AutoMachine>(parser);
		auto&& nfa = machine->BuildOptimizeNFA();
		auto& target = *machine;
		vector<bool> marks(target.states->size());
		auto&& state_list = target.states;
		function<void(int current, const State* element)> functor;
		auto find_functor = [&state_list](State* target)->int
		{
			for(size_t index = 0; index < state_list->size(); index++)
			{
				auto&& may_target = state_list->at(index).get();
				if(may_target == target)
				{
					return index;
				}
			}
			throw std::exception("can't find target!");
		};
		functor = [&functor, &output, &find_functor, &machine, &marks, &signmap, &state_list](int current, const State* element)
		{
			if(marks[current] == false)
			{
				marks[current] = true;
				//	wcout << "Current Node Address:" <<element << endl;

				for(auto&& iter : element->output)
				{
					/*	output << "		Edge Type:";
						output << signmap[(int)(iter->type)].c_str() << endl;*/
					//wcout << "	Edge Target Node Address:" << iter->target << endl;

					if((int)(iter->type) == 1 || (int)(iter->type) == 2)
					{
						output << "Current Node index:" << current << endl;
						output << "		Edge Type:";
						output << signmap[(int)(iter->type)].c_str() << endl;
						auto name = any_cast<wstring>(iter->userdata);
						auto& captures = *machine->captures;
						auto subindex = find_functor(captures[name].first);
						output << "Find Subexpression:" << endl;

						functor(subindex, captures[name].first);
						output << " Subexpression End" << endl;
					}
					else if((int)(iter->type) == 3)
					{
						output << "Current Node index:" << current << endl;
						output << "		Edge Type:";
						output << signmap[(int)(iter->type)].c_str() << endl;
						output << "Find Subexpression:" << endl;

						auto data = any_cast<Edge::LoopUserData>(iter->userdata);
						auto& subexpression = *machine->subexpression;
						auto subindex = find_functor(subexpression[data.index].first);
						functor(subindex, subexpression[data.index].first);
						output << " Subexpression End" << endl;
					}
					else if((int)(iter->type) == 7 || (int)(iter->type) == 8 || (int)(iter->type) == 9 || (int)(iter->type) == 10)
					{
						output << "Current Node index:" << current << endl;
						output << "		Edge Type:";
						output << signmap[(int)(iter->type)].c_str() << endl;
						output << "Find Subexpression:" << endl;

						auto loop_index = any_cast<int>(iter->userdata);
						auto& subexpression = *machine->subexpression;
						auto subindex = find_functor(subexpression[loop_index].first);
						functor(subindex, subexpression[loop_index].first);
						output << " Subexpression End" << endl;
					}
					output << "Current Node index:" << current << endl;
					output << "		Edge Type:";
					output << signmap[(int)(iter->type)].c_str() << endl;
					auto index = find_functor(iter->target);
					output << "		Edge Target Node Index:" << index << endl;
					functor(index, iter->target);
				}
			}
		};
		functor(find_functor(nfa.first)
			, nfa.first);
		output << "////////////////////////////////////////////////////////////////" << endl;
	}
	void TestENFA()
	{
		ofstream output("data.txt");
		unordered_map<int, std::string> signmap;
		signmap.insert({ 0, "Epsilon" });
		signmap.insert({ 1, "Capture" });
		signmap.insert({ 2, "BackReference" });
		signmap.insert({ 3, "Loop" });
		signmap.insert({ 4, "Char" });
		signmap.insert({ 5, "Head" });
		signmap.insert({ 6, "Tail" });
		signmap.insert({ 7, "PositivetiveLookahead" });
		signmap.insert({ 8, "NegativeLookahead" });
		signmap.insert({ 9, "PositiveLookbehind" });
		signmap.insert({ 10, "NegativeLookbehind" });
		signmap.insert({ 11, "Final" }); //边后面是终结状态
		PrintENFA(output, signmap, L"a");
		PrintENFA(output, signmap, L"ab");
		PrintENFA(output, signmap, L"a|a");
		PrintENFA(output, signmap, L"a|b");
		PrintENFA(output, signmap, L"(<one>a)\\k<one>");
		PrintENFA(output, signmap, L"$(?<=aa)");
		PrintENFA(output, signmap, L"(?!aa)");

		PrintENFA(output, signmap, L"^(?=aa)");
		PrintENFA(output, signmap, L"(?<!av)");
		PrintENFA(output, signmap, L"zh[^a-c]");
		PrintENFA(output, signmap, L"a|f(?<=sy)");
		PrintENFA(output, signmap, L"(^(?<!av))|(zh[^a-c]{3,4}(a|f(?<=sy)))");
		output.close();
	}
	void TestOptimize()
	{
		vector<wstring> TestList = {
			L"a(?:ab|ds)dd",
			L"a",
			L"ab",
			L"a|b",
			L"(<one>a)\\k<one>",
			L"(as(ad|bc)|fd)",
			L"$(?<=aa)", L"^(?=aa)",
			L"(?<!av)", L"zh[^a-c]",
			L"a|f(?<=sy)",
			L"(^(?<!av))|(zh[^a-c]{3,4}(a|f(?<=sy)))",
		};
		auto TestCase = [](wstring input)
		{
			RegexLex lexer(input);
			lexer.ParsingPattern();
			RegexParser parser(lexer);
			parser.RegexParsing();
			auto&& machine = make_shared<AutoMachine>(parser);
			/*auto&& nfa =*/ machine->BuildOptimizeNFA();
		};
		for(auto&& iter : TestList)
		{
			TestCase(iter);
		}
	}
	void TestAllComponent()
	{
		TestLexer();
		TestParserUnCrash();
		TestParserTree();
		//TestENFA();
		//TestOptimize();
	}
}
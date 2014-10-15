#pragma once
#include "ztl_regex_lex.h"
#include "ztl_regex_expression.h"
/*
输入: tokenlist
输出: expression tree

*/
/*
文法:

Alert = Unit "Alternation" Alert | Unit
Unit  = Express Unit | Express
Express = Factor Loop | Factor
Loop = “LoopBegin” |"ChoseLoop" | "ChoseLoopGreedy" | "PositiveLoop" | "PositiveLoopGreedy" | "KleeneLoop" | "KleeneLoopGreedy";

Factor	
= “CaptureBegin”  CaptureRight
= "RegexMacro"	 CaptureRight
= "NoneCapture"           Alert	"CaptureEnd"
= "PositivetiveLookahead" Alert "CaptureEnd"
= "NegativeLookahead"     Alert	"CaptureEnd"
= "PositiveLookbehind"    Alert	"CaptureEnd"
= "NegativeLookbehind"    Alert	"CaptureEnd"
= "StringHead"
= "StringTail"
= "Backreference"
= "CharSet"
= "NormalChar"
= "LineBegin"
= "LineEnd"
= "MatchAllSymbol"
= "GeneralMatch"
= "MacroReference"
CaptureRight = "Named"  Alert "CaptureEnd" |Alert  "CaptureEnd"
*/
/*
需要的修改
添加总字母表 已添加
字符集正规化算法 已修改
修改文法
设置control
*/
namespace ztl
{
	
	class RegexParser
	{
		using FirstMapType = unordered_map<TokenType, Ptr<unordered_set<TokenType>>>;
		using ActionType = unordered_map < TokenType, function<Ptr<Expression>(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)> >;
		using LoopActionType = unordered_map < TokenType, function<Ptr<Expression>(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)> >;
	private:
		static FirstMapType first_map;
		Ptr<vector<RegexToken>> tokens;
		Ptr<Expression>			expression;
		static ActionType actions;
		static LoopActionType loop_actions;
		Ptr<CharTable> char_table;
		Ptr<vector<RegexControl>> optional;
		wstring pattern;
	public:
		RegexParser() = delete;
		RegexParser(const RegexLex& lexer)
			:tokens(lexer.GetTokens()), pattern(lexer.GetRawString()), expression(nullptr), char_table(make_shared<CharTable>())
		{
		}
		~RegexParser() = default;
		void RegexParsing();
		Ptr<vector<int>> CreatWCharTable(const Ptr<vector<CharRange>>& table);
		Ptr<Expression> GetExpressTree()const
		{
			return expression;
		}
		Ptr<vector<CharRange>> GetCharTable()const
		{
			return char_table->range_table;
		}
		static Ptr<Expression> Alter(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index);

	private:
		static Ptr<Expression> RegexMacro(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index);

		static 	int GetChar(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const int index);
		static Ptr<Expression> CharSet(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const bool reverse, int& index);
		static Ptr<Expression>  BackReference(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int&index);

		static Ptr<Expression> CaptureBegin(const wstring& pattern,const Ptr<vector<RegexToken>>& tokens, int& index);
		static Ptr<Expression> LookBegin(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index);
		static Ptr<Expression> NoneCapture(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index);
		static LoopActionType InitLoopActionMap();
		static RegexParser::ActionType InitActionMap();
		static RegexParser::FirstMapType InitFirstMap();
	private:
		static Ptr<Expression> Unit(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index);
		static Ptr<Expression> Factor(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index);
		static Ptr<Expression> Express(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index);


		static Ptr<Expression> CaptureRight(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens,int& index, const int end_index);
		static wstring Named(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const int index);
		
	};
}
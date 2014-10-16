#pragma once
#include "ztl_regex_data.h"
namespace ztl
{
	//输入 原始串
	//输出 Token列表

	class RegexLex
	{
	private:
		using ActionType = unordered_map < wstring, function<void(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)> > ;
	private:
		static ActionType action_map;
		wstring pattern;
		Ptr<vector<RegexToken>> tokens;
		Ptr<vector<RegexControl>> optional;
	public:
		RegexLex() = delete;
		RegexLex(const wstring& target) : pattern(target), tokens(make_shared<vector<RegexToken>>()), optional(make_shared<vector<RegexControl>>())
		{
		}
		RegexLex(const wstring& target, const Ptr<vector<RegexControl>>& _optional) : pattern(target), tokens(make_shared<vector<RegexToken>>()), optional(_optional)
		{
		}
		~RegexLex() = default;
		void ParsingPattern()
		{
			tokens = ParsingPattern(0, pattern.size());
		}
		Ptr<vector<RegexToken>> ParsingPattern(int start_index, int end_index);
		Ptr<vector<RegexToken>> GetTokens() const
		{
			return move(tokens);
		}
		const wstring GetRawString() const
		{
			return pattern;
		}
		const wstring& GetPattern() const
		{
			return pattern;
		}
	private:
		static void CreatNewParsePattern(const wstring& pattern, int start, int end, Ptr<vector<RegexToken>>& tokens);
		static bool IsNumber(const wchar_t character)
		{
			return character >= 48 && character <= 57;
		}
		static bool IsBlankspace(const wchar_t character)
		{
			return character == L' ' || character == L'\n' || character == L'\r' || character == L'\t';
		}
		static bool IsNamedChar(const wchar_t character)
		{
			return (character <= L'Z' && character >= L'A') || (character <= L'z' && character >= L'a') || RegexLex::IsNumber(character) || character == '_';
		}
		static void NewNormalChar(Ptr<vector<RegexToken>>& tokens, int& index)
		{
			auto&& old = index++;
			tokens->emplace_back(RegexToken(TokenType::NormalChar, old, index));
		}
		static bool IsBlankSpace(const wchar_t character)
		{
			return character == L' ' || character == L'\n' || character == L'\r' || character == L'\t';
		}
		static void SkipBlankSpace(const wstring& pattern, int& index);
		static void GetNumber(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		//捕获名字 格式 空白 名字
		static void GetNamed(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		//find需要跳过转义字符.

		static int FindMetaSymbol(const wstring& pattern, const wchar_t target, int start_index, const exception& error);
		//解析字符集合

		static void ParseCharSet(TokenType type, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		static int GetLongestMatched(const wchar_t matched_begin, const wchar_t matched_end, const wstring& pattern, size_t start_index);

		//解析 默认捕获组
		static void ParseCapture(const TokenType type, const int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		// 预定义字符集合
		static void SetPreDefineCharSet(TokenType type, int& index, Ptr<vector<RegexToken>>& tokens)
		{
			tokens->emplace_back(RegexToken(type));
			index += 2;
		}
		/*
		Look Around
		(?<=Expression)
		逆序肯定环视，表示所在位置左侧能够匹配Expression
		(?<!Expression)
		逆序否定环视，表示所在位置左侧不能匹配Expression
		(?=Expression)
		顺序肯定环视，表示所在位置右侧能够匹配Expression
		(?!Expression)
		顺序否定环视，表示所在位置右侧不能匹配Expression
		*/

		static void ParseLookAround(TokenType begin_type, TokenType end_type, int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
	public:
		static RegexLex::ActionType RegexLex::InitActionMap();
	};
}
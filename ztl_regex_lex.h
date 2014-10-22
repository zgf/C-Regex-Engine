#pragma once
#include "ztl_regex_data.h"
namespace ztl
{
	/*!
	 * \class RegexLex
	 *
	 * \brief 输入 原始串
	 *		  输出 Token列表
	 * \author ZGF
	 * \date 十月 2014
	 */
	class RegexLex
	{
		using ActionType = unordered_map < wstring, function<void(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)> > ;
	public:/*构造析构赋值拷贝*/
		Ptr<vector<RegexToken>> GetTokens() const;
		Ptr<vector<RegexToken>> ParsingPattern(int start_index, int end_index);
		RegexLex() = delete;
		RegexLex(const wstring& target);
		RegexLex(const wstring& target, const Ptr<vector<RegexControl>>& _optional);
		const wstring GetRawString() const;
		void ParsingPattern();
		~RegexLex() = default;
	public:
		static RegexLex::ActionType RegexLex::InitActionMap();
	private:
		//解析字符集合
		static void ParseCharSet(TokenType type, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		//解析 默认捕获组
		static void ParseCapture(const TokenType type, const int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
		//解析匿名后向引用
		static void ParseAnonymityBackReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//解析命名后向引用
		static void ParseBackReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//解析用户自定义循环
		static void ParseUserDefineLoop(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//解析宏引用
		static void ParseMacroReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//解析宏表达式
		static void ParseRegexMacro(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index, const Ptr<vector<RegexControl>>& optional);
		//解析捕获组
		static void ParseNamedCapture(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index, const Ptr<vector<RegexControl>>& optional);
		//解析普通字符
		static void ParseNormalChar(Ptr<vector<RegexToken>>& tokens, int& index);
		//解析环视
		static void ParseLookAround(TokenType begin_type, int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
		//调用子例程解析子串
		static void CreatNewParsePattern(const wstring& pattern, int start, int end, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
	private:
		static bool IsBlankSpace(const wchar_t character);
		static bool IsBlankspace(const wchar_t character);
		static bool IsNamedChar(const wchar_t character);
		static bool IsNumber(const wchar_t character);
		//find需要跳过转义字符.
		static int  FindMetaSymbol(const wstring& pattern, const wchar_t target, int start_index, const exception& error);
		static int  GetLongestMatched(const wchar_t matched_begin, const wchar_t matched_end, const wstring& pattern, size_t start_index);
		static void GetNamed(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		static void JumpTrivalCharacter(int& index, int number = 1);
		static void SetNumberToken(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		// 预定义字符集合
		static void SetPreDefineCharSet(TokenType type, int& index, Ptr<vector<RegexToken>>& tokens);
		static void SkipBlankSpace(const wstring& pattern, int& index);
	private:
		Ptr<vector<RegexControl>>				optional;
		Ptr<vector<RegexToken>>					tokens;
		wstring									pattern;
		static ActionType						action_map;
	};
}
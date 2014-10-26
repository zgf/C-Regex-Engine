#pragma once
#include "ztl_regex_data.h"
namespace ztl
{
	/*!
	 * \class RegexLex
	 *
	 * \brief ���� ԭʼ��
	 *		  ��� Token�б�
	 * \author ZGF
	 * \date ʮ�� 2014
	 */
	class RegexLex
	{
		using ActionType = unordered_map < wstring, function<void(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)> > ;
	public:/*����������ֵ����*/
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
		//�����ַ�����
		static void ParseCharSet(TokenType type, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		//���� Ĭ�ϲ�����
		static void ParseCapture(const TokenType type, const int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
		//����������������
		static void ParseAnonymityBackReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//����������������
		static void ParseBackReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//�����û��Զ���ѭ��
		static void ParseUserDefineLoop(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//����������
		static void ParseMacroReference(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index);
		//��������ʽ
		static void ParseRegexMacro(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index, const Ptr<vector<RegexControl>>& optional);
		//����������
		static void ParseNamedCapture(const wstring& pattern, Ptr<vector<RegexToken>>& tokens, int& index, const Ptr<vector<RegexControl>>& optional);
		//������ͨ�ַ�
		static void ParseNormalChar(Ptr<vector<RegexToken>>& tokens, int& index);
		//��������
		static void ParseLookAround(TokenType begin_type, int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
		//���������̽����Ӵ�
		static void CreatNewParsePattern(const wstring& pattern, int start, int end, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional);
	private:
		static bool IsBlankSpace(const wchar_t character);
		static bool IsBlankspace(const wchar_t character);
		static bool IsNamedChar(const wchar_t character);
		static bool IsNumber(const wchar_t character);
		//find��Ҫ����ת���ַ�.
		static int  FindMetaSymbol(const wstring& pattern, const wchar_t target, int start_index, const exception& error);
		static int  GetLongestMatched(const wchar_t matched_begin, const wchar_t matched_end, const wstring& pattern, size_t start_index);
		static void GetNamed(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		static void JumpTrivalCharacter(int& index, int number = 1);
		static void SetNumberToken(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens);
		// Ԥ�����ַ�����
		static void SetPreDefineCharSet(TokenType type, int& index, Ptr<vector<RegexToken>>& tokens);
		static void SkipBlankSpace(const wstring& pattern, int& index);
	private:
		Ptr<vector<RegexControl>>				optional;
		Ptr<vector<RegexToken>>					tokens;
		wstring									pattern;
		static ActionType						action_map;
	};
}
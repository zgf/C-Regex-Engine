#pragma once
#include "ztl_regex_data.h"
namespace ztl
{
	//输入 原始串
	//输出 Token列表

	class RegexLex
	{
	private:
		wstring pattern;
		Ptr<vector<RegexToken>> tokens;
		unordered_map<wstring, function<void(int& index, Ptr<vector<RegexToken>>& tokens)>> action_map;
	public:
		RegexLex() = delete;
		RegexLex(const wstring& target) : pattern(target), tokens(make_shared<vector<RegexToken>>())
		{
			InitActionMap();
		}
		~RegexLex() = default;
		void ParsingPattern()
		{
			ParsingPattern(0, pattern.size());
		}
		void ParsingPattern(int start_index, int end_index)
		{
			for(auto index = start_index; index < end_index;)
			{
				for(auto catch_length = 4; catch_length >= 1; catch_length--)
				{
					auto&& key = pattern.substr(index, catch_length);
					if(action_map.find(key) != action_map.end())
					{
						//action_map执行完后,index指向正确的位置了.也就不用++了.
						action_map[key](index, tokens);
						break;
					}

					if(catch_length == 1)
					{
						//说明是普通字符. normal长度5 :)
						action_map[L"normal"](index, tokens);
						//break;
					}
				}
			}
		}
		const Ptr<vector<RegexToken>> GetTokens() const
		{
			return move(tokens);
		}
		const wstring GetRawString() const
		{
			return pattern;
		}

	private:
		bool IsNumber(const wchar_t character)
		{
			return character >= 48 && character <= 57;
		}
		bool IsBlankspace(const wchar_t character)
		{
			return character == L' ' || character == L'\n' || character == L'\r' || character == L'\t';
		}
		bool IsNamedChar(const wchar_t character)
		{
			return (character <= L'Z' && character >= L'A') || (character <= L'z' && character >= L'a') || IsNumber(character) || character == '_';
		}
		void NewNormalChar(int& index)
		{
			auto&& old = index++;
			tokens->emplace_back(RegexToken(TokenType::NormalChar, old, index));
		}
		bool IsBlankSpace(const wchar_t character)
		{
			return character == L' ' || character == L'\n' || character == L'\r' || character == L'\t';
		}
		void SkipBlankSpace(const wstring& pattern, int& index)
		{
			while(IsBlankSpace(pattern[index]))
			{
				++index;
			}
		}
		void GetNumber(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
		{
			SkipBlankSpace(pattern, index);
			assert(IsNumber(pattern[index]));
			tokens->emplace_back(RegexToken(TokenType::Number, index, -1));
			while(IsNumber(pattern[index]))
			{
				++index;
			}
			tokens->back().position.max = index;
		}
		//捕获名字 格式 空白 名字
		void GetNamed(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
		{
			SkipBlankSpace(pattern, index);
			assert(IsNamedChar(pattern[index]));
			auto index_begin = index;
			while(IsNamedChar(pattern[index]))
			{
				index++;
			}
			auto index_end = index;
			tokens->emplace_back(RegexToken(TokenType::Named, index_begin, index_end));
		}
		//find需要跳过转义字符.

		int FindMetaSymbol(const wstring& pattern, const wchar_t target, int start_index, const exception& error)
		{
			size_t find_result = 0;
			while(find_result < pattern.size())
			{
				find_result = pattern.find(target, start_index);
				if(find_result == 0)
				{
					return 0;
				}
				else if(pattern[find_result - 1] == '\\')
				{
					start_index = find_result + 1;
				}
				else
				{
					return find_result;
				}
			}
			throw error;
		}
		//解析字符集合

		void ParseCharSet(TokenType type, int& index)
		{
			auto&& result = FindMetaSymbol(pattern, L']', index, std::exception("Lex Parsing Error, can't find ']' in '[' action"));
			tokens->emplace_back(RegexToken(type));
			for(; index != result;)
			{
				if(pattern[index] == L'-')
				{
					tokens->emplace_back(RegexToken(TokenType::Component));
					++index;
				}
				else if(pattern[index] == '\\')
				{
					index += 1;
					NewNormalChar( index );
				}
				else
				{
					NewNormalChar( index);
				}
			}
			tokens->emplace_back(RegexToken(TokenType::CharSetEnd));
			index = result + 1;
		}
		int GetLongestMatched(const wchar_t matched_begin, const wchar_t matched_end, const wstring& pattern, size_t start_index)
		{
			auto count = 1;
			start_index = FindMetaSymbol(pattern, matched_begin, start_index, exception("Lex Parsing Error, can't find symbol in get_longest_matched_functor action"));

			for(start_index++; start_index < pattern.size(); ++start_index)
			{
				if(pattern[start_index] == matched_begin)
				{
					count++;
				}
				else if(pattern[start_index] == matched_end)
				{
					count--;
				}
				if(count == 0)
				{
					return start_index;
				}
			}
			return -1;
		}
		
		//解析 默认捕获组
		void ParseCapture(const TokenType type, const int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
		{
			tokens->emplace_back(RegexToken(type));
			auto index_begin = index + offset;
			auto index_end = GetLongestMatched(L'(', L')', pattern, index);

			ParsingPattern(index_begin, index_end);
			tokens->emplace_back(RegexToken(TokenType::CaptureEnd));
			index = index_end + 1;
		}
		// 预定义字符集合
		void SetPreDefineCharSet(TokenType type, int& index, Ptr<vector<RegexToken>>& tokens)
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

		void ParseLookAround(TokenType begin_type, TokenType end_type, int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
		{
			tokens->emplace_back(RegexToken(begin_type));
			auto index_end = GetLongestMatched(L'(', L')', pattern, index);
			auto index_begin = index + offset;
			ParsingPattern(index_begin, index_end);
			index = index_end + 1;
			tokens->emplace_back(RegexToken(end_type));
		}
		void InitActionMap()
		{
			//action map需要在结束时将index设置向正确的位置.不用循环的时候++了.

			action_map.insert({ L"normal", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				NewNormalChar(index);
			} });
			action_map.insert({ L"\\", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				//第一种,普通字符.第二种后向引用
				index += 1;
				if(IsNumber(pattern[index]))
				{
					tokens->emplace_back(RegexToken(TokenType::BackReference));
					GetNumber(pattern, index, tokens);
				}
				else
				{
					index += 1;
					NewNormalChar(index);
				}
			} });
			action_map.insert({ L"\\k", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				//第二种后向引用
				tokens->emplace_back(RegexToken(TokenType::BackReference));
				index += 2;
				SkipBlankSpace(pattern, index);
				index += 1;
				GetNamed(pattern, index, tokens);
				auto&& result = FindMetaSymbol(pattern, L'>', index, std::exception("Lex Parsing Error, can't find '>' in '\\k' action"));
				index = result + 1;
			} });

			//解析 自定义重复
			action_map.insert({ L"{", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::LoopBegin));
				++index;
				GetNumber(pattern, index, tokens);
				SkipBlankSpace(pattern, index);
				if(pattern[index] == ',')
				{
					//					tokens->emplace_back(RegexToken(TokenType::Comma));
					++index;
				}
				SkipBlankSpace(pattern, index);
				if(IsNumber(pattern[index]))
				{
					GetNumber(pattern, index, tokens);
				}
				else
				{
					tokens->emplace_back(RegexToken(TokenType::InFinite));
				}
				SkipBlankSpace(pattern, index);
				if(pattern[index] == '}')
				{
					if(pattern[index + 1] == '?')
					{
						tokens->emplace_back(RegexToken(TokenType::LoopEnd));
						index += 2;
					}
					else
					{
						tokens->emplace_back(RegexToken(TokenType::LoopEndGreedy));
						index += 1;
					}
				}
				else
				{
					throw std::exception("Lex Parsing Error, can't find '}' in '{' action");
				}
			} });
			//解析 预定义重复
			action_map.insert({ L"*", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::KleeneLoopGreedy));
				index += 1;
			} });
			action_map.insert({ L"*?", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::KleeneLoop));
				index += 2;
			} });
			action_map.insert({ L"+", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::PositiveLoopGreedy));
				index += 1;
			} });
			action_map.insert({ L"+?", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::PositiveLoop));
				index += 2;
			} });
			action_map.insert({ L"?", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::ChoseLoopGreedy));
				index += 1;
			} });
			action_map.insert({ L"??", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::ChoseLoop));
				index += 2;
			} });

			//解析 串头串尾
			action_map.insert({ L"$", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::StringTail));
				index += 1;
			} });
			action_map.insert({ L"^", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::StringHead));
				index += 1;
			} });

			action_map.insert({ L"[", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				index += 1;
				ParseCharSet(TokenType::CharSet, index);
			} });
			action_map.insert({ L"[^", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				index += 2;
				ParseCharSet(TokenType::CharSetReverse, index);
			} });
			action_map.insert({ L"(", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseCapture(TokenType::CaptureBegin, 1, pattern, index, tokens);
			} });
			action_map.insert({ L"(?:", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseCapture(TokenType::NoneCapture, 3, pattern, index, tokens);
			} });
			//解析命名捕获组
			action_map.insert({ L"(<", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::CaptureBegin));

				auto index_end = GetLongestMatched(L'(', L')', pattern, index);

				index += 2;

				GetNamed(pattern, index, tokens);
				auto&& result = FindMetaSymbol(pattern, L'>', index, exception("Lex Parsing Error, can't find '>' in '(<' action"));

				auto index_begin = result + 1;

				ParsingPattern(index_begin, index_end);

				tokens->emplace_back(RegexToken(TokenType::CaptureEnd));
				index = index_end + 1;
			} });

			action_map.insert({ L"(?<=", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseLookAround(TokenType::PositiveLookbehind, TokenType::LookbehindEnd, 4, pattern, index, tokens);
			} });
			action_map.insert({ L"(?<!", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseLookAround(TokenType::NegativeLookbehind, TokenType::LookbehindEnd, 4, pattern, index, tokens);
			} });
			action_map.insert({ L"(?=", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseLookAround(TokenType::PositivetiveLookahead, TokenType::LookaheadEnd, 3, pattern, index, tokens);
			} });
			action_map.insert({ L"(?!", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				ParseLookAround(TokenType::NegativeLookahead, TokenType::LookaheadEnd, 3, pattern, index, tokens);
			} });
			//选择
			action_map.insert({ L"|", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::Alternation));
				index += 1;
			} });
			//注释
			action_map.insert({ L"(#", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				auto&& result = FindMetaSymbol(this->pattern, L')', index, exception("Lex Parsing Error, can't find ')' in '(#' action"));

				index = result + 1;
			} });
			//通配符
			action_map.insert({ L".", []( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				tokens->emplace_back(RegexToken(TokenType::MatchAllSymbol));
				index += 1;
			} });

			action_map.insert({ L"\\s", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSets, index, tokens);
			} });
			action_map.insert({ L"\\S", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSetS, index, tokens);
			} });
			action_map.insert({ L"\\d", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSetd, index, tokens);
			} });
			action_map.insert({ L"\\D", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSetD, index, tokens);
			} });
			action_map.insert({ L"\\w", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSetw, index, tokens);
			} });
			action_map.insert({ L"\\W", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::CharSetW, index, tokens);
			} });
			action_map.insert({ L"\\b", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::Positionb, index, tokens);
			} });
			action_map.insert({ L"\\B", [this]( int& index, Ptr<vector<RegexToken>>& tokens)
			{
				SetPreDefineCharSet(TokenType::PositionB, index, tokens);
			} });
		}
	};
}
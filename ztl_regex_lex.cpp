#include "forward.h"
#include "ztl_regex_lex.h"
namespace ztl
{
	RegexLex::ActionType RegexLex::action_map = RegexLex::InitActionMap();

	Ptr<vector<RegexToken>> RegexLex::ParsingPattern(int start_index, int end_index)
	{
		Ptr<vector<RegexToken>> result(make_shared<vector<RegexToken>>());
		for(auto index = start_index; index < end_index;)
		{
			for(auto catch_length = 4; catch_length >= 1; catch_length--)
			{
				auto&& key = pattern.substr(index, catch_length);
				if(RegexLex::action_map.find(key) != RegexLex::action_map.end())
				{
					//RegexLex::action_map执行完后,index指向正确的位置了.也就不用++了.
					RegexLex::action_map[key](pattern, index, result, optional);
					break;
				}

				if(catch_length == 1)
				{
					//说明是普通字符. normal长度5 :)
					RegexLex::action_map[L"normal"](pattern, index, result, optional);
					//break;
				}
			}
		}
		return move(result);
	}
	RegexLex::ActionType RegexLex::InitActionMap()
	{
		//action map需要在结束时将index设置向正确的位置.不用循环的时候++了.
		RegexLex::ActionType action_map;
		action_map.insert({ L"normal", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::NewNormalChar(tokens, index);
		} });
		action_map.insert({ L"\\", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			//第一种,普通字符.第二种后向引用
			index += 1;
			if(RegexLex::IsNumber(pattern[index]))
			{
				tokens->emplace_back(RegexToken(TokenType::AnonymityBackReference));
				GetNumber(pattern, index, tokens);
			}
			else
			{
				RegexLex::NewNormalChar(tokens, index);
			}
		} });
		action_map.insert({ L"\\k", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			//第二种后向引用
			tokens->emplace_back(RegexToken(TokenType::BackReference));
			index += 2;
			RegexLex::SkipBlankSpace(pattern, index);
			index += 1;
			RegexLex::GetNamed(pattern, index, tokens);
			auto&& result = RegexLex::FindMetaSymbol(pattern, L'>', index, std::exception("Lex Parsing Error, can't find '>' in '\\k' action"));
			index = result + 1;
		} });

		//解析 自定义重复
		action_map.insert({ L"{", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::LoopBegin));
			++index;
			GetNumber(pattern, index, tokens);
			RegexLex::SkipBlankSpace(pattern, index);
			if(pattern[index] == ',')
			{
				//					tokens->emplace_back(RegexToken(TokenType::Comma));
				++index;
				RegexLex::SkipBlankSpace(pattern, index);
				if(RegexLex::IsNumber(pattern[index]))
				{
					GetNumber(pattern, index, tokens);
				}
				else
				{
					tokens->emplace_back(RegexToken(TokenType::InFinite));
				}
			}
			else
			{
				//这里不能用emplace_back
				//会导致迭代器失效
				tokens->push_back(tokens->back());
			}
			
			RegexLex::SkipBlankSpace(pattern, index);
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
		action_map.insert({ L"*", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::KleeneLoopGreedy));
			index += 1;
		} });
		action_map.insert({ L"*?", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::KleeneLoop));
			index += 2;
		} });
		action_map.insert({ L"+", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::PositiveLoopGreedy));
			index += 1;
		} });
		action_map.insert({ L"+?", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::PositiveLoop));
			index += 2;
		} });
		action_map.insert({ L"?", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::ChoseLoopGreedy));
			index += 1;
		} });
		action_map.insert({ L"??", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::ChoseLoop));
			index += 2;
		} });

		//解析 串头串尾
		action_map.insert({ L"$", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			if(find(optional->begin(), optional->end(), RegexControl::Multiline) != optional->end())
			{
				tokens->emplace_back(RegexToken(TokenType::LineEnd));
			}
			else
			{
				tokens->emplace_back(RegexToken(TokenType::StringTail));
			}
			index += 1;
		} });
		action_map.insert({ L"^", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			if(find(optional->begin(), optional->end(), RegexControl::Multiline) != optional->end())
			{
				tokens->emplace_back(RegexToken(TokenType::LineBegin));
			}
			else
			{
				tokens->emplace_back(RegexToken(TokenType::StringHead));
			}
			index += 1;
		} });

		action_map.insert({ L"[", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			index += 1;
			RegexLex::ParseCharSet(TokenType::CharSet, pattern, index, tokens);
		} });
		action_map.insert({ L"[^", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			index += 2;
			RegexLex::ParseCharSet(TokenType::CharSetReverse, pattern, index, tokens);
		} });
		action_map.insert({ L"(", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			if(find(optional->begin(), optional->end(), RegexControl::ExplicitCapture) != optional->end())
			{
				RegexLex::ParseCapture(TokenType::NoneCapture, 1, pattern, index, tokens,optional);
			}
			else
			{
				RegexLex::ParseCapture(TokenType::CaptureBegin, 1, pattern, index, tokens,optional);
			}
		} });
		action_map.insert({ L"(?:", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::ParseCapture(TokenType::NoneCapture, 3, pattern, index, tokens,optional);
		} });
		//解析命名捕获组
		action_map.insert({ L"(<", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::CaptureBegin));

			auto index_end = RegexLex::GetLongestMatched(L'(', L')', pattern, index);

			index += 2;

			RegexLex::GetNamed(pattern, index, tokens);
			auto&& result = RegexLex::FindMetaSymbol(pattern, L'>', index, exception("Lex Parsing Error, can't find '>' in '(<' action"));

			auto index_begin = result + 1;
			RegexLex::CreatNewParsePattern(pattern, index_begin, index_end, tokens,optional);

			tokens->emplace_back(RegexToken(TokenType::CaptureEnd));
			index = index_end + 1;
		} });
		action_map.insert({ L"(?#<", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::RegexMacro));

			auto index_end = RegexLex::GetLongestMatched(L'(', L')', pattern, index);

			index += 4;

			RegexLex::GetNamed(pattern, index, tokens);
			auto&& result = RegexLex::FindMetaSymbol(pattern, L'>', index, exception("Lex Parsing Error, can't find '>' in '(?#<' action"));

			auto index_begin = result + 1;
			RegexLex::CreatNewParsePattern(pattern, index_begin, index_end, tokens,optional);

			tokens->emplace_back(RegexToken(TokenType::CaptureEnd));
			index = index_end + 1;
		} });
		action_map.insert({ L"$<", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::MacroReference));
			//第二种后向引用
			index += 2;
			RegexLex::GetNamed(pattern, index, tokens);
			auto&& result = RegexLex::FindMetaSymbol(pattern, L'>', index, std::exception("Lex Parsing Error, can't find '>' in '$<' action"));
			index = result + 1;
		} });
		action_map.insert({ L"(?<=", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::ParseLookAround(TokenType::PositiveLookbehind, TokenType::LookbehindEnd, 4, pattern, index, tokens,optional);
		} });
		action_map.insert({ L"(?<!", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::ParseLookAround(TokenType::NegativeLookbehind, TokenType::LookbehindEnd, 4, pattern, index, tokens,optional);
		} });
		action_map.insert({ L"(?=", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::ParseLookAround(TokenType::PositivetiveLookahead, TokenType::LookaheadEnd, 3, pattern, index, tokens,optional);
		} });
		action_map.insert({ L"(?!", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::ParseLookAround(TokenType::NegativeLookahead, TokenType::LookaheadEnd, 3, pattern, index, tokens,optional);
		} });
		//选择
		action_map.insert({ L"|", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			tokens->emplace_back(RegexToken(TokenType::Alternation));
			index += 1;
		} });
		//注释
		action_map.insert({ L"(#", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			auto&& result = RegexLex::FindMetaSymbol(pattern, L')', index, exception("Lex Parsing Error, can't find ')' in '(#' action"));

			index = result + 1;
		} });
		//通配符
		action_map.insert({ L".", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			if(find(optional->begin(), optional->end(), RegexControl::Singleline) != optional->end())
			{
				tokens->emplace_back(RegexToken(TokenType::MatchAllSymbol));
			}
			else
			{
				tokens->emplace_back(RegexToken(TokenType::GeneralMatch));
			}
			index += 1;
		} });

		action_map.insert({ L"\\s", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSets, index, tokens);
		} });
		action_map.insert({ L"\\S", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSetS, index, tokens);
		} });
		action_map.insert({ L"\\d", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSetd, index, tokens);
		} });
		action_map.insert({ L"\\D", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSetD, index, tokens);
		} });
		action_map.insert({ L"\\w", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSetw, index, tokens);
		} });
		action_map.insert({ L"\\W", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::CharSetW, index, tokens);
		} });
		action_map.insert({ L"\\b", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::Positionb, index, tokens);
		} });
		action_map.insert({ L"\\B", [](const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
		{
			RegexLex::SetPreDefineCharSet(TokenType::PositionB, index, tokens);
		} });
		return move(action_map);
	}
	void RegexLex::CreatNewParsePattern(const wstring& pattern, int start, int end, Ptr<vector<RegexToken>>& tokens,const Ptr<vector<RegexControl>>& optional)
	{
		RegexLex lexer(pattern, optional);
		auto&& result = lexer.ParsingPattern(start, end);
		tokens->insert(tokens->end(), result->begin(), result->end());
	}
	void RegexLex::SkipBlankSpace(const wstring& pattern, int& index)
	{
		while(RegexLex::IsBlankSpace(pattern[index]))
		{
			++index;
		}
	}

	void RegexLex::GetNamed(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
	{
		RegexLex::SkipBlankSpace(pattern, index);
		assert(RegexLex::IsNamedChar(pattern[index]));
		auto index_begin = index;
		while(RegexLex::IsNamedChar(pattern[index]))
		{
			index++;
		}
		auto index_end = index;
		tokens->emplace_back(RegexToken(TokenType::Named, index_begin, index_end));
	}

	void RegexLex::GetNumber(const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
	{
		RegexLex::SkipBlankSpace(pattern, index);
		assert(RegexLex::IsNumber(pattern[index]));
		tokens->emplace_back(RegexToken(TokenType::Number, index, -1));
		while(RegexLex::IsNumber(pattern[index]))
		{
			++index;
		}
		tokens->back().position.max = index;
	}
	int RegexLex::FindMetaSymbol(const wstring& pattern, const wchar_t target, int start_index, const exception& error)
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

	void RegexLex::ParseCharSet(TokenType type, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens)
	{
		auto&& result = RegexLex::FindMetaSymbol(pattern, L']', index, std::exception("Lex Parsing Error, can't find ']' in '[' action"));
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
				RegexLex::NewNormalChar(tokens, index);
			}
			else
			{
				RegexLex::NewNormalChar(tokens, index);
			}
		}
		tokens->emplace_back(RegexToken(TokenType::CharSetEnd));
		index = result + 1;
	}

	int RegexLex::GetLongestMatched(const wchar_t matched_begin, const wchar_t matched_end, const wstring& pattern, size_t start_index)
	{
		auto count = 1;
		start_index = RegexLex::FindMetaSymbol(pattern, matched_begin, start_index, exception("Lex Parsing Error, can't find symbol in get_longest_matched_functor action"));

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
	void RegexLex::ParseCapture(const TokenType type, const int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
	{
		tokens->emplace_back(RegexToken(type));
		auto index_begin = index + offset;
		auto index_end = RegexLex::GetLongestMatched(L'(', L')', pattern, index);
		RegexLex::CreatNewParsePattern(pattern, index_begin, index_end, tokens,optional);

		tokens->emplace_back(RegexToken(TokenType::CaptureEnd));
		index = index_end + 1;
	}

	void RegexLex::ParseLookAround(TokenType begin_type, TokenType end_type, int offset, const wstring& pattern, int& index, Ptr<vector<RegexToken>>& tokens, const Ptr<vector<RegexControl>>& optional)
	{
		tokens->emplace_back(RegexToken(begin_type));
		auto index_end = RegexLex::GetLongestMatched(L'(', L')', pattern, index);
		auto index_begin = index + offset;
		RegexLex::CreatNewParsePattern(pattern, index_begin, index_end, tokens,optional);

		index = index_end + 1;
		tokens->emplace_back(RegexToken(end_type));
	}
}
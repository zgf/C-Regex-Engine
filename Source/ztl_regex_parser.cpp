#include "forward.h"
#include "ztl_regex_parser.h"
namespace ztl
{
	RegexParser::FirstMapType RegexParser::first_map = RegexParser::InitFirstMap();
	RegexParser::ActionType RegexParser::actions = RegexParser::InitActionMap();
	RegexParser::LoopActionType RegexParser::loop_actions = RegexParser::InitLoopActionMap();
	RegexParser::RegexParser(const RegexLex& lexer)
		:tokens(lexer.GetTokens()), pattern(lexer.GetRawString()), expression(nullptr), char_table(make_shared<CharTable>()), optional(make_shared<vector<RegexControl>>())
	{
	}
	RegexParser::RegexParser(const RegexLex& lexer, const Ptr<vector<RegexControl>>& _optional)
		: optional(_optional), tokens(lexer.GetTokens()), pattern(lexer.GetRawString()), expression(nullptr), char_table(make_shared<CharTable>())
	{
	}
	RegexParser::FirstMapType RegexParser::InitFirstMap()
	{
		RegexParser::FirstMapType first_map;
		first_map.insert({ TokenType::Loop, make_shared<unordered_set<TokenType>>(unordered_set<TokenType>(
		{
			TokenType::LoopBegin,
			TokenType::ChoseLoop,
			TokenType::ChoseLoopGreedy,
			TokenType::PositiveLoop,
			TokenType::PositiveLoopGreedy,
			TokenType::KleeneLoop,
			TokenType::KleeneLoopGreedy,
		})) });

		//first_map.insert({ TokenType::CaptureRight, { TokenType::Named } });
		first_map.insert({ TokenType::Factor, make_shared<unordered_set<TokenType>>(unordered_set<TokenType>(
		{
			TokenType::CaptureBegin,
			TokenType::AnonymityCaptureBegin,
			TokenType::NoneCapture,
			TokenType::PositivetiveLookahead,
			TokenType::NegativeLookahead,
			TokenType::PositiveLookbehind,
			TokenType::NegativeLookbehind,
			TokenType::StringHead,
			TokenType::StringTail,
			TokenType::BackReference,
			TokenType::CharSet,
			TokenType::CharSetReverse,
			TokenType::NormalChar,
			TokenType::CharSets,
			TokenType::CharSetS,
			TokenType::CharSetW,
			TokenType::CharSetw,
			TokenType::CharSetD,
			TokenType::CharSetd,
			TokenType::GeneralMatch,
			TokenType::PositionB,
			TokenType::Positionb,
			TokenType::LineEnd,
			TokenType::LineBegin,
			TokenType::MatchAllSymbol,
			TokenType::RegexMacro,
			TokenType::MacroReference,
			TokenType::AnonymityBackReference,
		})) });
		first_map.insert({ TokenType::Express, first_map[TokenType::Factor] });
		first_map.insert({ TokenType::Unit, first_map[TokenType::Factor] });
		first_map.insert({ TokenType::Alter, first_map[TokenType::Factor] });
		return move(first_map);
	}
	Ptr<Expression> RegexParser::GetExpressTree()const
	{
		return expression;
	}
	Ptr<CharTable> RegexParser::GetCharTable()const
	{
		return char_table;
	}
	int WstringToNumber(const wstring& str)
	{
		int number;
		wstringstream convetor;
		convetor << str;
		convetor >> number;
		convetor.clear();
		return move(number);
	}
	void RegexParser::RegexParsing()
	{
		int index = 0;
		expression = RegexParser::Alter(pattern, tokens, index, tokens->size());
		char_table->range_table = expression->GetCharSetTable(optional);
		char_table->char_table  = CreatWCharTable(char_table->range_table);
		expression->SetTreeCharSetOrthogonal(char_table);
	}

	Ptr<vector<unsigned short>> RegexParser::CreatWCharTable(const Ptr<vector<CharRange>>& table)
	{
		auto result(make_shared<vector<unsigned short>>(65536));
		auto current_iter = result->begin();
		for(size_t i = 0; i < table->size(); i++)
		{
			auto& element = (*table)[i];
			auto count    = element.max - element.min + 1;
			fill_n(current_iter, move(count), i);
			current_iter += count;
		}
		return move(result);
	}
	Ptr<Expression> RegexParser::LookBegin(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)
	{
		index += 1;
		auto loop = make_shared<LoopExpression>(express);
		auto current_type = tokens->at(index).type;
		auto count = 0;
		while(current_type != TokenType::LoopEnd && current_type != TokenType::LoopEndGreedy)
		{
			auto& position = tokens->at(index).position;
			current_type = tokens->at(index).type;
			if(count == 0)
			{
				loop->begin = WstringToNumber(pattern.substr(position.min, position.max - position.min));
			}
			else if(current_type == TokenType::Number)
			{
				loop->end = WstringToNumber(pattern.substr(position.min, position.max - position.min));
			}
			index++;
			count++;
		}
		if(current_type == TokenType::LoopEndGreedy)
		{
			loop->greedy = true;
		}
		return move(loop);
	}
	RegexParser::LoopActionType RegexParser::InitLoopActionMap()
	{
		LoopActionType loop_actions;
		loop_actions.insert({ TokenType::LoopBegin, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			return LookBegin(pattern, tokens, express, index);
		} });
		loop_actions.insert({ TokenType::ChoseLoop, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 0, 1);
		} });

		loop_actions.insert({ TokenType::ChoseLoopGreedy, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 0, 1, true);
		} });
		loop_actions.insert({ TokenType::PositiveLoop, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 1);
		} });
		loop_actions.insert({ TokenType::PositiveLoopGreedy, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 1, -1, true);
		} });
		loop_actions.insert({ TokenType::KleeneLoop, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 0, -1);
		} });
		loop_actions.insert({ TokenType::KleeneLoopGreedy, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, Ptr<Expression>& express, int& index)->Ptr < Expression >
		{
			index += 1;
			return  make_shared<LoopExpression>(express, 0, -1, true);
		} });
		return move(loop_actions);
	}
	template<typename Type>
	Ptr<Expression> LookAround(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, Type)
	{
		auto end_index = -1;
		index += 1;
		end_index = FindTheLongestCaptureEnd(tokens, index);
		auto result        = make_shared<Type>();
		result->expression = RegexParser::Alter(pattern, tokens, index, end_index);
		index = end_index + 1;
		return move(result);
	}
	wstring RegexParser::Named(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const int index)
	{
		return pattern.substr(tokens->at(index).position.min, tokens->at(index).position.max - tokens->at(index).position.min);
	}
	Ptr<Expression>  RegexParser::BackReference(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int&index)
	{
		index += 1;
		auto name = Named(pattern, tokens, index);
		index += 1;
		return make_shared<BackReferenceExpression>(name);
	}
	Ptr<Expression>  RegexParser::AnonymityBackReference(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int&index)
	{
		index += 1;
		auto name = Named(pattern, tokens, index);
		auto number = _wtoi(name.c_str());
		index += 1;
		return make_shared<AnonymityBackReferenceExpression>(number);
	}
	//进来时已经去掉了最外层的(括号,找到匹配的最外层的)括号
	int FindTheLongestCaptureEnd(const Ptr<vector<RegexToken>>& tokens, const int index)
	{
		int count = 1;
		for(auto i = index; i < tokens->size(); i++)
		{
			auto& type = (*tokens)[i].type;
			if( type == TokenType::PositiveLookbehind || 
				type == TokenType::PositivetiveLookahead || 
				type == TokenType::NegativeLookahead || 
				type == TokenType::NegativeLookbehind || 
				type == TokenType::CaptureBegin || 
				type == TokenType::NoneCapture || 
				type == TokenType::AnonymityCaptureBegin ||

				type == TokenType::RegexMacro)
			{
				count++;
			}
			else if(type == TokenType::CaptureEnd)
			{
				count--;
			}
			if(count == 0)
			{
				return i;
			}
		}
		throw exception("can't find the longest ')'");
	}
	Ptr<Expression> RegexParser::NoneCapture(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)
	{
		index += 1;

		auto end_index = FindTheLongestCaptureEnd(tokens, index);
		auto exp       = Alter(pattern, tokens, index, end_index);
		auto result    = make_shared<NoneCaptureExpression>(exp);
		index += 1;
		return result;
	}
	Ptr<Expression> RegexParser::RegexMacro(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)
	{
		index += 1;

		auto end_index = FindTheLongestCaptureEnd(tokens, index);

		wstring name   = Named(pattern, tokens, index);
		if(tokens->at(index).type != TokenType::Named)
		{
			throw exception("expect symbol not a name");
		}
		index += 1;
		if(RegexParser::first_map[TokenType::Alter]->find(tokens->at(index).type) == RegexParser::first_map[TokenType::Alter]->end())
		{
			throw exception("expect symbol not in fist[CaptureRight]");
		}
		auto alter  = Alter(pattern, tokens, index, end_index);
		auto result = make_shared<MacroExpression>(name, alter);
		index += 1;
		return result;
	}
	RegexParser::ActionType RegexParser::InitActionMap()
	{
		RegexParser::ActionType actions;
		actions.insert({ TokenType::NormalChar, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			auto character = GetChar(pattern, tokens, index);
			index += 1;
			return make_shared<NormalCharExpression>(CharRange(character, character));
		} });

		actions.insert({ TokenType::CaptureBegin, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return CaptureBegin(pattern, tokens, index);
		} });
		actions.insert({ TokenType::AnonymityCaptureBegin, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return AnonymityCaptureBegin(pattern, tokens, index);
		} });
		actions.insert({ TokenType::RegexMacro, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return RegexMacro(pattern, tokens, index);
		} });
		actions.insert({ TokenType::NoneCapture, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return NoneCapture(pattern, tokens, index);
		} });

		actions.insert({ TokenType::CharSet, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return CharSet(pattern, tokens, false, index);
		} });
		actions.insert({ TokenType::CharSetReverse, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return CharSet(pattern, tokens, true, index);
		} });

		actions.insert({ TokenType::CharSetW, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
		} });
		actions.insert({ TokenType::CharSetw, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
		} });
		actions.insert({ TokenType::CharSetS, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(true, vector<CharRange>({ { L' ', L' ' }, { L'\r', L'\r' }, { L'\n', L'\n' }, { L'\t', L'\t' } }));
		} });
		actions.insert({ TokenType::CharSets, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(false, vector<CharRange>({ { L' ', L' ' }, { L'\r', L'\r' }, { L'\n', L'\n' }, { L'\t', L'\t' } }));
		} });
		actions.insert({ TokenType::CharSetD, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(true, vector<CharRange>({ { L'0', L'9' } }));
		} });
		actions.insert({ TokenType::CharSetd, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(false, vector<CharRange>({ { L'0', L'9' } }));
		} });

		actions.insert({ TokenType::BackReference, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return BackReference(pattern, tokens, index);
		} });
		actions.insert({ TokenType::AnonymityBackReference, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return AnonymityBackReference(pattern, tokens, index);
		} });
		actions.insert({ TokenType::MacroReference, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			auto name = Named(pattern, tokens, index);
			index += 1;
			return make_shared<MacroReferenceExpression>(name);
		} });

		actions.insert({ TokenType::PositiveLookbehind, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return LookAround(pattern, tokens, index, PositiveLookbehindExpression());
		} });
		actions.insert({ TokenType::NegativeLookbehind, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return LookAround(pattern, tokens, index, NegativeLookbehindExpression());
		} });
		actions.insert({ TokenType::PositivetiveLookahead, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return LookAround(pattern, tokens, index, PositivetiveLookaheadExpression());
		} });
		actions.insert({ TokenType::NegativeLookahead, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			return LookAround(pattern, tokens, index, NegativeLookaheadExpression());
		} });
		actions.insert({ TokenType::StringHead, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<BeginExpression>();
		} });
		actions.insert({ TokenType::StringTail, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<EndExpression>();
		} });
		actions.insert({ TokenType::LineBegin, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			auto n = make_shared<PositiveLookbehindExpression>(make_shared<NormalCharExpression>(CharRange(L'\n', L'\n')));
			auto begin = make_shared<BeginExpression>();
			//(?<=\n)|^
			return make_shared<AlternationExpression>(move(n), move(begin));
		} });
		actions.insert({ TokenType::LineEnd, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			//(?=\n)|$
			auto n = make_shared<PositivetiveLookaheadExpression>(make_shared<NormalCharExpression>(CharRange(L'\n', L'\n')));
			auto end = make_shared<EndExpression>();
			return make_shared<AlternationExpression>(move(n),move(end));
		} });
		actions.insert({ TokenType::GeneralMatch, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			//不能让.匹配到\0,\0是结束符号- -.
			index += 1;
			return make_shared<CharSetExpression>(true, vector<CharRange>({ { L'\n', L'\n' }, { L'\0', L'\0' } }));
		} });
		actions.insert({ TokenType::MatchAllSymbol, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			return make_shared<CharSetExpression>(false, vector<CharRange>({ { 0, 65535 } }));
		} });
		actions.insert({ TokenType::Positionb, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			// \b匹配 \\w和\\W之间的位置,W和w位置可以交换
			//所以\b ==((?<=\w)(?=\W)|(?<=\W)(?=\w))| ((^(?=\w)|(?<=\w)$)
			//((?<=\w)(?=\W)|(?<=\W)(?=\w))|
			//((^(?=\w)|(?<=\w)$)
			index += 1;
			auto w = make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
			auto W = make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));

			auto seq_left = make_shared<SequenceExpression>(
				make_shared<PositiveLookbehindExpression>(w),
				make_shared<PositivetiveLookaheadExpression>(W));
			auto seq_right= make_shared<SequenceExpression>(
				make_shared<PositiveLookbehindExpression>(W),
				make_shared<PositivetiveLookaheadExpression>(w));
			auto input_head = make_shared<SequenceExpression>(make_shared<BeginExpression>(), make_shared<PositivetiveLookaheadExpression>(w));
			auto input_end = make_shared<SequenceExpression>(make_shared<PositiveLookbehindExpression>(w),make_shared<EndExpression>());
			auto left = make_shared<AlternationExpression>(move(seq_left), move(seq_right));
			auto right = make_shared<AlternationExpression>(move(input_head), move(input_end));
			return make_shared<AlternationExpression>(move(left), move(right));
		} });
		//所以\B == ((?<=\\w)(?=\\w))|((?<=\\W)(?=\\W))

		actions.insert({ TokenType::PositionB, [](const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)->Ptr < Expression >
		{
			index += 1;
			auto w = make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
			auto W = make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
			auto seq_left = make_shared<SequenceExpression>(
				make_shared<PositiveLookbehindExpression>(w),
				make_shared<PositivetiveLookaheadExpression>(w));
			auto seq_right= make_shared<SequenceExpression>
				(make_shared<PositiveLookbehindExpression>(W),
				 make_shared<PositivetiveLookaheadExpression>(W));
			return make_shared<AlternationExpression>(move(seq_left), move(seq_right));
		} });
		return move(actions);
	}
	int RegexParser::GetChar(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const int index)
	{
		auto&& position = tokens->at(index).position;
		auto&& substring = pattern.substr(position.min, position.max - position.min);
		assert(substring.size() == 1);
		return substring[0];
	}
	Ptr<Expression> RegexParser::CaptureBegin(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)
	{
		index += 1;
		auto end_index = FindTheLongestCaptureEnd(tokens, index);

		auto result    = CaptureRight(pattern, tokens, index, end_index);
		index += 1;
		return result;
	}
	Ptr<Expression>	 RegexParser::AnonymityCaptureBegin(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)
	{
		index += 1;
		auto end_index = FindTheLongestCaptureEnd(tokens, index);

		auto result    = AnonymityCaptureRight(pattern, tokens, index, end_index);
		index += 1;
		return result;
	}

	Ptr<Expression> RegexParser::CharSet(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, const bool reverse, int& index)
	{
		index += 1;
		auto result     = make_shared<CharSetExpression>();
		result->reverse = reverse;
		CharRange temp;
		while(tokens->at(index).type != TokenType::CharSetEnd)
		{
			if(tokens->at(index + 1).type == TokenType::Component)
			{
				temp.min = GetChar(pattern, tokens, index);
				temp.max = GetChar(pattern, tokens, index + 2);
				result->range.emplace_back(move(temp));
				index += 3;
			}
			else
			{
				temp.min = GetChar(pattern, tokens, index);
				temp.max = temp.min;
				result->range.emplace_back(move(temp));
				index += 1;
			}
		}
		index += 1;
		return result;
	}
	Ptr<Expression> RegexParser::CaptureRight(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index)
	{
		wstring name;
		name   = Named(pattern, tokens, index);
		index += 1;
		if(RegexParser::first_map[TokenType::Alter]->find(tokens->at(index).type) == RegexParser::first_map[TokenType::Alter]->end())
		{
			throw exception("expect symbol not in fist[CaptureRight]");
		}
		auto alter  = Alter(pattern, tokens, index, end_index);
		auto result = make_shared<CaptureExpression>(name, alter);
		return result;
	}
	Ptr<Expression>	RegexParser::AnonymityCaptureRight(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index)
	{
		if(RegexParser::first_map[TokenType::Alter]->find(tokens->at(index).type) == RegexParser::first_map[TokenType::Alter]->end())
		{
			throw exception("expect symbol not in fist[AnonymityCaptureRight]");
		}
		auto alter  = Alter(pattern, tokens, index, end_index);
		auto result = make_shared<AnonymityCaptureExpression>(0, alter);
		return result;
	}

	Ptr<Expression> RegexParser::Express(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index)
	{
		auto current_type = tokens->at(index).type;
		if(RegexParser::first_map[TokenType::Express]->find(current_type) == RegexParser::first_map[TokenType::Express]->end())
		{
			throw exception("expect symbol not in fist[Express]");
		}
		auto factor = Factor(pattern, tokens, index);
		if(index < end_index)
		{
			current_type = tokens->at(index).type;
			if(RegexParser::first_map[TokenType::Loop]->find(current_type) != RegexParser::first_map[TokenType::Loop]->end())
			{
				return loop_actions[current_type](pattern, tokens, factor, index);
			}
		}
		return factor;
	}
	Ptr<Expression> RegexParser::Factor(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index)
	{
		auto& current_type = tokens->at(index).type;
		if(RegexParser::first_map[TokenType::Factor]->find(current_type) == RegexParser::first_map[TokenType::Factor]->end())
		{
			throw exception("expect symbol not in fist[Factor]");
		}
		return RegexParser::actions[current_type](pattern, tokens, index);

	}
	Ptr<Expression> RegexParser::Unit(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index)
	{
		if(RegexParser::first_map[TokenType::Unit]->find(tokens->at(index).type) == RegexParser::first_map[TokenType::Unit]->end())
		{
			throw exception("expect symbol not in fist[Unit]");
		}
		auto left = Express(pattern, tokens, index, end_index);
		if(index < end_index)
		{
			if(RegexParser::first_map[TokenType::Unit]->find(tokens->at(index).type) != RegexParser::first_map[TokenType::Unit]->end())
			{
				auto right = Unit(pattern, tokens, index, end_index);
				return make_shared<SequenceExpression>(left, right);
			}
		}
		return left;
	}
	Ptr<Expression> RegexParser::Alter(const wstring& pattern, const Ptr<vector<RegexToken>>& tokens, int& index, const int end_index)
	{
		auto& current_type = tokens->at(index).type;
		if(RegexParser::first_map[TokenType::Alter]->find(current_type) == RegexParser::first_map[TokenType::Alter]->end())
		{
			throw exception("expect symbol not in first[Alter]");
		}
		auto left = Unit(pattern, tokens, index, end_index);
		if(index < end_index)
		{
			if(tokens->at(index).type == TokenType::Alternation)
			{
				index += 1;
				auto right = Alter(pattern, tokens, index, end_index);
				return make_shared<AlternationExpression>(left, right);
			}
		}
		return move(left);
	}
}
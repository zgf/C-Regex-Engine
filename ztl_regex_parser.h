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

Factor	= “CaptureBegin”  CaptureRight
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
CaptureRight = "Named"  Alert "CaptureEnd" |Alert  "CaptureEnd"
*/

namespace ztl
{
	class RegexParser
	{
	private:
		Ptr<vector<RegexToken>> tokens;
		Ptr<Expression>			expression;
		unordered_map<TokenType, function<Ptr<Expression>(  int& index)>> actions;
		unordered_map<TokenType, function<Ptr<Expression>( Ptr<Expression>& express, int& index)>> loop_actions;
		Ptr<vector<CharRange>> table;//字母表
		unordered_map<TokenType, Ptr<unordered_set<TokenType>>>first_map;
		int capture_count = 0;
		wstring pattern;
	public:
		RegexParser() = delete;
		RegexParser(const wstring& input_string, const Ptr<vector<RegexToken>>& token_list)
			:tokens(token_list), pattern(input_string), expression(nullptr), table(nullptr)
		{
			InitFirstMap();
			InitActionMap();
			InitLoopActionMap();
		}
		~RegexParser() = default;
		void RegexParsing()
		{
			int index = 0;
			expression = Alter(index, tokens->size());
			this->table = expression->GetCharSetTable();
			expression->SetTreeCharSetOrthogonal(table);
		}
		Ptr<Expression> GetExpressTree()const
		{
			return expression;
		}
		Ptr<vector<CharRange>> GetCharTable()const
		{
			return table;
		}
	private:
		int WstringToNumber(const wstring& str)
		{
			int number;
			wstringstream convetor;
			convetor << str;
			convetor >> number;
			return move(number);
		}
		int GetChar(const int index)
		{
			auto&& position = tokens->at(index).position;
			auto&& substring = pattern.substr(position.min, position.max - position.min);
			assert(substring.size() == 1);
			return substring[0];
		}
		Ptr<Expression> CharSet(const bool reverse, int& index)
		{
			index += 1;
			auto&& result = make_shared<CharSetExpression>();
			result->reverse = reverse;
			CharRange temp;
			while(tokens->at(index).type != TokenType::CharSetEnd)
			{
				if(tokens->at(index + 1).type == TokenType::Component)
				{
					temp.min = GetChar(index);
					temp.max = GetChar(index + 2);
					result->range.emplace_back(move(temp));
					index += 3;
				}
				else
				{
					temp.min = GetChar(index);
					temp.max = temp.min;
					result->range.emplace_back(move(temp));
					index += 1;
				}
			}
			index += 1;
			return move(result);
		}
		Ptr<Expression>  BackReference(int&index)
		{
			index += 1;
			auto&& name = this->Named(index);
			index += 1;
			return make_shared<BackReferenceExpression>(name);
		
		}
		Ptr<Expression> CaptureBegin(int& index)
		{
			index += 1;
			auto end_index = index;
			auto current_type = tokens->at(end_index).type;
			while(current_type != TokenType::CaptureEnd)
			{
				end_index++;
				current_type = tokens->at(end_index).type;
			}
			auto&& result = CaptureRight(index, end_index);
			index += 1;
			return move(result);
		}
		Ptr<Expression> LookBegin(Ptr<Expression>& express,int& index)
		{
			index += 1;
			auto&& loop = make_shared<LoopExpression>(express);
			auto&& current_type = tokens->at(index).type;
			auto count = 0;
			while(current_type != TokenType::LoopEnd && current_type != TokenType::LoopEndGreedy)
			{
				auto&& position = tokens->at(index).position;
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
		Ptr<Expression> NoneCapture(int& index)
		{
			index += 1;
			auto end_index = index;
			auto current_type = tokens->at(end_index).type;
			while(current_type != TokenType::CaptureEnd)
			{
				end_index++;
				current_type = tokens->at(end_index).type;
			}
			auto&& exp = Alter(index, end_index);
			auto&& result = make_shared<NoneCaptureExpression>(exp);
			index += 1;
			return move(result);
		}
		void InitLoopActionMap()
		{
			loop_actions.insert({ TokenType::LoopBegin, [this]( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				return LookBegin(express, index);
			} });
			loop_actions.insert({ TokenType::ChoseLoop, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 0, 1);
			} });

			loop_actions.insert({ TokenType::ChoseLoopGreedy, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 0, 1, true);
			} });
			loop_actions.insert({ TokenType::PositiveLoop, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 1);
			} });
			loop_actions.insert({ TokenType::PositiveLoopGreedy, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 1, -1, true);
			} });
			loop_actions.insert({ TokenType::KleeneLoop, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 0, -1);
			} });
			loop_actions.insert({ TokenType::KleeneLoopGreedy, []( Ptr<Expression>& express,  int& index)->Ptr<Expression>
			{
				index += 1;
				return  make_shared<LoopExpression>(express, 0, -1, true);
			} });
		}
		void InitActionMap()
		{
			actions.insert({ TokenType::NormalChar, [this](  int& index)->Ptr<Expression>
			{
				auto&& character = GetChar(index);
				index += 1;
				return make_shared<NormalCharExpression>(CharRange(character, character));
			} });

			actions.insert({ TokenType::CaptureBegin, [this](  int& index)->Ptr<Expression>
			{
				return CaptureBegin(index);
			} });
			actions.insert({ TokenType::NoneCapture, [this](  int& index)->Ptr<Expression>
			{
				return NoneCapture(index);
			} });

			actions.insert({ TokenType::CharSet, [this](  int& index)->Ptr<Expression>
			{
				return CharSet(false, index);
			} });
			actions.insert({ TokenType::CharSetReverse, [this](  int& index)->Ptr<Expression>
			{
				return CharSet(true, index);
			} });

			actions.insert({ TokenType::CharSetW, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
			} });
			actions.insert({ TokenType::CharSetw, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900 , 0xFA2D } }));
			} });
			actions.insert({ TokenType::CharSetS, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(true, vector<CharRange>({ { L' ', L' ' }, { L'\r', L'\r' }, { L'\n', L'\n' }, { L'\t', L'\t' } }));
			} });
			actions.insert({ TokenType::CharSets, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(false, vector<CharRange>({ { L' ', L' ' }, { L'\r', L'\r' }, { L'\n', L'\n' }, { L'\t', L'\t' } }));
			} });
			actions.insert({ TokenType::CharSetD, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(true, vector<CharRange>({ { L'0', L'9' } }));
			} });
			actions.insert({ TokenType::CharSetd, [](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<CharSetExpression>(false, vector<CharRange>({ { L'0', L'9' } }));
			} });
		
			actions.insert({ TokenType::BackReference, [this](  int& index)->Ptr<Expression>
			{
				return BackReference(index);
			} });

			actions.insert({ TokenType::PositiveLookbehind, [this](  int& index)->Ptr<Expression>
			{
				return LookAround(TokenType::LookbehindEnd, index, PositiveLookbehindExpression());
			} });
			actions.insert({ TokenType::NegativeLookbehind, [this](  int& index)->Ptr<Expression>
			{
				return LookAround(TokenType::LookbehindEnd, index, NegativeLookbehindExpression());
			} });
			actions.insert({ TokenType::PositivetiveLookahead, [this](  int& index)->Ptr<Expression>
			{
				return LookAround(TokenType::LookaheadEnd, index, PositivetiveLookaheadExpression());
			} });
			actions.insert({ TokenType::NegativeLookahead, [this](  int& index)->Ptr<Expression>
			{
				return LookAround(TokenType::LookaheadEnd,  index, NegativeLookaheadExpression());
			} });
			actions.insert({ TokenType::StringHead, [this](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<BeginExpression>();
			} });
			actions.insert({ TokenType::StringTail, [this](  int& index)->Ptr<Expression>
			{
				index += 1;
				return make_shared<EndExpression>();
			} });

			actions.insert({ TokenType::MatchAllSymbol, [](int& index)->Ptr < Expression >
			{
				index += 1;
				return make_shared<CharSetExpression>(false, vector<CharRange>({ { 0, 65535} }));
			} });
			actions.insert({ TokenType::Positionb, [](int& index)->Ptr < Expression >
			{
				// \b匹配 \\w和\\W之间的位置,W和w位置可以交换
				//所以\b == ((?<=\\w)(?=\\W))|((?<=\\W)(?=\\w))
				index += 1;
				auto&& w = make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
				auto&& W = make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
				
				auto&& seq_left = make_shared<SequenceExpression>(
					make_shared<PositiveLookbehindExpression>(w), 
					make_shared<PositivetiveLookaheadExpression>(W));
				auto&& seq_right = make_shared<SequenceExpression>(
					make_shared<PositiveLookbehindExpression>(W), 
					make_shared<PositivetiveLookaheadExpression>(w));
				return make_shared<AlternationExpression>(move(seq_left), move(seq_right));
			} });
			//所以\B == ((?<=\\w)(?=\\w))|((?<=\\W)(?=\\W))

			actions.insert({ TokenType::PositionB, [](int& index)->Ptr < Expression >
			{
				index += 1;
				auto&& w = make_shared<CharSetExpression>(false, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
				auto&& W = make_shared<CharSetExpression>(true, vector<CharRange>({ { L'a', L'z' }, { L'A', L'Z' }, { '0', '9' }, { '_', '_' }, { 0x4E00, 0x9FA5 }, { 0xF900, 0xFA2D } }));
				auto&& seq_left = make_shared<SequenceExpression>(
					make_shared<PositiveLookbehindExpression>(w), 
					make_shared<PositivetiveLookaheadExpression>(w));
				auto&& seq_right = make_shared<SequenceExpression>
					(make_shared<PositiveLookbehindExpression>(W), 
					make_shared<PositivetiveLookaheadExpression>(W));
				return make_shared<AlternationExpression>(move(seq_left), move(seq_right));
			} });
		}
		void InitFirstMap()
		{
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
				TokenType::MatchAllSymbol,
				TokenType::PositionB,
				TokenType::Positionb,
			})) });
			first_map.insert({ TokenType::Express, first_map[TokenType::Factor] });
			first_map.insert({ TokenType::Unit, first_map[TokenType::Factor] });
			first_map.insert({ TokenType::Alter, first_map[TokenType::Factor] });
		}
	private:
		Ptr<Expression> Alter(int& index, const int end_index)
		{
			auto&& current_type = tokens->at(index).type;
			if(first_map[TokenType::Alter]->find(current_type) != first_map[TokenType::Alter]->end())
			{
				auto&& left = Unit(index, end_index);
				if(index < end_index)
				{
					if(tokens->at(index).type == TokenType::Alternation)
					{
						index += 1;
						auto&& right = Alter(index, end_index);
						return make_shared<AlternationExpression>(left, right);
					}
				}
				return move(left);
			}
			else
			{
				throw exception("expect symbol not in first[Alter]");
			}
		}
		Ptr<Expression> Unit(int& index, const int end_index)
		{
			if(first_map[TokenType::Unit]->find(tokens->at(index).type) != first_map[TokenType::Unit]->end())
			{
				auto&& left = Express(index, end_index);
				if(index < end_index)
				{
					if(first_map[TokenType::Unit]->find(tokens->at(index).type) != first_map[TokenType::Unit]->end())
					{
						auto&& right = Unit(index, end_index);
						return make_shared<SequenceExpression>(left, right);
					}
				}
				return move(left);
			}
			else
			{
				throw exception("expect symbol not in fist[Unit]");
			}
		}
		Ptr<Expression> Factor(int& index)
		{
			auto&& current_type = tokens->at(index).type;
			if(first_map[TokenType::Factor]->find(current_type) != first_map[TokenType::Factor]->end())
			{
				return actions[current_type]( index);
			}
			else
			{
				throw exception("expect symbol not in fist[Factor]");
			}
		}
		Ptr<Expression> Express(int& index, const int end_index)
		{
			auto&& current_type = tokens->at(index).type;
			if(first_map[TokenType::Express]->find(current_type) != first_map[TokenType::Express]->end())
			{
				auto&& factor = Factor(index);
				if(index < end_index)
				{
					current_type = tokens->at(index).type;
					if(first_map[TokenType::Loop]->find(current_type) != first_map[TokenType::Loop]->end())
					{
						return loop_actions[current_type](factor, index);
					}
				}
				return move(factor);
			}
			else
			{
				throw exception("expect symbol not in fist[Express]");
			}
		}

		Ptr<Expression> CaptureRight(int& index, const int end_index)
		{
			auto&& result = make_shared<CaptureExpression>();
			wstring name;
			if(tokens->at(index).type == TokenType::Named)
			{
				name = Named(index);
				index += 1;
			}
			else
			{
				capture_count++;
				name = to_wstring(capture_count);
			}

			if(first_map[TokenType::Alter]->find(tokens->at(index).type) != first_map[TokenType::Alter]->end())
			{
				auto&& alter = Alter(index, end_index);
				auto&& result = make_shared<CaptureExpression>(name, alter);
				
				return move(result);
			}
			else
			{
				throw exception("expect symbol not in fist[CaptureRight]");
			}
		}
		wstring Named(const int index)
		{
			return pattern.substr(tokens->at(index).position.min, tokens->at(index).position.max - tokens->at(index).position.min);
		}
		template<typename Type>
		Ptr<Expression> LookAround(const TokenType end_type,  int& index, Type)
		{
			auto end_index = index;
			index += 1;
			auto&& current_type = tokens->at(end_index).type;
			while(current_type != end_type)
			{
				end_index += 1;
				current_type = tokens->at(end_index).type;
			}
			auto&& result = make_shared<Type>();
			result->expression = this->Alter(index, end_index);
			index = end_index + 1;
			return move(result);
		}
	};
}
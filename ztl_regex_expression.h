#pragma once
#include "ztl_regex_data.h"
#include "ztl_regex_automachine.h"
namespace ztl
{
	//////////////////////////////////////////////////////////////////////////
	// ǰ������
	//////////////////////////////////////////////////////////////////////////
	class IRegexAlogrithm;
	class AutoMachine;
	//////////////////////////////////////////////////////////////////////////
	//		���ʽ����
	//////////////////////////////////////////////////////////////////////////
	class Expression :public enable_shared_from_this < Expression >
	{
	public:
		Expression()
		{
		}

		~Expression()
		{
		}

		virtual void Apply(IRegexAlogrithm& algorithm) = 0;
		bool IsEqual(Ptr<Expression>& target);
		Ptr<vector<CharRange>> GetCharSetTable();
		void SetTreeCharSetOrthogonal(Ptr<vector<CharRange>>& table);
		AutoMachine::StatesType BuildEpsilonNFA(AutoMachine* target);
	private:
		void BuildOrthogonal(Ptr<vector<int>>&target);
	};

	//�ַ�����
	class CharSetExpression : public Expression
	{
	public:
		bool reverse;
		vector<CharRange> range;
	public:
		CharSetExpression() = default;

		CharSetExpression(const bool _reverse, const vector<CharRange>& _range) :reverse(_reverse), range(_range)
		{
		}
		~CharSetExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	//��ͨ�ַ�
	class NormalCharExpression : public Expression
	{
	public:
		CharRange range;
	public:
		NormalCharExpression() = default;
		NormalCharExpression(const CharRange& _range) :range(_range)
		{
		}

		~NormalCharExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};

	//ѭ��
	class LoopExpression : public Expression
	{
	public:
		Ptr<Expression>	expression;
		int begin;
		int end;
		bool greedy;
	public:
		LoopExpression() = default;

		LoopExpression(const Ptr<Expression>& _expression, const int _begin = -1, const int _end = -1, const bool _greedy = false)
			:expression(_expression), begin(_begin), end(_end), greedy(_greedy)
		{
		}
		~LoopExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};

	class SequenceExpression : public Expression
	{
	public:
		Ptr<Expression> left;
		Ptr<Expression>	right;
	public:
		SequenceExpression() = default;
		SequenceExpression(const Ptr<Expression>& _left, const Ptr<Expression>& _right) :left(_left), right(_right)
		{
		}
		~SequenceExpression()
		{
		}
	
		void Apply(IRegexAlogrithm& algorithm);

	};

	class AlternationExpression : public Expression
	{
	public:
		Ptr<Expression> left;
		Ptr<Expression>	right;
	public:
		AlternationExpression() = default;

		AlternationExpression(const Ptr<Expression>& _left, const Ptr<Expression>& _right) :left(_left), right(_right)
		{
		}
		~AlternationExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};

	class BeginExpression : public Expression
	{
	public:
		BeginExpression() = default;

		~BeginExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);
		
	};

	class EndExpression : public Expression
	{
	public:
		EndExpression() = default;

		~EndExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};

	class CaptureExpression : public Expression
	{
	public:
		wstring			name;
		Ptr<Expression> expression;
	public:
		CaptureExpression() = default;

		CaptureExpression(const wstring& _name, const Ptr<Expression>& _expression) :name(_name), expression(_expression)
		{
		}
		~CaptureExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	//�ǲ�����
	class NoneCaptureExpression : public Expression
	{
	public:
		Ptr<Expression> expression;
	public:
		NoneCaptureExpression() = default;

		NoneCaptureExpression(const Ptr<Expression>& _expression) : expression(_expression)
		{
		}
		~NoneCaptureExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	class BackReferenceExpression : public Expression
	{
	public:
		wstring name;
	public:
		BackReferenceExpression() = default;

		BackReferenceExpression(const wstring& _name) :name(_name)
		{
		}

		~BackReferenceExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);
	};

	class NegativeLookbehindExpression : public Expression
	{
	public:
		Ptr<Expression> expression;
	public:
		NegativeLookbehindExpression() = default;
		NegativeLookbehindExpression(const Ptr<Expression>& _expression) : expression(_expression)
		{
		}
		~NegativeLookbehindExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	class PositiveLookbehindExpression : public Expression
	{
	public:
		Ptr<Expression> expression;
	public:
		PositiveLookbehindExpression() = default;
		PositiveLookbehindExpression(const Ptr<Expression>& _expression) : expression(_expression)
		{
		}
		~PositiveLookbehindExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	class NegativeLookaheadExpression : public Expression
	{
	public:
		Ptr<Expression> expression;
	public:
		NegativeLookaheadExpression() = default;
		NegativeLookaheadExpression(const Ptr<Expression>& _expression) : expression(_expression)
		{
		}
		~NegativeLookaheadExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};
	class PositivetiveLookaheadExpression : public Expression
	{
	public:
		Ptr<Expression> expression;
	public:
		PositivetiveLookaheadExpression() = default;
		PositivetiveLookaheadExpression(const Ptr<Expression>& _expression) : expression(_expression)
		{
		}
		~PositivetiveLookaheadExpression()
		{
		}
		void Apply(IRegexAlogrithm& algorithm);

	};

	
}
#include "forward.h"

namespace ztl
{
	class IRegexAlogrithm
	{
	public:
		virtual void Visitor(Ptr<CharSetExpression>& expression) = 0;
		virtual void Visitor(Ptr<NormalCharExpression>& expression) = 0;
		virtual void Visitor(Ptr<LoopExpression>& expression) = 0;
		virtual void Visitor(Ptr<SequenceExpression>& expression) = 0;
		virtual void Visitor(Ptr<AlternationExpression>& expression) = 0;
		virtual void Visitor(Ptr<BeginExpression>& expression) = 0;
		virtual void Visitor(Ptr<EndExpression>& expression) = 0;
		virtual void Visitor(Ptr<CaptureExpression>& expression) = 0;
		virtual void Visitor(Ptr<NoneCaptureExpression>& expression) = 0;
		virtual void Visitor(Ptr<BackReferenceExpression>& expression) = 0;
		virtual void Visitor(Ptr<NegativeLookbehindExpression>& expression) = 0;
		virtual void Visitor(Ptr<PositiveLookbehindExpression>& expression) = 0;
		virtual void Visitor(Ptr<NegativeLookaheadExpression>& expression) = 0;
		virtual void Visitor(Ptr<PositivetiveLookaheadExpression>& expression) = 0;
	};
	template<typename ReturnType, typename ParameterType>
	class RegexAlogrithm : public IRegexAlogrithm
	{
	public:
		ReturnType return_value;
		ParameterType argument;
	public:
		//v大这个Invoke的想法真是精妙!
		ReturnType Invoke(Ptr<Expression>& expression, ParameterType& parameter)
		{
			this->argument = parameter;
			expression->Apply(*this);
			return return_value;
		}
		virtual ReturnType Apply(Ptr<CharSetExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<NormalCharExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<LoopExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<SequenceExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<AlternationExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<BeginExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<EndExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<CaptureExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<NoneCaptureExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<BackReferenceExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<NegativeLookbehindExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<PositiveLookbehindExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<NegativeLookaheadExpression>& expression) = 0;
		virtual ReturnType Apply(Ptr<PositivetiveLookaheadExpression>& expression) = 0;
	public:
		void Visitor(Ptr<CharSetExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<NormalCharExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<LoopExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<SequenceExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<AlternationExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<BeginExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<EndExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<CaptureExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<NoneCaptureExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<BackReferenceExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<NegativeLookbehindExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<PositiveLookbehindExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<NegativeLookaheadExpression>& expression)
		{
			return_value = Apply(expression);
		}
		void Visitor(Ptr<PositivetiveLookaheadExpression>& expression)
		{
			return_value = Apply(expression);
		}
	};
	//特化空返回类型
	template<typename ParameterType>
	class RegexAlogrithm<void, ParameterType> : public IRegexAlogrithm
	{
	public:
		ParameterType argument;
	public:
		//v大这个Invoke的想法真是精妙!
		void Invoke(Ptr<Expression>& expression, ParameterType& parameter)
		{
			this->argument = parameter;
			expression->Apply(*this);
		}
		virtual void Apply(Ptr<CharSetExpression>& expression) = 0;
		virtual void Apply(Ptr<NormalCharExpression>& expression) = 0;
		virtual void Apply(Ptr<LoopExpression>& expression) = 0;
		virtual void Apply(Ptr<SequenceExpression>& expression) = 0;
		virtual void Apply(Ptr<AlternationExpression>& expression) = 0;
		virtual void Apply(Ptr<BeginExpression>& expression) = 0;
		virtual void Apply(Ptr<EndExpression>& expression) = 0;
		virtual void Apply(Ptr<CaptureExpression>& expression) = 0;
		virtual void Apply(Ptr<NoneCaptureExpression>& expression) = 0;
		virtual void Apply(Ptr<BackReferenceExpression>& expression) = 0;
		virtual void Apply(Ptr<NegativeLookbehindExpression>& expression) = 0;
		virtual void Apply(Ptr<PositiveLookbehindExpression>& expression) = 0;
		virtual void Apply(Ptr<NegativeLookaheadExpression>& expression) = 0;
		virtual void Apply(Ptr<PositivetiveLookaheadExpression>& expression) = 0;
	public:
		void Visitor(Ptr<CharSetExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<NormalCharExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<LoopExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<SequenceExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<AlternationExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<BeginExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<EndExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<CaptureExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<NoneCaptureExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<BackReferenceExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<NegativeLookbehindExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<PositiveLookbehindExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<NegativeLookaheadExpression>& expression)
		{
			Apply(expression);
		}
		void Visitor(Ptr<PositivetiveLookaheadExpression>& expression)
		{
			Apply(expression);
		}
	};
	//Equal算法
	class IsEqualAlgorithm : public RegexAlogrithm < bool, Ptr<Expression> >
	{
	public:
		bool Apply(Ptr<CharSetExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<CharSetExpression>(argument);
			if(expect != nullptr)
			{
				sort(expect->range.begin(), expect->range.end());
				sort(expression->range.begin(), expression->range.end());

				if(expect->reverse == expression->reverse &&
					equal(expect->range.begin(), expect->range.end(), expression->range.begin()))
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<NormalCharExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<NormalCharExpression>(argument);
			if(expect != nullptr)
			{
				if(expect->range == expression->range)
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<LoopExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<LoopExpression>(argument);
			if(expect != nullptr)
			{
				if(expect->greedy == expression->greedy &&
					expect->begin == expression->begin &&
					expect->end == expression->end &&
					Invoke(expect->expression, expression->expression))
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<SequenceExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<SequenceExpression>(argument);
			if(expect != nullptr)
			{
				auto&& left = Invoke(expect->left, expression->left);
				auto&& right = Invoke(expect->right, expression->right);

				if(left&&
					right)
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<AlternationExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<AlternationExpression>(argument);
			if(expect != nullptr)
			{
				auto&& left = Invoke(expect->left, expression->left);
				auto&& right = Invoke(expect->right, expression->right);
				if(left&&
					right)
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<BeginExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<BeginExpression>(argument);
			if(expect != nullptr)
			{
				return true;
			}
			return false;
		}
		bool Apply(Ptr<EndExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<EndExpression>(argument);
			if(expect != nullptr)
			{
				return true;
			}
			return false;
		}
		bool Apply(Ptr<CaptureExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<CaptureExpression>(argument);
			if(expect != nullptr)
			{
				auto&& result = Invoke(expect->expression, expression->expression);
				if(expect->name == expression->name && result)
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<NoneCaptureExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<NoneCaptureExpression>(argument);
			if(expect != nullptr)
			{
				return  Invoke(expect->expression, expression->expression);
			}
			return false;
		}
		bool Apply(Ptr<BackReferenceExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<BackReferenceExpression>(argument);
			if(expect != nullptr)
			{
				if(expect->name == expression->name && Invoke(expect->expression, expression->expression))
				{
					return true;
				}
			}
			return false;
		}
		bool Apply(Ptr<NegativeLookbehindExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<NegativeLookbehindExpression>(argument);
			if(expect != nullptr)
			{
				return  Invoke(expect->expression, expression->expression);
			}
			return false;
		}
		bool Apply(Ptr<PositiveLookbehindExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<PositiveLookbehindExpression>(argument);
			if(expect != nullptr)
			{
				return  Invoke(expect->expression, expression->expression);
			}
			return false;
		}
		bool Apply(Ptr<NegativeLookaheadExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<NegativeLookaheadExpression>(argument);
			if(expect != nullptr)
			{
				return  Invoke(expect->expression, expression->expression);
			}
			return false;
		}
		bool Apply(Ptr<PositivetiveLookaheadExpression>& expression)
		{
			auto expect = dynamic_pointer_cast<PositivetiveLookaheadExpression>(argument);
			if(expect != nullptr)
			{
				return  Invoke(expect->expression, expression->expression);
			}
			return false;
		}
	};

	//字符集正交化算法
	//输入语法树
	//输出正交化后的表形式 vector<CharRange>每个CharRange都是互不相交的区间,并且是[,);
	class BuildOrthogonalAlgorithm : public RegexAlogrithm < void, Ptr<vector<int>> >
	{
	public:
		void Apply(Ptr<CharSetExpression>& expression)
		{
			for(auto&& Iter : expression->range)
			{
				if (Iter.max == Iter.min)
				{
					this->argument->emplace_back(Iter.min);
				}
				else
				{
					this->argument->emplace_back(Iter.min);
					this->argument->emplace_back(Iter.max);
				}
			}
		}
		void Apply(Ptr<NormalCharExpression>& expression)
		{
			this->argument->emplace_back(expression->range.min);
		}
		void Apply(Ptr<LoopExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<SequenceExpression>& expression)
		{
			this->Invoke(expression->left,this->argument);
			this->Invoke(expression->right, this->argument);
		}
		void Apply(Ptr<AlternationExpression>& expression)
		{
			this->Invoke(expression->left, this->argument);
			this->Invoke(expression->right, this->argument);
		}
		void Apply(Ptr<BeginExpression>& expression)
		{

		}
		void Apply(Ptr<EndExpression>& expression)
		{
		}
		void Apply(Ptr<CaptureExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<NoneCaptureExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<BackReferenceExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<NegativeLookbehindExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<PositiveLookbehindExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<NegativeLookaheadExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<PositivetiveLookaheadExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
	};
	//修改语法树的全部字符集为正规化后的字符集
	class SetOrthogonalAlgorithm : public RegexAlogrithm < void, Ptr<vector<CharRange>> >
	{
	public:
		void Apply(Ptr<CharSetExpression>& expression)
		{
			vector<CharRange> result;
			auto& range = expression->range;
			for (auto&& iter : range)
			{
				for_each(this->argument->begin(), this->argument->end(), [&result, &iter](const CharRange& element)
				{
					if (element.min>=iter.min&&element.max<=iter.max)
					{
						result.emplace_back(element);
					}
				});
			}
			range = move(result);
		}
		void Apply(Ptr<NormalCharExpression>& expression)
		{

		}
		void Apply(Ptr<LoopExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<SequenceExpression>& expression)
		{
			this->Invoke(expression->left, this->argument);
			this->Invoke(expression->right, this->argument);
		}
		void Apply(Ptr<AlternationExpression>& expression)
		{
			this->Invoke(expression->left, this->argument);
			this->Invoke(expression->right, this->argument);
		}
		void Apply(Ptr<BeginExpression>& expression)
		{

		}
		void Apply(Ptr<EndExpression>& expression)
		{
		}
		void Apply(Ptr<CaptureExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<NoneCaptureExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<BackReferenceExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
		void Apply(Ptr<NegativeLookbehindExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<PositiveLookbehindExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<NegativeLookaheadExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);

		}
		void Apply(Ptr<PositivetiveLookaheadExpression>& expression)
		{
			this->Invoke(expression->expression, this->argument);
		}
	};

	void Expression::BuildOrthogonal(Ptr<vector<int>>&target)
	{
		assert(target->size() == 0);
		return BuildOrthogonalAlgorithm().Invoke(shared_from_this(), target);
	}
	bool Expression::IsEqual(Ptr<Expression>& target)
	{
		return IsEqualAlgorithm().Invoke(shared_from_this(), target);
	}
	Ptr<vector<CharRange>> Expression::GetCharSetTable()
	{
		auto&&  result(make_shared<vector<CharRange>>());
		auto&& charset = make_shared<vector<int>>();
		BuildOrthogonal(charset);
		charset->push_back(0);
		charset->emplace_back(65535);
		sort(charset->begin(), charset->end());
		charset->erase(unique(charset->begin(), charset->end()),charset->end());

		for(auto i = 0; i < charset->size() - 1;i++)
		{
			auto&& left = charset->at(i);
			auto&& right = charset->at(i+1);
			result->emplace_back(CharRange(left, left));
			if (right - left >= 2)
			{
				result->emplace_back(CharRange(left+1, right-1));
			}
		}
		result->emplace_back(CharRange(65535, 65535));
 		return move(result);
	}
	void Expression::SetTreeCharSetOrthogonal(Ptr<vector<CharRange>>& target)
	{
		return SetOrthogonalAlgorithm().Invoke(shared_from_this(), target);
	}
	void CharSetExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<CharSetExpression>(shared_from_this()));
	};
	void NormalCharExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<NormalCharExpression>(shared_from_this()));
	};
	void LoopExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<LoopExpression>(shared_from_this()));
	};
	void SequenceExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<SequenceExpression>(shared_from_this()));
	};
	void AlternationExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<AlternationExpression>(shared_from_this()));
	};
	void BeginExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<BeginExpression>(shared_from_this()));
	};
	void EndExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<EndExpression>(shared_from_this()));
	};
	void CaptureExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<CaptureExpression>(shared_from_this()));
	};
	void NoneCaptureExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<NoneCaptureExpression>(shared_from_this()));
	};
	void BackReferenceExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<BackReferenceExpression>(shared_from_this()));
	};
	void NegativeLookbehindExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<NegativeLookbehindExpression>(shared_from_this()));
	};
	void PositiveLookbehindExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<PositiveLookbehindExpression>(shared_from_this()));
	};
	void NegativeLookaheadExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<NegativeLookaheadExpression>(shared_from_this()));
	};
	void PositivetiveLookaheadExpression::Apply(IRegexAlogrithm& algorithm)
	{
		algorithm.Visitor(dynamic_pointer_cast<PositivetiveLookaheadExpression>(shared_from_this()));
	};
}
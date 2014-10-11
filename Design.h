/*
������ơ�
CharSetExpression
NormalCharExpression
LoopExpression
SequenceExpression
AlternationExpression
BeginExpression
EndExpression
CaptureExpression
NoneCaptureExpression
BackReferenceExpression
NegativeLookbehindExpression
PositiveLookbehindExpression
NegativeLookaheadExpression
PositivetiveLookaheadExpression


Functions:
ParseRegexExpression            �����ַ�������ΪRegexExpression��������﷨�����������쳣
Look Around
(?<=Expression)
����϶����ӣ���ʾ����λ������ܹ�ƥ��Expression
(?<!Expression)
����񶨻��ӣ���ʾ����λ����಻��ƥ��Expression
(?=Expression)
˳��϶����ӣ���ʾ����λ���Ҳ��ܹ�ƥ��Expression
(?!Expression)
˳��񶨻��ӣ���ʾ����λ���Ҳ಻��ƥ��Expression

Quantifier
*
ƥ����һ��Ԫ����λ��Ρ�
+
ƥ����һ��Ԫ��һ�λ��Ρ�

?
ƥ����һ��Ԫ����λ�һ�Ρ�

{ n }
ƥ����һ��Ԫ��ǡ�� n �Ρ�

{ n ,}
ƥ����һ��Ԫ������ n �Ρ�

{ n , m }
ƥ����һ��Ԫ������ n �Σ��������� m �Ρ�

*?
ƥ����һ��Ԫ����λ��Σ��������������١�

+?
ƥ����һ��Ԫ��һ�λ��Σ��������������١�

??
ƥ����һ��Ԫ����λ�һ�Σ��������������١�

{ n }?
ƥ��ǰ��Ԫ��ǡ�� n �Ρ�

{ n ,}?
ƥ����һ��Ԫ������ n �Σ��������������١�

{ n , m }?
ƥ����һ��Ԫ�صĴ������� n �� m ֮�䣬�������������١�

//////////////////////////////////////////////////////////////////////////
�ķ�

ע�� "(""#"...")"
Alert = Unit "|" Alert
Unit��
Unit  = Express Unit | Express
Express = Factor Loop | Factor
Loop = ��{�� Number ��}��
= ��{�� Number ������ ��}��
= ��{��Number ������ Number ��}��
= ��{�� Number ��}?��
= ��{�� Number ������ ��}?��
= ��{��Number ������ Number ��}?��
= "*"
= "?"
= "+"
= "*?"
= "??"
= "+?"
Factor	= ������ Alert ������
= ����<�� Name ">" Alert ������
= "(?:" Alert ")"
= "(?=" Alert ")"
= "(?!" Alert ")"
= "(?<" Alert ")"
= "(?<!"Alert ")"
= "$"
= "^"
= Backreference
= CharSet
= NormalChar

CharSet = "[^" CharSetCompnent "]" |"[" CharSetCompnent "]" | Char | "\X"
CharSetCompnent = CharUnit CharSetCompnent | CharUnit
CharUnit = Char "-" Char | Char

Backreference = "\k" <�� Name ">" | "\" Number

Note = "(#" ... ")"

//////////////////////////////////////////////////////////////////////////
�ķ���������ʼ�ͽ�β,��β�Ϳ�ʼ�ɴʷ���������.
�ʷ���������:
���� ԭʼ��
����: ��string�г�tokenlist��ȥ��ע�ͺ�ת��

//////////////////////////////////////////////////////////////////////////
enum TokenType
{
NormalChar,
BackReference,
Named,
Number,
LoopBegin,
LoopEndGreedy,
LoopEnd,
Comma,
ChoseLoop,
ChoseLoopGreedy,
PositiveLoop,
PositiveLoopGreedy,
KleeneLoop,
KleeneLoopGreedy,
StringHead,
StringTail,
CharSet,
CharSetReverse,
CharSetEnd,
CharSetW,
CharSetw,
CharSetS,
CharSets,
CharSetD,
CharSetd,
PositionB,
Positionb,
MatchAllSymbol,
Component,
CaptureBegin,
CaptureEnd,
PositivetiveLookahead,
NegativeLookahead,
PositiveLookbehind,
NegativeLookbehind,
LookaheadEnd,
LookbehindEnd,
Alternation,
}
struct RegexToken
{
TokenType		type
CharRange		position
}

*/
/*
Code
����:
д��lex 4Сʱ
��Ԫ���� 4Сʱ.
�Դ�������: ��ֵ�crash lambda��&Ϊ������������. �ѽ��
���ڱ����ò����lambda A,��lambda B C D�ڵ���,����C D������,��B��crash��.������Debug�²����������. �ѽ��
���ڲ��������������Щ����������.lambda�ڲ���thisò���ǿ�������ʽ- -�ַ������ݶ�crash��.Ȼ��û��Ȼ����,ֻ����lambda���¿���regexlex����subexpression.����ֱ����lambda�ڵ���parseringpattern����. �ѽ��
wstring��string��ת������ std::exception��֧��wstring�ַ��� �����ȼ�.
parser�Ķ������first��Ӧ�ø�Ϊstatic��.ÿ�ζ�Ҫ���쿪��̫��
finalset ��Ӧ����set����,Ӧ��ֻ��һ��
unro set��hashû��ϸ����
*/
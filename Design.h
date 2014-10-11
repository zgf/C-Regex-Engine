/*
引擎设计。
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
ParseRegexExpression            ：将字符串分析为RegexExpression对象，如果语法有问题则抛异常
Look Around
(?<=Expression)
逆序肯定环视，表示所在位置左侧能够匹配Expression
(?<!Expression)
逆序否定环视，表示所在位置左侧不能匹配Expression
(?=Expression)
顺序肯定环视，表示所在位置右侧能够匹配Expression
(?!Expression)
顺序否定环视，表示所在位置右侧不能匹配Expression

Quantifier
*
匹配上一个元素零次或多次。
+
匹配上一个元素一次或多次。

?
匹配上一个元素零次或一次。

{ n }
匹配上一个元素恰好 n 次。

{ n ,}
匹配上一个元素至少 n 次。

{ n , m }
匹配上一个元素至少 n 次，但不多于 m 次。

*?
匹配上一个元素零次或多次，但次数尽可能少。

+?
匹配上一个元素一次或多次，但次数尽可能少。

??
匹配上一个元素零次或一次，但次数尽可能少。

{ n }?
匹配前导元素恰好 n 次。

{ n ,}?
匹配上一个元素至少 n 次，但次数尽可能少。

{ n , m }?
匹配上一个元素的次数介于 n 和 m 之间，但次数尽可能少。

//////////////////////////////////////////////////////////////////////////
文法

注释 "(""#"...")"
Alert = Unit "|" Alert
Unit；
Unit  = Express Unit | Express
Express = Factor Loop | Factor
Loop = “{” Number “}”
= “{” Number “，” “}”
= “{”Number “，” Number “}”
= “{” Number “}?”
= “{” Number “，” “}?”
= “{”Number “，” Number “}?”
= "*"
= "?"
= "+"
= "*?"
= "??"
= "+?"
Factor	= “（” Alert “）”
= “（<” Name ">" Alert “）”
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

Backreference = "\k" <” Name ">" | "\" Number

Note = "(#" ... ")"

//////////////////////////////////////////////////////////////////////////
文法不处理串开始和结尾,结尾和开始由词法分析处理.
词法分析功能:
输入 原始串
处理: 将string切成tokenlist并去除注释和转义

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
问题:
写完lex 4小时
单元测试 4小时.
仍存在问题: 奇怪的crash lambda改&为拷贝就正常了. 已解决
对于被引用捕获的lambda A,在lambda B C D内调用,可能C D内正常,在B上crash了.而且在Debug下才有这个问题. 已解决
对于捕获组和零宽断言有些性能上问题.lambda内捕获this貌似是拷贝的形式- -字符串内容都crash了.然后没有然后了,只能在lambda内新开个regexlex处理subexpression.不能直接在lambda内调用parseringpattern函数. 已解决
wstring到string的转换问题 std::exception不支持wstring字符串 低优先级.
parser的动作表和first表应该改为static的.每次都要构造开销太大
finalset 不应该是set集合,应该只有一个
unro set的hash没详细考虑
*/
#pragma once
#include "forward.h"
namespace ztl
{
	class RegexMatchResult
	{
	public:
		//�������ʽƥ��ɹ���õ���������
		unordered_map<wstring, wstring> group;
		//���ʽƥ����
		wstring matched;
	};
	
	class RegexInterpretor
	{
	private:
		wstring pattern;
		vector<RegexControl> control;
		Ptr<AutoMachine> machine;
		//ƥ��������ʲô�ط���ʼ
		int match_start = 0;
		pair<State*,State*> nfa;
	public:
		RegexInterpretor() =delete;
		RegexInterpretor(const wstring& pattern,const vector<RegexControl>& control);
	public:
		//��ָ������ʼλ�ÿ�ʼ���������ַ���������������ʽ�ĵ�һ��ƥ������ҽ�����ָ���������ַ���
		const vector<RegexMatchResult> 	Match(wstring& input,const int start = 0);
		//��ָ������ʼλ�ÿ�ʼ���ж������ַ������Ƿ����������ʽ�ĵ�һ��ƥ����
		bool IsMatch(wstring& input,const int index = 0);
		//���ַ����е�ָ����ʼλ�ÿ�ʼ����ָ���������ַ���������������ʽ������ƥ���
		const vector<RegexMatchResult> 	Matches(wstring& input, int start = 0);
		void Replace(wstring& input, wstring& repalce, int start);
	public:

	};
}
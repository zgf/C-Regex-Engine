#include "forward.h"
#include "ztl_regex_interpretor.h"
namespace ztl
{
	void TestAllComponent();
}
void TestRegexByShell()
{
	std::wstring input;
	std::wstring pattern;
	wchar_t buff[1000];
	do 
	{
		std::wcout << L"��������Դ�:" << std::endl;
		std::wcin.getline(buff, 1000);

		input = buff;
		std::wcout << L"����ģʽ��:" << std::endl;
		std::wcin.getline(buff, 1000);
		pattern = buff;
		ztl::RegexInterpretor interpretor(pattern);
		auto result = interpretor.Matches(input, input.begin());
		if(!result.empty())
		{
			for(auto& item : result)
			{
				std::wcout << L"ƥ��λ��:		" << item.start << std::endl;
				std::wcout << L"ƥ�䳤��:		" << item.length << std::endl;
				std::wcout << L"ƥ���ַ�:		" << item.matched << std::endl;
				if(!item.group.empty())
				{
					for(auto& element : item.group)
					{
						std::wcout << L"����������:" << std::endl;
						std::wcout << L"����:" << element.first << std::endl;
						std::wcout << L"��������:		" << element.second.content << std::endl;
						std::wcout << L"����λ��:		" << element.second.position << std::endl;
						std::wcout << L"���񳤶�:		" << element.second.length << std::endl;
					}

				}
				if(!item.anonymity_group.empty())
				{
					auto count = 0;
					for(auto& element : item.anonymity_group)
					{
						if(element.length != 0)
						{
							std::wcout << L"������:		" << std::endl;
							std::wcout << L"����:		" << std::to_wstring(count) << std::endl;
							std::wcout << L"��������:		" << element.content << std::endl;
							std::wcout << L"����λ��:		" << element.position << std::endl;
							std::wcout << L"���񳤶�:		" << element.length << std::endl;
						}

						count++;
					}
				}
			}
		}
		else
		{
			std::wcout << L"û��ƥ��" << std::endl;
		}
		std::wcout << L"�Ƿ����:(y/n)" << std::endl;
		std::wcin.getline(buff, 1000);
		
	}
	while(buff[0] == L'y');
}
int main()
{
	std::locale::global(std::locale(""));
	ztl::TestAllComponent();
	return 0;
}
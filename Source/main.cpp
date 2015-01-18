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
		std::wcout << L"输入待测试串:" << std::endl;
		std::wcin.getline(buff, 1000);

		input = buff;
		std::wcout << L"输入模式串:" << std::endl;
		std::wcin.getline(buff, 1000);
		pattern = buff;
		ztl::RegexInterpretor interpretor(pattern);
		auto result = interpretor.Matches(input, input.begin());
		if(!result.empty())
		{
			for(auto& item : result)
			{
				std::wcout << L"匹配位置:		" << item.start << std::endl;
				std::wcout << L"匹配长度:		" << item.length << std::endl;
				std::wcout << L"匹配字符:		" << item.matched << std::endl;
				if(!item.group.empty())
				{
					for(auto& element : item.group)
					{
						std::wcout << L"命名捕获组:" << std::endl;
						std::wcout << L"名称:" << element.first << std::endl;
						std::wcout << L"捕获内容:		" << element.second.content << std::endl;
						std::wcout << L"捕获位置:		" << element.second.position << std::endl;
						std::wcout << L"捕获长度:		" << element.second.length << std::endl;
					}

				}
				if(!item.anonymity_group.empty())
				{
					auto count = 0;
					for(auto& element : item.anonymity_group)
					{
						if(element.length != 0)
						{
							std::wcout << L"捕获组:		" << std::endl;
							std::wcout << L"名称:		" << std::to_wstring(count) << std::endl;
							std::wcout << L"捕获内容:		" << element.content << std::endl;
							std::wcout << L"捕获位置:		" << element.position << std::endl;
							std::wcout << L"捕获长度:		" << element.length << std::endl;
						}

						count++;
					}
				}
			}
		}
		else
		{
			std::wcout << L"没有匹配" << std::endl;
		}
		std::wcout << L"是否继续:(y/n)" << std::endl;
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
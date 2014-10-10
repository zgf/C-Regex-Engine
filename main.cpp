#include "forward.h"
#include "ztl_regex_unittest.h"
int main()
{
	std::locale::global(std::locale(""));
	ztl::TestAllComponent();
	auto i = std::make_shared<int>(3);
	
	return 0;
}
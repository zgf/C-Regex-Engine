#include "forward.h"
#include "ztl_regex_unittest.h"
int main()
{
	ztl::TestAllComponent();
	auto i = std::make_shared<int>(3);
	auto p = i.get();
	*p = 5;
	return 0;
}
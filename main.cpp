#include "forward.h"
namespace ztl
{
	void TestAllComponent();
}
int main()
{
	std::locale::global(std::locale(""));
	ztl::TestAllComponent();
	return 0;
}
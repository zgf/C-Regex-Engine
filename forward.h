#pragma once
#include <exception>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <numeric>
#include <fstream>
#include "boost/any.hpp"
#include <locale>
#include <deque>
namespace ztl
{
	//////////////////////////////////////////////////////////////////////////
	// ǰ������
	//////////////////////////////////////////////////////////////////////////
	using std::vector;
	using std::shared_ptr;
	using std::make_shared;
	using std::wstring;
	using std::function;
	using std::unordered_map;
	using std::exception;
	using std::unordered_set;
	using std::to_wstring;
	using std::move;
	using std::equal;
	using std::wcout;
	using std::endl;
	using std::make_shared;
	using std::dynamic_pointer_cast;
	using std::wstringstream;
	using std::to_wstring;
	using std::decay;
	using std::enable_shared_from_this;
	using std::equal;
	using std::reverse_iterator;
	using std::weak_ptr;
	template<typename Type>
	using Ptr = shared_ptr < Type > ;
	using boost::any;
	using boost::any_cast;
	using std::pair;
	using std::set_difference;
	using std::iota;
	using std::inserter;
	using std::fstream;
	using std::ofstream;
	using std::string;
	using std::wofstream;
	using std::hash;
	using std::deque;
	using std::unordered_multimap;
	using std::swap;
	
}
#include "ztl_regex_data.h"
#include "ztl_regex_expression.h"
#include "ztl_regex_lex.h"
#include "ztl_regex_parser.h"

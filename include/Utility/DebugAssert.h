#pragma once

#ifdef _DEBUG
#include <iostream>

#define EXC_ASSERT(expression, exception, message) \
if(!expression)\
	throw exception(message);

#define ASSERT(expression, message) \
if(!expression)\
	std::cerr << "Assertion in: " __FILE__ << ", line: " << __LINE__ << " failed. Message: " << message << std::endl;

#else 
#define EXC_ASSERT(...)
#define ASSERT(expression, message)
#endif

#ifndef DEBUGASSERT_H
#define DEBUGASSERT_H

#ifdef _DEBUG
#    include <iostream>
#endif

namespace epp
{

#ifdef _DEBUG

#    define EXC_ASSERT(expression, exception, message) \
        if (!expression)                               \
            throw exception(message);

#    define ASSERT(expression, message) \
        if (!expression)                \
            std::cerr << "Assertion in: " __FILE__ << ", line: " << __LINE__ << " failed. Message: " << message << std::endl;

#else
#    define EXC_ASSERT(...)
#    define ASSERT(expression, message)
#endif
} // namespace epp

#endif // DEBUGASSERT_H
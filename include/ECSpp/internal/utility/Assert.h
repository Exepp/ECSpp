#ifndef EPP_ASSERT_HEADER
#define EPP_ASSERT_HEADER

#include <exception>
#include <string>

namespace epp {
struct AssertFailed : public std::exception {
    AssertFailed() = default;
    AssertFailed(std::string msg) noexcept : str(std::move(msg)) {}
    virtual char const* what() const noexcept override { return str.c_str(); }
    std::string str;
};
} // namespace epp

#define EPP_THROW_ASSERT_FAIL(msg) \
    throw epp::AssertFailed(std::string("Assertion from file ") + __FILE__ + ":" + std::to_string(__LINE__) + " failed: " + msg + "\n")

///  assert always (debug and release) with fail message
#define EPP_ASSERTA_M(expr, msg) \
    if (!(expr))                 \
        EPP_THROW_ASSERT_FAIL(msg);

/// assert always (debug and release)
#define EPP_ASSERTA(expr) EPP_ASSERTA_M((expr), "")


#ifdef EPP_DEBUG
#define EPP_ASSERT_M(expr, msg) EPP_ASSERTA_M((expr), msg)
#define EPP_ASSERT(expr) EPP_ASSERT_M((expr), "")
#else
#define EPP_ASSERT_M(expr, msg)
#define EPP_ASSERT(expr) EPP_ASSERT_M((expr), "")
#endif

#endif //  EPP_ASSERT_HEADER
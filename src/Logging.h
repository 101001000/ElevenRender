#pragma once

#define BOOST_ALL_NO_LIB // Needed in Windows to avoid Boost to add the "w" postfix to "clang15".
#define  _WIN32_WINNT   0x0601

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#if defined (__WIN32__)
#include <windows.h>
#endif
#include <string>

template <typename T, size_t S>
inline constexpr size_t get_file_name_offset(const T(&str)[S], size_t i = S - 1)
{
    return (str[i] == '/' || str[i] == '\\') ? i + 1 : (i > 0 ? get_file_name_offset(str, i - 1) : 0);
}

template <typename T>
inline constexpr size_t get_file_name_offset(T(&str)[1])
{
    return 0;
}

namespace utility {

    template <typename T, T v>
    struct const_expr_value
    {
        static constexpr const T value = v;
    };

}

#define UTILITY_CONST_EXPR_VALUE(exp) ::utility::const_expr_value<decltype(exp), exp>::value

#define LOG(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(),\
        (::boost::log::keywords::severity = ::boost::log::trivial::lvl)) \
        << ::boost::log::add_value("Line", __LINE__) << ::boost::log::add_value("File", &__FILE__[UTILITY_CONST_EXPR_VALUE(get_file_name_offset(__FILE__))])


BOOST_LOG_ATTRIBUTE_KEYWORD(a_timestamp, "TimeStamp", boost::log::attributes::local_clock::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(a_thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type);

namespace logging { void init(); }
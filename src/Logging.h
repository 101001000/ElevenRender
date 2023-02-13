#pragma once

#define BOOST_ALL_NO_LIB // Needed in Windows to avoid Boost to add the "w" postfix to "clang15".
#define  _WIN32_WINNT   0x0601

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <Windows.h>

#define LOG(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(),\
        (::boost::log::keywords::severity = ::boost::log::trivial::lvl)) \
        << ::boost::log::add_value("Line", __LINE__) << ::boost::log::add_value("File", __FILE_NAME__)


BOOST_LOG_ATTRIBUTE_KEYWORD(a_timestamp, "TimeStamp", boost::log::attributes::local_clock::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(a_thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type);

namespace logging { void init(); }
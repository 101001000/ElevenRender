#pragma once

#define  _WIN32_WINNT   0x0601

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>


BOOST_LOG_ATTRIBUTE_KEYWORD(a_timestamp, "TimeStamp", boost::log::attributes::local_clock::value_type);
BOOST_LOG_ATTRIBUTE_KEYWORD(a_thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type);

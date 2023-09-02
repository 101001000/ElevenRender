#include "Logging.h"

inline void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    using namespace boost::log::trivial;
    using namespace boost::log::expressions;

    std::map<unsigned int, unsigned int> windows_colors{ {info, 10}, {warning, 14}, {error, 12}, {fatal, 4}, {debug, 9}, {trace, 8} };

    // TODO: Make the Linux version
#if defined (__WIN32__)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), windows_colors[rec[severity].get()]);
#endif

    // TODO: Find a way to turn the thread_id hex representation to dec.
    auto file_line_formatter = stream << attr<std::string>("File") << ": " << attr<int>("Line");
    strm << rec[a_timestamp] << "[" << rec[a_thread_id] << "] <" << rec[severity] << ">[";
    file_line_formatter(rec, strm);
    strm << "] -> " << rec[smessage];
}

void logging::init() {
    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
    boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(stream);
    sink->set_formatter(&coloring_formatter);
    boost::log::core::get()->add_sink(sink);
    boost::log::add_common_attributes();
}
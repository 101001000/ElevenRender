#include "CommandManager.h"
#include "Logging.h"

//TODO: Move Logging to Logging.h
inline void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
        // Set the color for Windows console.
        // TODO: Make the Linux version
        switch (severity.get())
        {
        case boost::log::trivial::severity_level::info:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
            break;
        case boost::log::trivial::severity_level::warning:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
            break;
        case boost::log::trivial::severity_level::error:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
            break;
        case boost::log::trivial::severity_level::fatal:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
            break;
        case boost::log::trivial::severity_level::debug:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
            break;
        default:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
            break;
        }
    }

    //auto last_word = [](std::string path) {};

    auto file_line_formatter = boost::log::expressions::stream << boost::log::expressions::attr<std::string>("File") << ": " << boost::log::expressions::attr<int>("Line");
    strm << rec[a_timestamp] << " - [" << rec[a_thread_id] << "] [" << rec[boost::log::trivial::severity] << "][";
    file_line_formatter(rec, strm); 
    strm << "] -> " << rec[boost::log::expressions::smessage];
}

int main(int argc, char* argv[]) {

    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(stream);
    sink->set_formatter(&coloring_formatter);

    boost::log::core::get()->add_sink(sink);
    //boost::log::core::get()->add_thread_attribute("File", boost::log::attributes::mutable_constant<std::string>(""));
    //boost::log::core::get()->add_thread_attribute("Line", boost::log::attributes::mutable_constant<int>(0));

    boost::log::add_common_attributes();

    CommandManager cm;
    cm.init();
   
    LOG(info) << "Quitting";

    return 0;
}

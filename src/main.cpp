#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "sycl.h"
#include <conio.h>

#include "Logging.h"

#include "BVH.h"
#include "Camera.h"
#include "Definitions.h"
#include "PostProcessing.h"
#include "Ray.h"
#include "Scene.h"
#include "Texture.hpp"
#include "kernel.h"
#include "CommandManager.h"


inline void coloring_formatter(boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
{
    auto severity = rec[boost::log::trivial::severity];
    if (severity)
    {
        // Set the color
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
        default:
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
            break;
        }
    }

    strm << rec[a_timestamp] << " - [" << rec[a_thread_id] << "] [" << rec[boost::log::trivial::severity] << "]: " << rec[boost::log::expressions::smessage];
}

int main(int argc, char* argv[]) {

    typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());
    sink->locked_backend()->add_stream(stream);
    sink->set_formatter(&coloring_formatter);

    // Register the sink in the logging core
    boost::log::core::get()->add_sink(sink);
    boost::log::add_common_attributes();


    CommandManager cm;
    cm.init();

    BOOST_LOG_TRIVIAL(info) << "Quitting";

    return 0;
}

#pragma once
#include "Managers.h"

class TCPInterface {

public:

    std::unique_ptr<boost::asio::ip::tcp::socket> sock;
    boost::system::error_code error;
    boost::asio::io_context io_context;

    void write_message(Message msg);
    Message read_message();

};


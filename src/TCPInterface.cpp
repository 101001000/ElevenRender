#include "TCPInterface.h"

void TCPInterface::write_message(Message msg) {

    std::string header_str = serialize(Message::msg2json_header(msg));

    if (header_str.size() > MESSAGE_HEADER_SIZE)
        LOG(error) << "TCP header size exceded.";

    // Fill with empty characters to fit header size.
    header_str += std::string(MESSAGE_HEADER_SIZE - header_str.size(), '\0');

    LOG(trace) << "Writting message with header: " << header_str;
    boost::asio::write(*(sock.get()), boost::asio::buffer(header_str, MESSAGE_HEADER_SIZE));

    if (msg.data_size != 0 &&
        msg.data_format != Message::DataFormat::NONE &&
        msg.data != nullptr) {
        LOG(trace) << "Writting additional data: " << msg.data_size << " bytes";
        boost::asio::write(*(sock.get()), boost::asio::buffer((char*)(msg.data), msg.data_size));
    }
}


Message TCPInterface:: read_message() {

    Message msg;
    char input_data[MESSAGE_HEADER_SIZE];

    try {

    boost::asio::read(*sock.get(), boost::asio::buffer(input_data, 1024));

        if (error == boost::asio::error::eof) {
            LOG(info) << "EOF";
            return Message::CloseSession();
        }

        std::string header_str(input_data);
        LOG(trace) << "Reading message with header: " << header_str;

        boost::json::value input_json = boost::json::parse(header_str);
        msg = Message::json2header(input_json.as_object());

        if (msg.data_size != 0) {
            LOG(trace) << "InputManager::read_message() -> reading additional " << msg.data_size << "bytes";
            // TODO: RAII
            msg.data = malloc(msg.data_size);
            boost::asio::read(*sock.get(), boost::asio::buffer(msg.data, msg.data_size));
        }

    }
    catch (std::exception& e) {
        std::cerr << "Error reading message: " << e.what() << std::endl;
    }

    return msg;
}


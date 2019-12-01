//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#include <zmq.h>

#include "HttpRequester.h"

std::string HttpRequester::get_data() {
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_REQ);

    auto connect_result = zmq_connect(requester, m_message.requestLine.tcpUrl.c_str());
    if (connect_result == -1) {
        std::cout << "err connect" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    m_message.headers.at("Host") = m_message.requestLine.url;

    if (m_message.requestLine.uri.empty()) {
        m_message.requestLine.uri = "/";
    }

    auto message = m_message.CreateHttpRequestMsg();

    auto send_res = zmq_send(requester, message.c_str(), message.size() + 1, 0);
    if (send_res == -1) {
        std::cout << "err send_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    std::cout << "after send req " << send_res << std::endl;

    zmq_msg_t response;
    zmq_msg_init(&response);

    auto recv_res = zmq_msg_recv(&response, requester, 0);
    if (recv_res == -1) {
        std::cout << "err recv_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    auto msg_content = zmq_msg_data(&response);

    std::cout << "req msg content" << std::endl << msg_content << std::endl;

    zmq_msg_close(&response);

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return std::string((char*)msg_content);
}

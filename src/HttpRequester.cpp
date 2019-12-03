//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#include <zmq.h>

#include "HttpRequester.h"

std::string HttpRequester::get_data() {
    std::cout << "HttpRequester::get_data()\n";
    void* context = zmq_ctx_new();
    void* requester = zmq_socket(context, ZMQ_STREAM);

    auto connect_result = zmq_connect(requester, m_message.requestLine.tcpUrl.c_str());
    if (connect_result == -1) {
        std::cout << "err connect" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    uint8_t id [256];
    uint8_t raw [256];
    size_t id_size = 256;
    size_t raw_size = 256;

    id_size = zmq_recv (requester, id, 256, 0);
    raw_size = zmq_recv (requester, raw, 256, 0);

    m_message.headers.at("Host") = m_message.requestLine.url;

    if (m_message.requestLine.uri.empty()) {
        m_message.requestLine.uri = "/";
    }

    auto message = m_message.CreateHttpRequestMsg();
    std::cout << "message: [" << message << "]\n";

    zmq_msg_t msg1;
    zmq_msg_init_data(&msg1, (void*)id, id_size, NULL, NULL);
    auto send_res = zmq_msg_send(&msg1, requester, ZMQ_SNDMORE);

    if (send_res == -1) {
        std::cout << "err send_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    zmq_msg_t msg2;
    zmq_msg_init_data(&msg2, (void*)message.c_str(), message.size(), NULL, NULL);
    send_res = zmq_msg_send(&msg2, requester, 0);

    if (send_res == -1) {
        std::cout << "err send_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    std::cout << "after send req " << send_res << std::endl;

    zmq_msg_t response;
    zmq_msg_init(&response);

    // 1-st msg
    auto recv_res = zmq_msg_recv(&response, requester, 0);
    if (recv_res == -1) {
        std::cout << "err recv_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    zmq_msg_t response2;
    zmq_msg_init(&response2);
    // 2-nd msg
    recv_res = zmq_msg_recv(&response2, requester, 0);

    if (recv_res == -1) {
        std::cout << "err recv_res" << std::endl << zmq_strerror(zmq_errno());
        exit(0);
    }

    std::string msg_content{};
    msg_content.assign((char *) zmq_msg_data(&response2), recv_res);

    if (recv_res) {
        std::cout << "req msg content[" <<  msg_content << "]" << std::endl;
    }
    else {
        std::cout << "****No message received\n";
    }

    zmq_msg_close(&response);

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return msg_content;
}

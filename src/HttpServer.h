#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "HttpRequester.h"
#include "HttpRequestMessage.h"
#include "HttpResponseMessage.h"

class HttpServer {
public:
    HttpServer() = default;

    ~HttpServer();

    bool Initialize();

    bool Start();
private:
    // function throws an exception ios_base::failure if file does not exist (or something goes wrong)
    static std::vector<char> readFile(const char *path);

    HttpRequestMessage parseClientRequest(const std::string &str);

    static std::string fileExtensionToContentType(const std::string &ext);

    void ProcessRequest(const HttpRequestMessage &httpRequestMsg, bool isValid) const;

private:
    void *m_socket = nullptr;

    // When receiving a message the first part of it is ZMQ id, the second is a proper message (we need to call zmq_msg_recv 2 times).
    // This member is used to cache ZMQ id, because it needs to be used while sending.
    std::string m_zmqMsgId{};
};
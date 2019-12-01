#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class HttpServer {
public:
    HttpServer() = default;

    ~HttpServer();

    bool Initialize();

    bool Start();

    struct HttpRequestMessage {
        bool IsValid() const;

        void PrintRequestLine() const;

        void PrintHeaders() const;

        struct RequestLine {
            // method should always be uppercase
            std::string method{};
            std::string uri{};
            std::string httpVersion{};
        };

        RequestLine requestLine;

        std::unordered_map<std::string, std::string> headers{};

        // Todo: we should rely on this flag. Currently it's never checked.
        bool isProcessingDone = false;
    };

    struct HttpResponseMessage {
        struct StatusLine {
            std::string httpVersion = "HTTP/1.1";
            std::string codeStatus{};
            std::string reason{}; //OK, DENIED
        };

        StatusLine statusLine{};
        std::unordered_map<std::string, std::string> headers{};
        std::string response{};

        std::string CreateHttpResponseMsg() const;
    };


private:
    // function throws an exception ios_base::failure if file does not exist (or something goes wrong)
    static std::vector<char> readFile(const char *path);

    HttpRequestMessage parseClientRequest(const std::string &str);

    static std::string fileExtensionToContentType(const std::string &ext);

    void ProcessRequest(const HttpServer::HttpRequestMessage &httpRequestMsg, bool isValid) const;

private:
    void *m_socket = nullptr;

    // When receiving a message the first part of it is ZMQ id, the second is a proper message (we need to call zmq_msg_recv 2 times).
    // This member is used to cache ZMQ id, because it needs to be used while sending.
    std::string m_zmqMsgId{};
};
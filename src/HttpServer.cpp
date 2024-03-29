#include "HttpServer.h"

#include <iostream>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <zmq.h>

void handleHardZmqError(const char *functionName) {
    printf("Error occurred during %s: %s\n", functionName, zmq_strerror(errno));
    abort();
}

std::string getContentType(std::string response) {
    std::string defaultContentType = "text/plain";

    std::istringstream iss(response);
    std::string line;

    while (std::getline(iss, line))
    {
        if (line.find("Content-Type") != std::string::npos)
            return line.substr (line.find(":")+2);
    }

    return defaultContentType;
}

HttpServer::~HttpServer() {}

bool HttpServer::Initialize() {
    std::cout << "Initialize" << std::endl;
    void *context = zmq_ctx_new();
    if (context == NULL)
        handleHardZmqError("zmq_ctx_new");

    m_socket = zmq_socket(context, ZMQ_STREAM);
    if (m_socket == NULL)
        handleHardZmqError("zmq_socket");

    std::cout << "zmq_bind\n";
    if (zmq_bind(m_socket, "tcp://127.0.0.1:65280") == -1)
        handleHardZmqError("zmq_bind");

    return true;
}


// GET / HTTP/1.1
// Host: 127.0.0.1:65288
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:60.0) Gecko/20100101 Firefox/60.0
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
bool HttpServer::Start() {
    std::cout << "[Start] called \n" << std::endl;
    unsigned long long loopsAmount = 0;
    while (true) {
        ++loopsAmount;
        std::cout << "**While loop** ( amount: " << loopsAmount << ") \n";
        zmq_msg_t msg;
        zmq_msg_init(&msg);

        // 1-st msg
        int numberOfBytes = zmq_msg_recv(&msg, m_socket, 0);
        if (numberOfBytes == -1)
            handleHardZmqError("zmq_msg_recv");

        const char *peerAddress = zmq_msg_gets(&msg, "Peer-Address");
        m_zmqMsgId.assign((char *) zmq_msg_data(&msg), numberOfBytes);

        // 2-nd msg
        zmq_msg_t msg2;
        zmq_msg_init(&msg2);
        numberOfBytes = zmq_msg_recv(&msg2, m_socket, 0);
        if (numberOfBytes == -1)
            handleHardZmqError("zmq_msg_recv");

        if (numberOfBytes) {
            std::cout << "From :" << peerAddress << "\n"
                      << "1st (zmqIdsMsg) message (size: " << m_zmqMsgId.size() << ") received: " << m_zmqMsgId << "\n";

            std::string properRequestMsg((char *) zmq_msg_data(&msg2), numberOfBytes);
            std::cout << "2nd (properRequestMsg) message (size: " << properRequestMsg.size() << ") received: "
                      << properRequestMsg << "\n";

            // start parsing
            const auto httpRequestMessage = parseClientRequest(properRequestMsg);
            ProcessRequest(httpRequestMessage, httpRequestMessage.IsValid());
        }


        zmq_msg_close(&msg);
    }

    return true;
}

// function throws an exception ios_base::failure if file does not exist (or something goes wrong)
std::vector<char> HttpServer::readFile(const char *path) {
    using namespace std;

    vector<char> result;
    ifstream file(path, ios::in | ios::binary | ios::ate);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    result.resize(file.tellg(), 0);
    file.seekg(0, ios::beg);
    file.read(result.data(), result.size());
    file.close();

    return result;
}

HttpRequestMessage HttpServer::parseClientRequest(const std::string &str) {
    std::cout << "[parseClientRequest]" << std::endl;
    std::istringstream iss{str};

    HttpRequestMessage HttpRequestMessage{};

    for (std::string line; std::getline(iss, line, '\n');) {
        // get rid of /r at the end of the string
        line.erase(line.size() - 1);
        if (line.empty() && HttpRequestMessage.IsValid()) {
            std::cout << "Done processing. " << std::endl;
            HttpRequestMessage.isProcessingDone = true;
            break;
        }

        if (!HttpRequestMessage.IsValid()) {
            // GET / HTTP/1.1
            const auto delimiterPos = line.find(' ');
            if (delimiterPos == std::string::npos) {
                std::cout << "Received wrong format!" << std::endl;
                // send error to server
                return {};
            }
            HttpRequestMessage.requestLine.method = line.substr(0, delimiterPos);
            const auto delimiterPos2 = line.find(' ', delimiterPos + 1);
            if (delimiterPos2 == std::string::npos) {
                HttpRequestMessage.requestLine.uri = line.substr(delimiterPos + 1);
            } else {
                HttpRequestMessage.requestLine.uri = line.substr(delimiterPos + 1, delimiterPos2 - delimiterPos - 1);
                HttpRequestMessage.requestLine.httpVersion = line.substr(delimiterPos2 + 1);
            }
        } else {
            // Host: 127.0.0.1:65288
            const auto delimiterPos = line.find(':');
            if (delimiterPos == std::string::npos) {
                std::cout << "Received wrong format!" << std::endl;
                // send error to server
                return {};
            }
            std::string key = line.substr(0, delimiterPos);
            // +2 because of additional space
            std::string value = line.substr(delimiterPos + 2);
            const auto result = HttpRequestMessage.headers.emplace(key, value);
            if (!result.second)
                std::cout << "Adding packet to map failed " << std::endl;
        }
    }

    HttpRequestMessage.PrintRequestLine();
    // HttpRequestMessage.PrintHeaders();

    return HttpRequestMessage;
}

std::string HttpServer::fileExtensionToContentType(const std::string &ext) {
    std::string contentType{};
    if (ext.compare(".html") == 0)
        contentType.assign("text/html");
    else if (ext.compare(".jpg") == 0)
        contentType.assign("image/jpeg");
    else if (ext.compare(".js") == 0)
        contentType.assign("application/javascript");
    else
        contentType.assign("application/octet-stream");

    //apply correct charset
    contentType.append("; charset=UTF-8");
    return contentType;
}

void HttpServer::ProcessRequest(const HttpRequestMessage &httpRequestMsg, bool isValid) const {
    std::cout << "[ProcessRequest] function \n";

    HttpResponseMessage httpResponseMessage;
    std::stringstream responseStringStream;

    if (!isValid) {
        std::cout << "[ProcessRequest] Request is not valid - send 501 \n";
        httpResponseMessage.statusLine.codeStatus = "501";
        httpResponseMessage.statusLine.reason = "Not Implemented";
        httpResponseMessage.headers.emplace("Content-Type", "text/html; charset=UTF-8");
    } else if (httpRequestMsg.headers.size() > 8000) {
        std::cout << "[ProcessRequest] Headers payload is too big - send 413 \n";
        httpResponseMessage.statusLine.codeStatus = "413";
        httpResponseMessage.statusLine.reason = "Payload Too Large";
        httpResponseMessage.headers.emplace("Content-Type", "text/html; charset=UTF-8");
    } else {
        if (httpRequestMsg.requestLine.method == "CONNECT") {
            std::cout << "Received CONNECT method" << std::endl;

            auto connectRequest = HttpRequestMessage{};
            connectRequest.requestLine = httpRequestMsg.requestLine;
            connectRequest.requestLine.tcpUrl = "tcp://" + httpRequestMsg.requestLine.uri;
            connectRequest.headers = httpRequestMsg.headers;

            auto requester = HttpRequester(connectRequest);
            auto requester_response = requester.get_data();

            responseStringStream << requester_response;
        } else {
            std::cout << "Received non CONNECT method" << std::endl;
            const auto requestedURI = httpRequestMsg.requestLine.uri.substr(1);

            httpRequestMsg.PrintHeaders();

            auto fisRequest = HttpRequestMessage{};
            fisRequest.requestLine = httpRequestMsg.requestLine;
            fisRequest.requestLine.uri = requestedURI;
            fisRequest.requestLine.url = httpRequestMsg.getHeaderValue("Host");
            fisRequest.requestLine.tcpUrl = "tcp://" + httpRequestMsg.getHeaderValue("Host") + ":80";
            fisRequest.headers = httpRequestMsg.headers;

            auto requester = HttpRequester(fisRequest);
            auto requester_response = requester.get_data();

            httpResponseMessage.statusLine.codeStatus = "200";
            httpResponseMessage.statusLine.reason = "OK";
            httpResponseMessage.headers.emplace("Content-Type", getContentType(requester_response));
            httpResponseMessage.headers.emplace("X-Forwarded-For", "[2001:6d8:10:4400::3]:80");
            httpResponseMessage.headers.emplace("Remote Address", "[2001:6d8:10:4400::3]:80");

            responseStringStream << requester_response;
        }
    }

    httpResponseMessage.response = responseStringStream.str();
    httpResponseMessage.headers.emplace("Content-Length", std::to_string(httpResponseMessage.response.size()));

    std::cout << "ZMQ msg ID: [" << m_zmqMsgId << "] \n";
    std::cout << "Send 1st part of the message with ZMQ ID \n";
    if (zmq_send(m_socket, m_zmqMsgId.c_str(), m_zmqMsgId.size(), ZMQ_SNDMORE) == -1) {
        std::cout << "Error occurred during zmq_send(): " << zmq_strerror(errno) << "\n";
        return;
    }

    const auto httpResponseMsg = httpResponseMessage.CreateHttpResponseMsg();

    // 2-nd message - proper HTTP response message
    std::cout << "Send 2nd part of the message with a proper HTTP response \n";
    if (zmq_send(m_socket, httpResponseMsg.c_str(), httpResponseMsg.size(), 0) == -1) {
        std::cout << "Error occurred during zmq_send(): " << zmq_strerror(errno) << "\n";
        return;
    }

    return;
}
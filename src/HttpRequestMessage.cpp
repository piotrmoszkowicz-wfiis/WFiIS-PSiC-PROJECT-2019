//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#include "HttpRequestMessage.h"

bool HttpRequestMessage::IsValid() const {
    return !requestLine.method.empty() && !requestLine.uri.empty();
}

void HttpRequestMessage::PrintRequestLine() const {
    std::cout << "**Printing request line: " << std::endl;
    std::cout << "Method: [" << requestLine.method << "] " << std::endl;
    std::cout << "URI: [" << requestLine.uri << "] " << std::endl;
    std::cout << "HTTP version: [" << requestLine.httpVersion << "] " << std::endl;
}

void HttpRequestMessage::PrintHeaders() const {
    std::cout << "**Printing headers: " << std::endl;
    for (const auto &p : headers)
        std::cout << p.first << " => " << p.second << '\n';
}

std::string HttpRequestMessage::getHeaderValue(std::string key) const {
    std::string value;

    for (const auto &p : headers)
        if(p.first == key)
            return p.second;

    return NULL;
}

std::string HttpRequestMessage::CreateHttpRequestMsg() const {
    std::stringstream ss;
    ss << requestLine.method << " " << requestLine.uri << " " << requestLine.httpVersion << "\r\n";

    for (const auto &header : headers)
        ss << header.first << ": " << header.second << "\r\n";

    ss << "\r\n";
    // TODO: Pass for example POST data
    // ss << response;
    return ss.str();
}
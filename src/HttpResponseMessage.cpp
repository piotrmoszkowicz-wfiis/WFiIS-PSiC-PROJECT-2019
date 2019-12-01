//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#include "HttpResponseMessage.h"

std::string HttpResponseMessage::CreateHttpResponseMsg() const {
    std::stringstream ss;
    ss << statusLine.httpVersion << " " << statusLine.codeStatus << " " << statusLine.reason << "\r\n";

    for (const auto &header : headers)
        ss << header.first << ": " << header.second << "\r\n";

    ss << "\r\n";
    ss << response;
    return ss.str();
}
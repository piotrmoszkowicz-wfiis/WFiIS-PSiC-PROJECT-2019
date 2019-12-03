//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

struct HttpRequestMessage {
    bool IsValid() const;

    void PrintRequestLine() const;

    void PrintHeaders() const;

    std::string getHeaderValue(std::string key) const;

    struct RequestLine {
        // method should always be uppercase
        std::string method{};
        std::string url{};
        std::string tcpUrl{};
        std::string uri{};
        std::string httpVersion{};
    };

    std::string CreateHttpRequestMsg() const;

    RequestLine requestLine;

    std::unordered_map<std::string, std::string> headers{};

    // Todo: we should rely on this flag. Currently it's never checked.
    bool isProcessingDone = false;
};

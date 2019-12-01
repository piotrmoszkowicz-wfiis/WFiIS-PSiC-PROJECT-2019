//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>


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

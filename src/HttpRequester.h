//
// Created by Piotr Moszkowicz on 01/12/2019.
//

#pragma once

#include <string>
#include <unordered_map>

#include "HttpRequestMessage.h"

class HttpRequester {
public:
    explicit HttpRequester(HttpRequestMessage& message) : m_message(message) {}

    std::string get_data();

private:
    HttpRequestMessage m_message;
};


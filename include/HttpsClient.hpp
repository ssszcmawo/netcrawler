#pragma once
#include <iostream>
#include <curl/curl.h>

class HttpsClient
{
public:
    HttpsClient();
   ~HttpsClient();

    std::string get_request(const std::string& url);

private: 
    static size_t write_callback(void* contents,std::size_t size,std::size_t nmemb,void* userp);

private: 
    CURL* curl;

    long timeout_seconds;
    long connection_timeout_seconds;

    std::string user_agent;
    std::string proxy;

    bool follow_redirects;
    int max_redirects;

    char error_buffer[CURL_ERROR_SIZE];
};

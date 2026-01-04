#include "HttpClient.hpp"

HttpClient::HttpClient() : curl(curl_easy_init()),
                            timeout_seconds(10),
                            connection_timeout_seconds(5),
                            user_agent("WebScraper/1.0"),
                            proxy(""),
                            follow_redirects(true),
                            max_redirects(5)
{
    if(!curl)
        throw std::runtime_error("Failed to init CURL");

    curl_easy_setopt(curl,CURLOPT_USERAGENT,user_agent.c_str());
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,timeout_seconds);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,connection_timeout_seconds);

    curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,follow_redirects ? 1 : 0);
    curl_easy_setopt(curl,CURLOPT_MAXREDIRS,max_redirects);

    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,error_buffer);

}

HttpClient::~HttpClient()
{
    
}

size_t HttpClient::write_callback(void* contents,std::size_t size,std::size_t nmemb,void* userp)
{
    std::size_t total = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);

    str->append(static_cast<char*>(contents),total);

    return total;
}

std::string HttpClient::get_request(std::string& url)
{
    if(!curl)
        throw std::runtime_error("CURL not initialized");


    last_error.clear();
    std::string response;

    curl_easy_setopt(curl,CURLOPT_URL,url.c_str());

    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_callback);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&response);

    CURLcode res = curl_easy_perform(curl);

    if(res != CURLE_OK)
    {
        last_error = error_buffer;
        response.clear();
    }

    return response; 
}
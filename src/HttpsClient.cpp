#include "HttpsClient.hpp"
#include "slogger.h"

HttpsClient::HttpsClient() :
    curl(curl_easy_init()),
    timeout_seconds(10),
    connection_timeout_seconds(5),
    user_agent("WebScraper/1.0"),
    proxy(""),
    follow_redirects(true),
    max_redirects(5)
{
    if (!curl)
    {
        LOG_ERROR("[HttpsClient] Failed to initialize CURL");
        throw std::runtime_error("Failed to init CURL");
    }

    LOG_INFO("[HttpsClient] CURL initialized successfully");

    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connection_timeout_seconds);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, follow_redirects ? 1 : 0);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, max_redirects);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);

    LOG_DEBUG("[HttpsClient] CURL options set: timeout=%d, connect_timeout=%d, follow_redirects=%d",
        timeout_seconds,
        connection_timeout_seconds,
        follow_redirects ? 1 : 0);
}

HttpsClient::~HttpsClient()
{
    curl_easy_cleanup(curl);
    LOG_INFO("[HttpsClient] CURL cleaned up");
}

size_t HttpsClient::write_callback(void *contents, std::size_t size, std::size_t nmemb, void *userp)
{
    std::size_t total = size * nmemb;
    std::string *str = static_cast<std::string *>(userp);

    str->append(static_cast<char *>(contents), total);

    return total;
}

std::string HttpsClient::get_request(const std::string &url)
{
    if (!curl)
        throw std::runtime_error("CURL not initialized");

    std::string response;
    long http_code = 0;
    const int max_retries = 3;
    int attempt = 0;

    while (attempt < max_retries)
    {
        response.clear();
        error_buffer[0] = '\0';

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            ++attempt;
            if (attempt < max_retries)
                continue;

            throw std::runtime_error(error_buffer[0] ? error_buffer : curl_easy_strerror(res));
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code >= 400)
        {
            if ((http_code == 429 || http_code == 500 || http_code == 502 || http_code == 503 || http_code == 504) &&
                attempt < max_retries - 1)
            {
                ++attempt;
                continue;
            }

            throw std::runtime_error("HTTP error code: " + std::to_string(http_code));
        }

        break;
    }

    return response;
}

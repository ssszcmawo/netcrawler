#include "CSV.hpp"
#include "HtmlParser.hpp"
#include "HttpsClient.hpp"
#include "ProductXPathConfig.hpp"
#include "Utils.hpp"
#include "slogger.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);
#ifdef DEBUG_BUILD
    init_consoleLog(stdout);
    set_log_level(INFO);
#else
    init_fileLog("log_file", 1024 * 1024, true);
    set_log_level(INFO);
#endif

    HttpsClient client;

    std::string url = "https://www.scrapingcourse.com/ecommerce/";
    std::string document = client.get_request(url);

    std::string filename = "page.html";
    std::ofstream out(filename);
    if (out.is_open())
    {
        out << document;
        out.close();
        std::cout << "page.html saved\n";
    }
    else
    {
        std::cerr << "Could not open page.html\n";
    }

    HtmlParser parser;

    try
    {
        auto config = ProductXPathConfig::from_json_file("products_xpath.json");

        parser.parse(document, config);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to load XPath config: %s", e.what());
    }

    auto &repo = ProductRepository::instance();

    auto filtered = Utils::filter_products_by_price(repo.get_all(), 40.0, 50.0);

    CSV::export_to_csv("filtered_products.csv", filtered);

    auto prod = Utils::find_product_by_name(repo.get_all(), "Ana Running Short");
    if (prod)
        std::cout << prod->name << " " << prod->price << "\n";
    else
        std::cout << "Product not found\n";

    curl_global_cleanup();
    return 0;
}

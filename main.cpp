#include "HtmlParser.hpp"
#include "HttpsClient.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);

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

    HtmlParser parser(document);

    Utils::filter_csv_by_price("products.csv", 40, 50);

    auto prod = Utils::find_product_by_name("products.csv", "Ana Running Short");
    if (prod)
        std::cout << prod->name << " " << prod->price << "\n";
    else
        std::cout << "error\n";

    curl_global_cleanup();

    return 0;
}
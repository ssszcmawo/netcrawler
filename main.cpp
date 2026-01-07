#include <iostream>
#include "HttpsClient.hpp"
#include "HtmlParser.hpp"


int main()
{
    curl_global_init(CURL_GLOBAL_ALL);

    HttpsClient client;

    std::string url = "https://www.scrapingcourse.com/ecommerce/";

    std::string document = client.get_request(url);

    std::string filename = "page.html";

    std::ofstream out(filename);

    if(out.is_open())
    {
        out << document;
        out.close();

        std::cout << "page.html saved\n";
    }else{
        std::cerr << "Could not open page.html\n";
    } 

    HtmlParser parser(document);

    parser.filter_csv_by_price("products.csv",0,200);


    curl_global_cleanup();

    return 0;
}
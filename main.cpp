#include "CSV.hpp"
#include "HtmlParser.hpp"
#include "HttpsClient.hpp"
#include "ProductXPathConfig.hpp"
#include "Utils.hpp"
#include "Crawler.hpp"
#include "slogger.h"
#include <curl/curl.h>
#include <fstream>
#include <iostream>

int main(int argc,char **argv)
{
    curl_global_init(CURL_GLOBAL_ALL);
#ifdef DEBUG_BUILD
    init_consoleLog(stdout);
    set_log_level(INFO);
#else
    init_fileLog("log_file", 1024 * 1024, true);
    set_log_level(INFO);
#endif


    if (argc < 2)
    {
        LOG_ERROR("Usage: ./WebCrawler <path_to_config.json>");
        return 1;
    }


    ProductXPathConfig config;

    try{
      if(!config.from_json_file(argv[1]))
      {
        LOG_ERROR("Could not load json");
        return 1;
      }
    }catch(const std::exception& e)
    {
      LOG_ERROR("Failed to load config: %s", e.what());
      return 1;
    }

    Crawler crawler(config); 

    crawler.run();

    CSV::export_to_csv(
        "products.csv",
        ProductRepository::instance().get_all()
    );

    curl_global_cleanup();
    return 0;
}

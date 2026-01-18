#pragma once
#include "HtmlParser.hpp"
#include "HttpsClient.hpp"
#include "ProductXPathConfig.hpp"
#include <queue>
#include <string>
#include <unordered_set>

class Crawler
{
  public:
    explicit Crawler(ProductXPathConfig &config);
    ~Crawler();

    void run();

  private:
    void crawl_page(const std::string &url);
    void discover_pages(const std::string &html);

  private:
    HttpsClient client_;
    HtmlParser parser_;
    ProductXPathConfig config_;
    std::unordered_set<std::string> pages_visited;
    std::queue<std::string> pages_to_visit;
};

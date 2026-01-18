#include "ProductXPathConfig.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

ProductXPathConfig ProductXPathConfig::from_json_string(const std::string &json_str)
{
    auto j = json::parse(json_str);

    ProductXPathConfig config;
    config.product_item = j["product_item"];
    config.url = j["url"];
    config.image = j["image"];
    config.name = j["name"];
    config.price = j["price"];
    config.first_page = j["first_page"];
    config.page_numbers = j["page_numbers"];

    return config;
}

bool ProductXPathConfig::from_json_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open file: " + path);

    nlohmann::json j;
    file >> j;

    product_item = j.value("product_item", "");
    url          = j.value("url", "");
    image        = j.value("image", "");
    name         = j.value("name", "");
    price        = j.value("price", "");
    first_page   = j.value("first_page", "");
    page_numbers = j.value("page_numbers", "");

    return !product_item.empty() && !first_page.empty();
}

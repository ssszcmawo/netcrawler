#pragma once
#include <string>

struct ProductXPathConfig
{
    std::string product_item;
    std::string url;
    std::string image;
    std::string name;
    std::string price;
    std::string first_page;
    std::string page_numbers;

    ProductXPathConfig from_json_string(const std::string &json_str);
    bool from_json_file(const std::string &path);
};

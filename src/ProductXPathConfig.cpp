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

ProductXPathConfig ProductXPathConfig::from_json_file(const std::string &path)
{
    fs::path json_path = fs::current_path().parent_path() / path;
    if (!fs::exists(json_path))
    {
        throw std::runtime_error("File does not exist" + json_path.string());
    }

    std::ifstream file(json_path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + json_path.string());

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();
    if (content.empty())
        throw std::runtime_error("File is empty: " + json_path.string());

    return from_json_string(content);
}
#pragma once
#include "ProductRepository.hpp"
#include "slogger.h"
#include <optional>
#include <string>
#include <vector>

namespace Utils
{
static inline std::optional<double> parse_price(std::string_view str)
{
    std::string clean;
    bool dot_used = false;

    for (char c : str)
    {
        if (std::isdigit(static_cast<unsigned char>(c)))
            clean += c;

        else if ((c == '.' || c == ',') && !dot_used)
        {
            clean += '.';
            dot_used = true;
        }
    }

    if (clean.empty())
    {
        LOG_DEBUG("[parse_price] Empty numeric value from: %.*s", (int)str.size(), str.data());
        return std::nullopt;
    }
    try
    {
        return std::stod(clean);
    }
    catch (...)
    {
        LOG_ERROR("[parse_price] Failed to convert: %.*s", (int)str.size(), str.data());
        return std::nullopt;
    }
}
std::optional<Product> find_product_by_name(const std::vector<Product> &products, const std::string &name);
std::vector<Product> filter_products_by_price(const std::vector<Product> &products, double min, double max);
} // namespace Utils

#include "Utils.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>

namespace Utils
{

std::optional<Product> find_product_by_name(const std::vector<Product> &products, const std::string &name)
{
    std::string name_lower = name;

    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c)
        { return std::tolower(c); });

    auto it = std::find_if(products.begin(), products.end(), [&name_lower](const Product &p)
        {
          std::string product_name_lower = p.name;

          std::transform(product_name_lower.begin(),product_name_lower.end(),product_name_lower.begin(),
              [](unsigned char c){ return std::tolower(c);});

          return product_name_lower.find(name_lower)  != std::string::npos; });

    if (it != products.end())
        return *it;
    return std::nullopt;
}

std::vector<Product> filter_products_by_price(const std::vector<Product> &products, double min, double max)
{
    std::vector<Product> filtered;
    std::copy_if(products.begin(), products.end(), std::back_inserter(filtered), [&](const Product &p)
        {
                         auto price = parse_price(p.price);
                         return price && *price >= min && *price <= max; });

    std::sort(filtered.begin(), filtered.end(), [](const Product &a, const Product &b)
        { return parse_price(a.price).value_or(0.0) < parse_price(b.price).value_or(0.0); });

    return filtered;
}

} // namespace Utils

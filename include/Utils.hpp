#pragma once
#include "ProductRepository.hpp"
#include <optional>
#include <string>
#include <vector>

namespace Utils
{
std::string escape_csv(const std::string &field);
std::optional<double> parse_price(std::string_view str);
std::string get_csv_dir();
void export_to_csv(const std::string &filename, const std::vector<Product> &products);
std::vector<Product> read_csv(const std::string &filename);
std::optional<Product> find_product_by_name(const std::vector<Product> &products, const std::string &name);
std::vector<Product> filter_products_by_price(const std::vector<Product> &products, double min, double max);
} // namespace Utils
#include "ProductRepository.hpp"
#include "csv.h"
#include <optional>
#include <string>
#include <vector>
namespace CSV
{
std::string escape_csv(const std::string &field);
std::optional<double> parse_price(std::string_view str);
std::string get_csv_dir();
void export_to_csv(const std::string &filename, const std::vector<Product> &products);
std::vector<Product> read_csv(const std::string &filename);
} // namespace CSV
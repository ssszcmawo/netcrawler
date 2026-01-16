#include "Utils.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include "csv.h"
extern "C"
{
#include "slogger.h"
}

namespace fs = std::filesystem;

namespace Utils
{

ProductXPathConfig load_xpath_config()
{
    ProductXPathConfig config;

    fs::path json_path = fs::current_path().parent_path() / "products_xpath.json";

    if (!fs::exists(json_path))
    {
        throw std::runtime_error("Cannot find json config: " + json_path.string());
    }

    std::ifstream file(json_path);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open config file: " + json_path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    content.erase(std::remove_if(content.begin(), content.end(), ::isspace), content.end());

    auto get_value = [&](const std::string &key) -> std::string
    {
        std::string pattern = "\"" + key + "\":\"";
        auto start = content.find(pattern);
        if (start == std::string::npos)
            return "";
        start += pattern.size();
        auto end = content.find("\"", start);
        if (end == std::string::npos)
            return "";
        return content.substr(start, end - start);
    };

    config.product_item = get_value("product_item");
    config.url = get_value("url");
    config.image = get_value("image");
    config.name = get_value("name");
    config.price = get_value("price");

    return config;
}
std::string escape_csv(const std::string &field)
{
    std::string result = "\"";
    for (char c : field)
    {
        if (c == '"')
            result += "\"\"";
        else
            result += c;
    }
    result += "\"";
    return result;
}

std::optional<double> parse_price(std::string_view str)
{
    std::string clean;
    bool dot_used = false;


    for (char c : str)
    {
        if (std::isdigit(static_cast<unsigned char>(c)))
            clean += c;

        else if((c == '.' || c == ',') && !dot_used)
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

std::string get_csv_dir()
{
    fs::path dir = fs::current_path().parent_path() / "csv_files";
    if (!fs::exists(dir))
    {
        if (fs::create_directory(dir))
            LOG_INFO("[Utils] Folder created: %s", dir.string().c_str());
        else
            LOG_ERROR("[Utils] Failed to create folder: %s", dir.string().c_str());
    }
    return dir.string();
}

void export_to_csv(const std::string &filename, const std::vector<Product> &products)
{
    fs::path csv_path = fs::path(get_csv_dir()) / filename;
    std::ofstream csv_file(csv_path);
    if (!csv_file)
    {
        LOG_ERROR("[export_to_csv] Failed to open file: %s", csv_path.string().c_str());
        return;
    }

    csv_file << "url,name,image,price\n";
    for (const auto &p : products)
        csv_file << escape_csv(p.url) << "," << escape_csv(p.name) << "," << escape_csv(p.image) << "," << escape_csv(p.price) << "\n";

    LOG_INFO("[export_to_csv] CSV saved to: %s", csv_path.string().c_str());
}

std::vector<Product> read_csv(const std::string &filename)
{
    std::vector<Product> products;
    fs::path csv_path = fs::path(get_csv_dir()) / filename;

    if (!fs::exists(csv_path))
    {
        LOG_ERROR("[read_csv] CSV file not found: %s", csv_path.string().c_str());
        return products;
    }

    io::CSVReader<4> in(csv_path.string());

    in.read_header(io::ignore_extra_column,"url","name","image","price");

    try
    {
      std::string url,name,image,price;

      while(in.read_row(url,name,image,price))
      {
        if(url.empty() && name.empty() && image.empty() && price.empty()) continue;

        auto price_value = parse_price(price);

        if(!price_value.has_value())
        {
          LOG_DEBUG("[read_csv] Skipping product with invalid price");
          continue;
        }
        
        url = std::string(trim(url));
        name = std::string(trim(name));
        image = std::string(trim(image));

        auto duplicate = std::find_if(products.begin(), products.end(), [&](const Product& p){
            return p.name == name;
            });

        if(duplicate != products.end())
          continue;


        products.push_back(Product{
            url,
            name,
            image,
            price
            });
      }
    }catch(const std::exception& e_row)
    {
      LOG_ERROR("[read_csv] exception while parsing row: %s", e_row.what());
    }

    LOG_INFO("[read_csv] Total products read: %zu", products.size());
    return products;
}

std::optional<Product> find_product_by_name(const std::vector<Product> &products, const std::string &name)
{
    std::string name_lower = name;

    std::transform(name_lower.begin(),name_lower.end(),name_lower.begin(),
        [](unsigned char c){ return std::tolower(c);});
    
    auto it = std::find_if(products.begin(), products.end(), [&name_lower](const Product &p)
        {
          if (p.name.size() != name_lower.size())return false;

          return std::equal(p.name.begin(),p.name.end(),name_lower.begin(),
              [](unsigned char a,unsigned char b){
                  return std::tolower(a) == std::tolower(b);
              });
        });
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

std::string trim(std::string& str)
{
  auto first = std::find_if(str.begin(),str.end(),[](unsigned char c){ return !std::isspace(c);});

  if(first == str.end()) return "";

  auto last = std::find_if(str.rbegin(),str.rend(),
      [](unsigned char c){ return !std::isspace(c);}).base();

  return std::string(first,last);
}
} // namespace Utils

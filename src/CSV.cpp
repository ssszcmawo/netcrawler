#include "CSV.hpp"
#include "Utils.hpp"
#include "slogger.h"
#include <filesystem>
#include <fstream>
#include <unordered_set>

namespace fs = std::filesystem;

namespace CSV
{
static void trim(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c)
                               { return !std::isspace(c); }));

    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c)
                  { return !std::isspace(c); })
                  .base(),
        str.end());
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

    in.read_header(io::ignore_extra_column, "url", "name", "image", "price");

    std::unordered_set<std::string> seen_names;

    try
    {
        std::string url, name, image, price;

        while (in.read_row(url, name, image, price))
        {
            if (url.empty() && name.empty() && image.empty() && price.empty())
                continue;

            trim(url);
            trim(name);
            trim(image);
            trim(price);

            auto price_value = Utils::parse_price(price);

            if (!price_value.has_value())
            {
                LOG_DEBUG("[read_csv] Skipping product with invalid price");
                continue;
            }

            if (!seen_names.insert(name).second)
            {
                LOG_DEBUG("[read_csv] Skipping duplicate name: %s", name.c_str());
                continue;
            }

            products.push_back(Product{
                std::move(url),
                std::move(name),
                std::move(image),
                std::move(price)});
        }
    }
    catch (const std::exception &e_row)
    {
        LOG_ERROR("[read_csv] exception while parsing row: %s", e_row.what());
    }

    LOG_INFO("[read_csv] Total products read: %zu", products.size());
    return products;
}
} // namespace CSV
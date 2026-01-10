#include "HtmlParser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

extern "C"
{
#include "slogger.h"
}

namespace fs = std::filesystem;

namespace Utils
{
std::vector<Product> products;

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

void export_to_csv(const std::string &filename)
{
    fs::path folder_name = fs::current_path().parent_path() / "csv_files";
    if (!fs::exists(folder_name))
    {
        if (fs::create_directory(folder_name))
        {
            LOG_INFO("[export_to_csv] Folder created: %s", folder_name.string().c_str());
        }
        else
        {
            LOG_ERROR("[export_to_csv] Failed to create folder: %s", folder_name.string().c_str());
            return;
        }
    }

    fs::path csv_path = folder_name / filename;
    std::ofstream csv_file(csv_path);

    if (!csv_file)
    {
        LOG_ERROR("[export_to_csv] Failed to open file: %s", csv_path.string().c_str());
        return;
    }

    csv_file << "url,name,image,price\n";

    for (const auto &p : products)
    {
        csv_file << escape_csv(p.url) << ","
                 << escape_csv(p.name) << ","
                 << escape_csv(p.image) << ","
                 << escape_csv(p.price) << "\n";
    }

    csv_file.close();
    LOG_INFO("[export_to_csv] CSV saved to: %s", csv_path.string().c_str());
}

std::vector<Product> read_csv(const std::string &filename)
{
    std::vector<Product> read_products;

    const fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    const fs::path csv_path = csv_files_dir / filename;

    if (!fs::exists(csv_path))
    {
        LOG_ERROR("[read_csv] CSV file not found: %s", csv_path.string().c_str());
        return read_products;
    }

    std::ifstream file(csv_path);
    if (!file.is_open())
    {
        LOG_ERROR("[read_csv] Failed to open CSV file: %s", csv_path.string().c_str());
        return read_products;
    }

    std::string line;
    std::getline(file, line);
    LOG_INFO("[read_csv] Reading CSV: %s", csv_path.string().c_str());

    while (std::getline(file, line))
    {
        std::string url, name, image, price;

        auto read_field = [&](std::string &dest)
        {
            if (line.empty())
                return;
            if (line.front() == '"')
            {
                line.erase(0, 1);
                auto pos = line.find("\"");
                if (pos != std::string::npos)
                {
                    dest = line.substr(0, pos);
                    line.erase(0, pos + 2);
                }
            }
            else
            {
                auto pos = line.find(',');
                if (pos != std::string::npos)
                {
                    dest = line.substr(0, pos);
                    line.erase(0, pos + 1);
                }
                else
                {
                    dest = line;
                    line.clear();
                }
            }
        };

        read_field(url);
        read_field(name);
        read_field(image);
        read_field(price);

        read_products.emplace_back(Product{url, name, image, price});
        LOG_DEBUG("[read_csv] Read product: %s | %s | %s | %s",
            url.c_str(),
            name.c_str(),
            image.c_str(),
            price.c_str());
    }

    LOG_INFO("[read_csv] Total products read: %zu", read_products.size());
    return read_products;
}

std::optional<double> parse_price(std::string_view str)
{
    std::string clean;
    for (char c : str)
    {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.')
            clean += c;
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
    catch (const std::exception &e)
    {
        LOG_ERROR("[parse_price] Failed to convert: %.*s", (int)str.size(), str.data());
        return std::nullopt;
    }
}

void filter_csv_by_price(const std::string &filename, double min, double max)
{
    if (filename.empty())
    {
        LOG_ERROR("[filter_csv_by_price] Filename is empty");
        return;
    }

    auto products_list = read_csv(filename);

    products_list.erase(
        std::remove_if(products_list.begin(), products_list.end(), [&](const Product &p)
            {
                           auto price = parse_price(p.price);
                           if (!price)
                           {
                               LOG_DEBUG("[filter_csv_by_price] Skipping product with invalid price: %s", p.price.c_str());
                               return true;
                           }
                           return *price < min || *price > max; }),
        products_list.end());

    std::sort(products_list.begin(), products_list.end(), [](const Product &a, const Product &b)
        { return parse_price(a.price).value_or(0.0) < parse_price(b.price).value_or(0.0); });

    fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    fs::path csv_path = csv_files_dir / filename;
    std::ofstream out(csv_path);

    if (!out)
    {
        LOG_ERROR("[filter_csv_by_price] Failed to open CSV for writing: %s", csv_path.string().c_str());
        return;
    }

    out << "url,name,image,price\n";
    for (const auto &p : products_list)
    {
        out << escape_csv(p.url) << ","
            << escape_csv(p.name) << ","
            << escape_csv(p.image) << ","
            << escape_csv(p.price) << "\n";
    }

    out.close();
    LOG_INFO("[filter_csv_by_price] Filtered CSV saved: %s", csv_path.string().c_str());
}

std::optional<Product> find_product_by_name(const std::string &filename, const std::string &name)
{
    if (filename.empty())
    {
        LOG_ERROR("[find_product_by_name] Filename is empty");
        return std::nullopt;
    }

    auto products_list = read_csv(filename);

    auto it = std::find_if(products_list.begin(), products_list.end(), [&](const Product &p)
        { return p.name == name; });

    if (it != products_list.end())
    {
        LOG_INFO("[find_product_by_name] Product found: %s", it->name.c_str());
        return *it;
    }

    LOG_DEBUG("[find_product_by_name] Product not found: %s", name.c_str());
    return std::nullopt;
}

const std::vector<Product> &get_products()
{
    return products;
}

} // namespace Utils

void HtmlParser::parse(const std::string &html)
{
    LOG_INFO("[HtmlParser::parse] Parsing started, HTML size: %zu", html.size());
    Utils::products.clear();

    XmlDocWrapper doc(htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR));
    if (!doc)
    {
        LOG_ERROR("[HtmlParser::parse] Failed to parse HTML");
        return;
    }

    XmlXPathContextWrapper context(xmlXPathNewContext(doc));
    if (!context)
    {
        LOG_ERROR("[HtmlParser::parse] Failed to create XPath context");
        return;
    }

    XmlXPathObjectWrapper html_elements(xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>("//li[contains(@class, 'product')]"), context));

    if (!html_elements || !html_elements->nodesetval)
    {
        LOG_ERROR("[HtmlParser::parse] No product elements found in HTML");
        return;
    }

    LOG_INFO("[HtmlParser::parse] Found %d product elements", html_elements->nodesetval->nodeNr);

    for (int i = 0; i < html_elements->nodesetval->nodeNr; ++i)
    {
        xmlNodePtr product_node = html_elements->nodesetval->nodeTab[i];
        if (!product_node)
            continue;

        auto get_text_from_xpath = [&](xmlNodePtr base_node, const char *xpath_expr) -> std::string
        {
            context->node = base_node;
            XmlXPathObjectWrapper obj(xmlXPathEvalExpression(
                reinterpret_cast<const xmlChar *>(xpath_expr),
                context));

            if (!obj || !obj->nodesetval || obj->nodesetval->nodeNr == 0)
                return "";

            xmlNodePtr node = obj->nodesetval->nodeTab[0];
            if (!node)
                return "";

            xmlChar *content = xmlNodeGetContent(node);
            std::string result = content ? reinterpret_cast<char *>(content) : "";
            if (content)
                xmlFree(content);

            return result;
        };

        auto get_attr_from_xpath = [&](const char *xpath_expr, const char *attr) -> std::string
        {
            context->node = product_node;
            XmlXPathObjectWrapper obj(xmlXPathEvalExpression(
                reinterpret_cast<const xmlChar *>(xpath_expr),
                context));

            if (!obj || !obj->nodesetval || obj->nodesetval->nodeNr == 0)
                return "";

            xmlNodePtr node = obj->nodesetval->nodeTab[0];
            if (!node)
                return "";

            xmlChar *value = xmlGetProp(node, reinterpret_cast<const xmlChar *>(attr));
            std::string result = value ? reinterpret_cast<char *>(value) : "";
            if (value)
                xmlFree(value);

            return result;
        };

        std::string url = get_attr_from_xpath(".//a", "href");
        std::string image = get_attr_from_xpath(".//img", "src");
        std::string name = get_text_from_xpath(product_node, ".//a/h2");
        std::string price = get_text_from_xpath(product_node, ".//a/span");

        LOG_DEBUG("[HtmlParser::parse] Product parsed: %s | %s | %s | %s",
            url.c_str(),
            name.c_str(),
            image.c_str(),
            price.c_str());

        Utils::products.push_back(Product{url, name, image, price});
    }

    Utils::export_to_csv("products.csv");
    LOG_INFO("[HtmlParser::parse] Parsing finished, total products: %zu", Utils::products.size());
}

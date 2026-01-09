#include "HtmlParser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

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
            std::cout << "Folder created: " << folder_name << "\n";
        else
        {
            std::cerr << "Failed to create folder: " << folder_name << "\n";
            return;
        }
    }

    fs::path csv_path = folder_name / filename;
    std::ofstream csv_file(csv_path);

    if (!csv_file)
    {
        std::cerr << "Failed to open file: " << csv_path << "\n";
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
    std::cout << "CSV saved to: " << csv_path << "\n";
}

std::vector<Product> read_csv(const std::string &filename)
{
    std::vector<Product> read_products;

    const fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    const fs::path csv_path = csv_files_dir / filename;

    if (!fs::exists(csv_path))
    {
        std::cerr << "CSV file not found: " << csv_path << '\n';
        return read_products;
    }

    std::ifstream file(csv_path);
    if (!file.is_open())
        return read_products;

    std::string line;
    std::getline(file, line);

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
    }

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
        return std::nullopt;

    try
    {
        return std::stod(clean);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to convert: " << str << ": " << e.what() << "\n";
        return std::nullopt;
    }
}

void filter_csv_by_price(const std::string &filename, double min, double max)
{
    if (filename.empty())
        return;

    auto products_list = read_csv(filename);

    products_list.erase(
        std::remove_if(products_list.begin(), products_list.end(), [&](const Product &p)
            {
                auto price = parse_price(p.price);
                if (!price)
                    return true;
                return *price < min || *price > max; }),
        products_list.end());

    std::sort(products_list.begin(), products_list.end(), [](const Product &a, const Product &b)
        { return parse_price(a.price).value_or(0.0) < parse_price(b.price).value_or(0.0); });

    fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    fs::path csv_path = csv_files_dir / filename;
    std::ofstream out(csv_path);

    if (!out)
    {
        std::cerr << "Failed to open CSV for writing: " << csv_path << "\n";
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
    std::cout << "Filtered CSV by price\n";
}

std::optional<Product> find_product_by_name(const std::string &filename, const std::string &name)
{
    if (filename.empty())
        return std::nullopt;

    auto products_list = read_csv(filename);

    auto it = std::find_if(products_list.begin(), products_list.end(), [&](const Product &p)
        { return p.name == name; });

    if (it != products_list.end())
        return *it;

    return std::nullopt;
}

const std::vector<Product> &get_products()
{
    return products;
}

} // namespace Utils

void HtmlParser::parse(const std::string &html)
{
    Utils::products.clear();

    XmlDocWrapper doc(htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR));
    if (!doc)
        return;

    XmlXPathContextWrapper context(xmlXPathNewContext(doc));
    if (!context)
        return;

    XmlXPathObjectWrapper html_elements(xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>("//li[contains(@class, 'product')]"), context));

    if (!html_elements || !html_elements->nodesetval)
        return;

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

        Utils::products.push_back(Product{url, name, image, price});
    }

    Utils::export_to_csv("products.csv");
}

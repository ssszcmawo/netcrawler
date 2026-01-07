#include "HtmlParser.hpp"
#include <sstream>
#include <algorithm>
#include <string>
#include <cctype>

namespace fs = std::filesystem;

void HtmlParser::parse(const std::string& html)
{
    doc = htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR);
    if(!doc) 
    {
        std::cerr << "Error parsing HTML\n";
        return;
    }

    context = xmlXPathNewContext(doc);
    if(!context) 
    {
        std::cerr << "Error creating XPath context\n";
        xmlFreeDoc(doc);
        return;
    }

    html_elements = xmlXPathEvalExpression((xmlChar *) "//li[contains(@class, 'product')]", context);
    if(!html_elements || !html_elements->nodesetval) 
    {
        std::cerr << "No products found\n";
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        return;
    }

    std::cout << "Found " << html_elements->nodesetval->nodeNr << " products\n";

    for(int i = 0; i < html_elements->nodesetval->nodeNr; ++i)
    {
        xmlNodePtr product_node = html_elements->nodesetval->nodeTab[i];
        if(!product_node) continue;

        auto get_text_from_xpath = [&](const char* xpath_expr) -> std::string {
        context->node = product_node;

        xmlXPathObjectPtr obj =
        xmlXPathEvalExpression((xmlChar*)xpath_expr, context);

        std::string result;

        if (obj && obj->type == XPATH_NODESET &&
            obj->nodesetval && obj->nodesetval->nodeNr > 0)
        {
            xmlNodePtr node = obj->nodesetval->nodeTab[0];
            xmlChar* content = xmlNodeGetContent(node);
            if (content) {
            result = (char*)content;
            xmlFree(content);
            }
        }

        if (obj) xmlXPathFreeObject(obj);
        return result;
};

        std::string url   = get_text_from_xpath(".//a/@href");
        std::string image = get_text_from_xpath(".//img/@src");
        std::string name  = get_text_from_xpath(".//a/h2");
        std::string string_price = get_text_from_xpath(".//a/span");

        double price = clean_price(string_price);

        Product product = {url, name, image, price};
        products.push_back(product);
    }

    xmlXPathFreeObject(html_elements);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    export_to_csv("products.csv");
}


void HtmlParser::export_to_csv(const std::string& filename)
{
    std::string folder_name = "csv_files";

    if(!fs::exists(folder_name))
    {
        if(fs::create_directory(folder_name))
        {
            std::cout << "Folder created: " << folder_name << "\n";
        }else{
            std::cerr << "Failed to create folder: " << folder_name << "\n";
            return;
        }
    }

    std::string csv_path = folder_name + "/" + filename;

    std::ofstream csv_file(csv_path);

    if(!csv_file)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    csv_file << "url,name,image,price" << "\n";

    for(int i = 0; i < products.size(); ++i)
    {
        Product p = products.at(i);

        std::string csv_record = p.url + "," + p.name + "," + p.image + "," + std::to_string(p.price);

        csv_file << csv_record << "\n";
    }

    csv_file.close();
}

std::vector<Product> HtmlParser::read_csv(const std::string& filename)
{
    std::vector<Product> read_products;

    std::ifstream file(filename);

    if(! file.is_open()) return read_products;

    std::string line;
    std::getline(file,line);

    while(std::getline(file,line))
    {
        std::istringstream ss(line);
        std::string name,priceStr,url,image;

        std::getline(ss,name,',');
        std::getline(ss,priceStr,',');
        std::getline(ss,url,',');
        std::getline(ss,image,',');

        Product p;

        p.name = name;
        p.price = std::stod(priceStr);
        p.url = url;
        p.image = image;

        read_products.push_back(p);
    }

    return read_products;
}

void HtmlParser::filter_csv_by_price(const std::string& filename,double min,double max)
{   
    if(!fs::exists(filename))
    {
        std::cerr << "Could not open file: " << filename << '\n';
        return;
    }

    auto products = read_csv(filename);

    products.erase(
        std::remove_if(products.begin(), products.end(),
                       [&](const Product& p){ return p.price < min || p.price > max; }),
        products.end()
    );

    std::ofstream out (filename);

    out << "url,name,image,price\n";

    for(auto& p : products)
    {
        out << p.url << "," << p.name << "," << p.image << "," << p.price << '\n';
    }
}

double HtmlParser::clean_price(const std::string& str)
{
    std::string clean;

    for(char c : str)
    {
        if(std::isdigit(c) || c == '.')
            clean += c;
    }

    double result = std::stod(clean);

    return result;
}


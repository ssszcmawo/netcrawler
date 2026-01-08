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

        auto get_text_from_xpath = [&](const char* xpath_expr) -> std::string
        {
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
        std::string price = get_text_from_xpath(".//a/span"); 

        Product product = {url, name, image, price};
        products.push_back(product);
    }

    xmlXPathFreeObject(html_elements);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);

    export_to_csv("products.csv");
}

static std::string escape_csv(const std::string& field)
{
    std::string result = "\"";
    for (char c : field)
    {
        if (c == '"') result += "\"\"";
        else result += c;
    }
    result += "\"";
    return result;
}

void HtmlParser::export_to_csv(const std::string& filename)
{
    fs::path folder_name = fs::current_path().parent_path() / "csv_files";
    if(!fs::exists(folder_name))
    {
        if(fs::create_directory(folder_name))
        {
            std::cout << "Folder created: " << folder_name << "\n";
        } else {
            std::cerr << "Failed to create folder: " << folder_name << "\n";
            return;
        }
    }

    fs::path csv_path = folder_name / filename;
    std::ofstream csv_file(csv_path);

    if(!csv_file)
    {
        std::cerr << "Failed to open file: " << csv_path << "\n";
        return;
    }

    csv_file << "url,name,image,price\n";

    for (auto& p : products)
    {
        csv_file << escape_csv(p.url) << ","
                 << escape_csv(p.name) << ","
                 << escape_csv(p.image) << ","
                 << escape_csv(p.price) << "\n";
    }

    csv_file.close();
    std::cout << "CSV saved to: " << csv_path << "\n";
}

std::vector<Product> HtmlParser::read_csv(const std::string& filename)
{
    std::vector<Product> read_products;

    const fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    const fs::path csv_path = csv_files_dir / filename;

   if(!fs::exists(csv_path)) 
   {
        std::cerr << "CSV file not found: " << csv_path << '\n';
        return read_products;
   }

   std::ifstream file(csv_path);
   if(!file.is_open()) return read_products;

   std::string line;
   std::getline(file,line);

   while(std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string url,name,image,price;

        auto read_field = [&](std::string& dest)
        {
            if(line.empty()) return;
            if(line.front() == '"')
            {
                line.erase(0,1);
                auto pos = line.find("\"");
                if(pos != std::string::npos)
                {
                    dest = line.substr(0,pos);
                    line.erase(0,pos+2);
                }
            }
            else
            {
                auto pos = line.find(',');
                if(pos != std::string::npos)
                {
                    dest = line.substr(0,pos);
                    line.erase(0,pos+1);
                } else {
                    dest = line;
                    line.clear();
                }
            }
        };
        read_field(url);
        read_field(name);
        read_field(image);
        read_field(price);

        read_products.emplace_back(Product{url,name,image,price});
    }

    return read_products;
}

static double clean_price(const std::string& str)
{
    std::string clean;

    for(char c : str)
    {
        if(std::isdigit(c) || c == '.')
            clean += c;
    }

    if(clean.empty()) return 0.0;

    try
    {
        return std::stod(clean); 
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed to convert: " << str << ": " << e.what() << '\n';
        return 0.0;
    }
}

void HtmlParser::filter_csv_by_price(const std::string& filename,double min,double max)
{   
    if(filename.empty())
        return;

    auto products = read_csv(filename);

    products.erase(
        std::remove_if(products.begin(), products.end(),
                       [&](const Product& p){ 
                            double price = clean_price(p.price);
                            return price < min || price > max;
                        }),
        products.end()
    );

    std::sort(products.begin(),products.end(),
            [](const Product& a, const Product& b){
                return clean_price(a.price) < clean_price(b.price);
            });

    fs::path csv_files_dir = fs::current_path().parent_path() / "csv_files";
    fs::path csv_path = csv_files_dir / filename;
    std::ofstream out(csv_path);

    if(!out) { std::cerr << "Failed to open CSV for writing: " << csv_path << "\n"; return; }

    out << "url,name,image,price\n";
    for(auto& p : products)
    {
        out << escape_csv(p.url) << ","
            << escape_csv(p.name) << ","
            << escape_csv(p.image) << ","
            << escape_csv(p.price) << "\n";
    }

    out.close();

    std::cout << "Filtered CSV by price\n";
}


std::optional<Product> HtmlParser::find_product_by_name(const std::string& filename,const std::string& name)
{
    if(filename.empty())
    {
        std::cerr << "Filename is empty";
        return std::nullopt;
    }

    auto products = read_csv(filename);

    auto it = std::find_if(products.begin(),products.end(),[&](const Product& p){
        return p.name == name;
    });

    if(it != products.end())
        return *it;

    return std::nullopt;
}


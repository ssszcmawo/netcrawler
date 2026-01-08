#include <iostream>
#include <vector>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <fstream>
#include <filesystem>
#include <optional>

struct Product
{
    std::string url;
    std::string name;
    std::string image;
    std::string price;

};

class HtmlParser
{
public:
    HtmlParser(const std::string& html)
    {
        parse(html);
    }

    ~HtmlParser(){};

    const std::vector<Product>& get_products() const { return products; }

    std::optional<Product> find_product_by_name(const std::string& filename,const std::string& name);
    void filter_csv_by_price(const std::string& filename,double min,double max);

private:
    void parse(const std::string& html);
    void export_to_csv(const std::string& filename);
    std::vector<Product> read_csv(const std::string& filename); 

    std::vector<Product> products;
    htmlDocPtr doc;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr html_elements;
};
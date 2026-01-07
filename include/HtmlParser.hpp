#include <iostream>
#include <vector>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <fstream>

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

    ~HtmlParser()
    {
        
    }

    const std::vector<Product>& get_products() const { return products; }

    Product* find_product_by_name(const std::string& name);
    std::vector<Product> filter_by_price(double min,double max);

private:
    void parse(const std::string& html);
    void export_to_csv(const std::string& filename);

    std::vector<Product> products;
    htmlDocPtr doc;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr html_elements;
};
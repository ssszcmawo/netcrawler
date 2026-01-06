#include <iostream>
#include <vector>

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
    HtmlParser(const std::string& html){}

    const std::vector<Product>& get_products() const { return products; }

    Product* find_product_by_name(const std::string& name);
    std::vector<Product> filter_by_price(double min,double max);

private:
    void parse(const std::string& html);

    std::vector<Product> products;
};
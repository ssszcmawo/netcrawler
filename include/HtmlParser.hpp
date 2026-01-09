#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <optional>
#include <string_view>
#include <vector>

struct Product
{
    std::string url;
    std::string name;
    std::string image;
    std::string price;
};

namespace Utils
{
extern std::vector<Product> products;

std::string escape_csv(const std::string &field);
std::optional<double> parse_price(const std::string_view str);
std::vector<Product> read_csv(const std::string &filename);
void export_to_csv(const std::string &filename);
std::optional<Product> find_product_by_name(const std::string &filename,
    const std::string &name);
void filter_csv_by_price(const std::string &filename, double min, double max);
const std::vector<Product> &get_products();
} // namespace Utils

class HtmlParser
{
  public:
    HtmlParser(const std::string &html)
    {
        parse(html);
    }
    ~HtmlParser() {};

  private:
    void parse(const std::string &html);

    struct XmlDocWrapper
    {
        xmlDocPtr doc;
        XmlDocWrapper(xmlDocPtr d) :
            doc(d)
        {
        }
        ~XmlDocWrapper()
        {
            if (doc)
                xmlFreeDoc(doc);
        }
        operator xmlDocPtr() const
        {
            return doc;
        }
        xmlDocPtr operator->() const
        {
            return doc;
        }
    };

    struct XmlXPathContextWrapper
    {
        xmlXPathContextPtr ctx;
        XmlXPathContextWrapper(xmlXPathContextPtr c) :
            ctx(c)
        {
        }
        ~XmlXPathContextWrapper()
        {
            if (ctx)
                xmlXPathFreeContext(ctx);
        }
        operator xmlXPathContextPtr() const
        {
            return ctx;
        }
        xmlXPathContextPtr operator->() const
        {
            return ctx;
        }
    };

    struct XmlXPathObjectWrapper
    {
        xmlXPathObjectPtr obj;
        XmlXPathObjectWrapper(xmlXPathObjectPtr o) :
            obj(o)
        {
        }
        ~XmlXPathObjectWrapper()
        {
            if (obj)
                xmlXPathFreeObject(obj);
        }
        operator xmlXPathObjectPtr() const
        {
            return obj;
        }
        xmlXPathObjectPtr operator->() const
        {
            return obj;
        }
    };
};
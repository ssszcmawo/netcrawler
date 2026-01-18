#include "HtmlParser.hpp"
#include "slogger.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

std::vector<Product> HtmlParser::parse(const std::string &html, ProductXPathConfig &config)
{
    std::vector<Product> products;
    LOG_INFO("[HtmlParser::parse] Parsing started, HTML size: %zu", html.size());

    XmlDocWrapper doc(htmlReadMemory(html.c_str(), html.size(), nullptr, nullptr, HTML_PARSE_NOERROR));
    if (!doc)
    {
        LOG_ERROR("[HtmlParser::parse] Failed to parse HTML");
        return products;
    }

    XmlXPathContextWrapper context(xmlXPathNewContext(doc));
    if (!context)
    {
        LOG_ERROR("[HtmlParser::parse] Failed to create XPath context");
        return products;
    }

    XmlXPathObjectWrapper html_elements(xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(config.product_item.c_str()), context));

    if (!html_elements || !html_elements->nodesetval)
    {
        LOG_ERROR("[HtmlParser::parse] No product elements found in HTML");
        return products;
    }

    LOG_INFO("[HtmlParser::parse] Found %d product elements", html_elements->nodesetval->nodeNr);

    for (int i = 0; i < html_elements->nodesetval->nodeNr; ++i)
    {
        xmlNodePtr product_node = html_elements->nodesetval->nodeTab[i];
        if (!product_node)
            continue;

        auto get_text_from_xpath = [&](xmlNodePtr base_node, const std::string &xpath_expr) -> std::string
        {
            context->node = base_node;
            XmlXPathObjectWrapper obj(xmlXPathEvalExpression(
                reinterpret_cast<const xmlChar *>(xpath_expr.c_str()), context));

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

        auto get_attr_from_xpath = [&](xmlNodePtr base_node, const std::string &xpath_expr, const char *attr) -> std::string
        {
            context->node = base_node;
            XmlXPathObjectWrapper obj(xmlXPathEvalExpression(
                reinterpret_cast<const xmlChar *>(xpath_expr.c_str()), context));

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

        std::string url = get_attr_from_xpath(product_node, config.url, "href");
        std::string image = get_attr_from_xpath(product_node, config.image, "src");
        std::string name = get_text_from_xpath(product_node, config.name);
        std::string price = get_text_from_xpath(product_node, config.price);

        LOG_DEBUG("[HtmlParser::parse] Product parsed: %s | %s | %s | %s",
            url.c_str(),
            name.c_str(),
            image.c_str(),
            price.c_str());

        products.push_back(std::move(Product{url, name, image, price}));
    }

    LOG_INFO("[HtmlParser::parse] Parsing finished, total products: %zu", products.size());
    return products;
}

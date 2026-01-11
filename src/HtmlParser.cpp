#include "HtmlParser.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

extern "C"
{
#include "slogger.h"
}

void HtmlParser::parse(const std::string &html)
{
    LOG_INFO("[HtmlParser::parse] Parsing started, HTML size: %zu", html.size());

    auto &repo = ProductRepository::instance();
    repo.clear();

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

        repo.add(Product{url, name, image, price});
    }

    Utils::export_to_csv("products.csv", repo.get_all());

    LOG_INFO("[HtmlParser::parse] Parsing finished, total products: %zu", repo.get_all().size());
}

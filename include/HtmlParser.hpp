#pragma once
#include "ProductRepository.hpp"
#include "ProductXPathConfig.hpp"
#include "Utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <optional>
#include <string_view>
#include <vector>

class HtmlParser
{
  public:
    HtmlParser() = default;
    ~HtmlParser() = default;
    void parse(const std::string &html, ProductXPathConfig &config);

  private:
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
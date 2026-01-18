#include "Crawler.hpp"
#include "ProductRepository.hpp"
#include "slogger.h"

Crawler::Crawler(ProductXPathConfig &config) :
    config_(config)
{
    pages_visited.insert(config.first_page);
    pages_to_visit.push(config.first_page);
}

Crawler::~Crawler() = default;

void Crawler::run()
{
    auto &repo = ProductRepository::instance();
    repo.clear();

    while (!pages_to_visit.empty())
    {
        const std::string url = pages_to_visit.front();
        pages_to_visit.pop();

        crawl_page(url);
    }
}

void Crawler::crawl_page(const std::string &url)
{
    LOG_INFO("[Crawler::crawl_page] Fetching %s", url.c_str());

    const std::string html = client_.get_request(url);
    if (html.empty())
    {
        LOG_ERROR("[Crawler::crawl_page] Empty HTML: %s", url.c_str());
        return;
    }

    auto products = parser_.parse(html, config_);
    ProductRepository::instance().add_range(std::move(products));
    discover_pages(html);
}

static std::string make_absolute_url(const std::string &base, const std::string &link)
{
    if (link.rfind("http", 0) == 0)
        return link;

    if (link.front() == '/')
    {
        auto pos = base.find("://");
        if (pos == std::string::npos)
            return link;

        pos = base.find('/', pos + 3);
        if (pos == std::string::npos)
            return base + link;

        return base.substr(0, pos) + link;
    }

    if (base.back() == '/')
        return base + link;

    return base + "/" + link;
}

static std::string normalize_url(std::string url)
{
    if (!url.empty() && url.back() == '/')
        url.pop_back();
    return url;
}

void Crawler::discover_pages(const std::string &html)
{
    XmlDocWrapper doc(htmlReadMemory(
        html.c_str(),
        html.size(),
        nullptr,
        nullptr,
        HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING));

    if (!doc)
        return;

    XmlXPathContextWrapper ctx(xmlXPathNewContext(doc));
    if (!ctx)
        return;

    XmlXPathObjectWrapper obj(xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(config_.page_numbers.c_str()), ctx));

    if (!obj || !obj->nodesetval || obj->nodesetval->nodeNr == 0)
        return;

    for (int i = 0; i < obj->nodesetval->nodeNr; ++i)
    {
        xmlNodePtr node = obj->nodesetval->nodeTab[i];
        if (!node)
            continue;

        xmlChar *href = xmlGetProp(node, BAD_CAST "href");
        if (!href)
            continue;

        std::string next = reinterpret_cast<char *>(href);
        xmlFree(href);

        if (next.empty())
            continue;

        next = make_absolute_url(config_.first_page, next);
        next = normalize_url(next);

        if (next.find("/page/") == std::string::npos &&
            next.find("page=") == std::string::npos &&
            next.find("page-") == std::string::npos)
        {
            continue;
        }

        if (pages_visited.insert(next).second)
        {
            pages_to_visit.push(next);
        }
    }
}

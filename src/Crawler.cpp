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

    parser_.parse(html, config_);
    discover_pages(html);
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

        if (next.find("/page/") == std::string::npos)
            continue;

        next = normalize_url(next);

        LOG_INFO("[Crawler::discover_pages] Found page link: %s", next.c_str());

        if (pages_visited.insert(next).second)
        {
            pages_to_visit.push(next);
            LOG_INFO("[Crawler::discover_pages] New page added: %s", next.c_str());
        }
        else
        {
            LOG_INFO("[Crawler::discover_pages] Page already visited: %s", next.c_str());
        }
    }

    LOG_INFO("[Crawler::discover_pages] Queue size: %zu", pages_to_visit.size());
}

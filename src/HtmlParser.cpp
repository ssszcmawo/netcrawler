#include "HtmlParser.hpp"

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

        auto get_text_from_xpath = [&](const char* xpath_expr) -> std::string {
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


void HtmlParser::export_to_csv(const std::string& filename)
{
   std::ofstream csv_file(filename);

   csv_file << "url,name,image,price" << "\n";

   for(int i = 0; i < products.size(); ++i)
   {
      Product p = products.at(i);

      std::string csv_record = p.url + "," + p.name + "," + p.image + "," + p.price;

      csv_file << csv_record << "\n";
   }

   csv_file.close();
}


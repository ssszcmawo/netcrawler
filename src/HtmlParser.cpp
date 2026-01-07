#include "HtmlParser.hpp"

HtmlParser::~HtmlParser()
{
   if(html_elements) {
        xmlXPathFreeObject(html_elements);
        html_elements = nullptr;
    }

    if(context) {
        xmlXPathFreeContext(context);
        context = nullptr;
    }

    if(doc) {
        xmlFreeDoc(doc);
        doc = nullptr;
    }
}

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
            xmlXPathObjectPtr obj = xmlXPathNodeEval(product_node, (xmlChar*)xpath_expr, context);
            std::string result;
            if(obj && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
                xmlNodePtr node = obj->nodesetval->nodeTab[0];
                if(node) {
                    xmlChar* content = nullptr;
                    if(node->type == XML_ELEMENT_NODE) {
                        content = xmlNodeGetContent(node);
                    } else {
                        content = xmlGetProp(node, (xmlChar*)"href");
                    }
                    if(content) {
                        result = reinterpret_cast<char*>(content);
                        xmlFree(content);
                    }
                }
            }
            if(obj) xmlXPathFreeObject(obj);
            return result;
        };

        std::string url   = get_text_from_xpath(".//a/@href");
        std::string image = get_text_from_xpath(".//a/img/@src");
        std::string name  = get_text_from_xpath(".//a/h2");
        std::string price = get_text_from_xpath(".//a/span");

        Product product = {url, image, name, price};
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

   csv_file << "url,image,name,price" << "\n";

   for(int i = 0; i < products.size(); ++i)
   {
      Product p = products.at(i);

      std::string csv_record = p.url + "," + p.image + "," + p.name + "," + p.price;

      csv_file << csv_record << "\n";
   }

   csv_file.close();
}

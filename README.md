# NetCrawler
A simple educational web crawler written in C++ that demonstrates how to fetch web pages, parse HTML content, extract links, and traverse websites. The project is designed for learning purposes and focuses on core networking, string processing, data structures, using Git, and getting hands-on experience calling C libraries from C++ code, rather than large-scale crawling.

## What it does

- downloads HTML pages via HTTPS using **libcurl**
- parses HTML using **libxml2 + XPath**
- finds product blocks and extracts:
  - name
  - price
  - image URL
  - product URL
- follows pagination links
- stores results in `csv_files/products.csv`

## Features

- config driven (XPath rules are in JSON)
- basic HTTP error handling + retries
- simple logging system implemented with my own library (slogger)

## Requirements

You need these libs installed:

- `libcurl`
- `libxml2`
- `nlohmann/json`
- `fast-cpp-csv-parser`

## How to use
1.Build the project (CMake)
  The project has two types of building

  You can build in Debug or Release mode:

  - Debug: console logs, no optimization (good for development)
  ```c
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  cmake --build .
  ```
  - Release: file logs, optimized (better performance)
  ```c
  cmake -DCMAKE_BUILD_TYPE=Release ..
  cmake --build .
  ```
2.Run it with a JSON config:
  ```c
  ./bin/WebCrawler path/to/config.json
  ```
Example config:
```c
{
  "first_page": "https://example.com/shop",
  "product_item": "//div[@class='product']",
  "url": ".//a[@class='product-link']",
  "image": ".//img[@class='product-image']",
  "name": ".//h2[@class='product-title']",
  "price": ".//span[@class='price']",
  "page_numbers": "//a[@class='page-link']"
}
```

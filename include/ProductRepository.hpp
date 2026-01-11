#pragma once
#include <string>
#include <vector>
struct Product
{
    std::string url;
    std::string name;
    std::string image;
    std::string price;
};

class ProductRepository
{
  public:
    static ProductRepository &instance()
    {
        static ProductRepository repo;
        return repo;
    }

    void clear()
    {
        products_.clear();
    }
    void add(Product p)
    {
        products_.push_back(std::move(p));
    }
    void add_range(std::vector<Product> v)
    {
        products_.insert(products_.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
    }

    const std::vector<Product> &get_all() const &
    {
        return products_;
    }
    std::vector<Product> get_all() &&
    {
        return std::move(products_);
    }

  private:
    ProductRepository() = default;
    std::vector<Product> products_;
};
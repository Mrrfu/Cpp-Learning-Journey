//
// Created by a2057 on 25-7-28.
//

#include "unique_ptr.h"
#include <iostream>


int main() {
    unique_ptr<int> p(new int(42));
    std::cout << *p << std::endl;
    std::cout << p.get() << std::endl;
    unique_ptr<int> q(std::move(p));
    unique_ptr<int> r = std::move(q);
    std::cout << *r << std::endl;
    unique_ptr<std::string> s(new std::string("hello"));
    std::cout << *s << std::endl;
    std::cout << s->size() << std::endl;
}
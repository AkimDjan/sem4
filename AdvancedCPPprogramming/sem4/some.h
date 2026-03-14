#pragma once
#include <bits/stdc++.h>

class SomeImpl;
class Some{
public:
    int Do(size_t i) const;
    int& Do(size_t i);
    void Inc();
    ~Some();//объявили 
private:
    std::unique_ptr<SomeImpl> pimpl_=nullptr;
};
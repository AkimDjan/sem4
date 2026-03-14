#include "some.h"

class SomeImpl{
std::vector<int> v_={3,2,1};
public:
    int Do(size_t i) const {
        i = i % 3;
        return v_[i];
    }
private:
    void Inc(){
        for (int i=0; i<v_.size(); i++){
            v_[i]++;
        }
    }
};


int Some::Do(size_t i) const {
    return pimpl_->Do(i);
}

Some::~Some()=default;


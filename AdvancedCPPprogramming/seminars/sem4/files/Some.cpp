#include "Some.h"

#include <vector>

class SomeImpl {
public:
	int& Do(size_t i) {
		i = i % v_.size();
		return v_[i];
	}

	int Do(size_t i) const {
		i = i % v_.size();
		return v_[i];
	}
private:
	std::vector<int> v_ = { 3, 2, 1 };
};

Some::Some() {
	pimpl_ = std::make_unique<SomeImpl>();
}

int Some::Do(size_t i) const {
	return pimpl_->Do(i);
}

int& Some::Do(size_t i) {
	return pimpl_->Do(i);
}

Some::Some(const Some& rhs) {
	pimpl_ = std::make_unique<SomeImpl>(*rhs.pimpl_);
}

Some::~Some() = default;
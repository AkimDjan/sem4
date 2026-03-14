#pragma once
#include <memory>

class SomeImpl;

class Some {
	std::unique_ptr<SomeImpl> pimpl_;
public:
	int Do(size_t i) const;
	int& Do(size_t i);

	Some();
	Some(Some&& rhs) = default;
	Some(const Some& rhs);
	~Some();
};
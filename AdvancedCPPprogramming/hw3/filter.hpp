#ifndef POINTCLOUD_PREPROCESSOR__FILTER_HPP_
#define POINTCLOUD_PREPROCESSOR__FILTER_HPP_

#include "Logger.h"
#include "PointCloud.h"

#include <string>
#include <cmath>
#include <unordered_map>

namespace pointcloud_preprocessor
{

class FilterParametr {
public:
    FilterParametr() {}
    FilterParametr(std::unordered_map<std::string, double> params) : params_(std::move(params)) {}
    
    // Add new param or change exsistent
    void ChangeParam(const std::string& param_name, double param_val) {
        params_[param_name] = param_val;
    }

    double GetParam(const std::string& param_name) const {
        return params_.at(param_name);
    }

    double GetParam(const std::string& param_name, double default_val) const noexcept {
        auto it = params_.find(param_name);
        if (it == params_.end())
            return default_val;

        return it->second;
    }
private:
    std::unordered_map<std::string, double> params_;
};

/** \brief @b Filter represents the base filter class. 
 */
class Filter
{
public:
  Filter(const std::string & filter_name = "pointcloud_preprocessor_filter") : filter_name_(filter_name), logger_(filter_name_) {}
  
  const std::string& GetFilterName() { return filter_name_; }
  
  double GetDistance(PointCloud* pc, size_t index) {
      if (pc->pointcloud_type_ == "XYZIR") {
          double x = pc->points_[index * pc->point_size_ + 0];
          double y = pc->points_[index * pc->point_size_ + 1];
          double z = pc->points_[index * pc->point_size_ + 2];
          return std::hypot(x, y, z);
      }

      return pc->points_[index * pc->point_size_ + 5];
  }
  
  double GetAzimuth(PointCloud* pc, size_t index) {
      if (pc->pointcloud_type_ == "XYZIR") {
          double x = pc->points_[index * pc->point_size_ + 0];
          double y = pc->points_[index * pc->point_size_ + 1];
          return std::atan2(y, x);
      }

      return pc->points_[index * pc->point_size_ + 6];
  }

  virtual PointCloud* Apply(PointCloud* pc) = 0;
  
  virtual void SetParams(const FilterParametr& param) {
      params_ = std::move(param);
  }

  const FilterParametr& GetFilterParam() const {
      return params_;
  }
protected:
    std::string filter_name_ = "pointcloud_preprocessor_filter";
    Logger logger_;
    FilterParametr params_;
};

}  // namespace pointcloud_preprocessor

#endif  // POINTCLOUD_PREPROCESSOR__FILTER_HPP_

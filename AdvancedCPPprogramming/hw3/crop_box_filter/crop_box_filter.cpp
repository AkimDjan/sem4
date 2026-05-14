#include "crop_box_filter.hpp"

#include <vector>

namespace pointcloud_preprocessor
{
CropBoxFilter::CropBoxFilter()
: Filter("CropBoxFilter")
{
}

PointCloud* CropBoxFilter::Apply(PointCloud* pc)
{
  size_t output_size = 0, output_points_count = 0;
  std::vector<double> output(pc->size_ * pc->point_size_);

  for (size_t global_offset = 0; global_offset + pc->point_size_ <= pc->size_ * pc->point_size_;
       global_offset += pc->point_size_) {
    std::vector<double> point(3);
    std::memcpy(&point[0], &pc->points_[global_offset + 0], sizeof(double));
    std::memcpy(&point[1], &pc->points_[global_offset + 1], sizeof(double));
    std::memcpy(&point[2], &pc->points_[global_offset + 2], sizeof(double));

    if (!std::isfinite(point[0]) || !std::isfinite(point[1]) || !std::isfinite(point[2])) {
      logger_.log("Ignoring point containing NaN values");
      continue;
    }

    bool point_is_inside = point[2] > param_.min_z && point[2] < param_.max_z &&
                           point[1] > param_.min_y && point[1] < param_.max_y &&
                           point[0] > param_.min_x && point[0] < param_.max_x;
    if ((!param_.negative && point_is_inside) || (param_.negative && !point_is_inside)) {
      memcpy(&output[output_size], &pc->points_[global_offset], pc->point_size_ * sizeof(double));
      output_size += pc->point_size_;
      output_points_count += 1;
    }
  }

  output.resize(output_size);
  PointCloud* output_pc = new PointCloud;
  output_pc->points_ = std::move(output);
  output_pc->pointcloud_type_ = pc->pointcloud_type_;
  output_pc->size_ = output_points_count;
  output_pc->point_size_ = pc->point_size_;
  return output_pc;
}

void CropBoxFilter::SetParams(const FilterParametr& param)
{

  CropBoxParam new_param{};

  new_param.min_x = param.GetParam("min_x", new_param.min_x);
  new_param.max_x = param.GetParam("max_x", new_param.max_x);
  new_param.min_y = param.GetParam("min_y", new_param.min_y);
  new_param.max_y = param.GetParam("max_y", new_param.max_y);
  new_param.min_z = param.GetParam("min_z", new_param.min_z);
  new_param.max_z = param.GetParam("max_z", new_param.max_z);
  new_param.negative = static_cast<bool>(param.GetParam("max_z", param_.negative ? 1 : 0));
  if (
    new_param.min_x != 0 && new_param.max_x != 0 &&
    new_param.min_y != 0 && new_param.max_y != 0 &&
    new_param.min_z != 0 && new_param.max_z != 0) {
    if (
      param_.min_x != new_param.min_x || param_.max_x != new_param.max_x ||
      param_.min_y != new_param.min_y || param_.max_y != new_param.max_y ||
      param_.min_z != new_param.min_z || param_.max_z != new_param.max_z ||
      param_.negative != new_param.negative) {
      logger_.log("[paramCallback] Setting the minimum point to: " +
        std::to_string(new_param.min_x) + " " + std::to_string(new_param.min_y) + " " + std::to_string(new_param.min_z));
      logger_.log("[paramCallback] Setting the maximum point to: " +
        std::to_string(new_param.max_x) + " " + std::to_string(new_param.max_y) + " " + std::to_string(new_param.max_z));
      logger_.log("[paramCallback] Setting the filter negative flag to: " + new_param.negative ? "true" : "false");
      param_ = new_param;
    }
  }
  Filter::SetParams(param);
}

}  // namespace pointcloud_preprocessor

#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include <string>
#include <stdexcept>
#include <vector>

// The lidar's pointcloud contains coordinates of point (X, Y and Z) and Intencity of ret beam for point (I) and beam ring (R)
// Also it can contains Distance (D), Azimuth (A) and Timestamp (T)

class PointCloud {
public:
	void Init(std::string pc_type, size_t points_count) {
		pointcloud_type_ = std::move(pc_type);
		size_ = points_count;
		if (pointcloud_type_ == "XYZIR")
			points_ = std::vector<double>(size_ * 5);
		else if (pointcloud_type_ == "XYZIRDAT")
			points_ = std::vector<double>(size_ * 8);
		else
			throw std::runtime_error("unknown pointcloud type");
	}

	void AddPoint(std::vector<double> point, size_t ind) {
		if ( (pointcloud_type_ == "XYZIR"  && point.size() != 5) ||
			(pointcloud_type_ == "XYZIRDAT" && point.size() != 8))
			throw std::runtime_error("Point has unsuported size = " + std::to_string(point.size()) + " for type " + pointcloud_type_);

		if (pointcloud_type_ == "XYZIR")
			point_size_ = 5;
		else if (pointcloud_type_ == "XYZIRDAT")
			point_size_ = 8;
		
		for (size_t i = 0; i < point.size(); ++i) {
			points_[point_size_ * ind + i] = point[i];
		}
	}

	std::string pointcloud_type_{""};
	size_t size_ = 0;
	size_t point_size_ = 0;
	std::vector<double> points_;
};

// For tests
inline void FillPointCloud(PointCloud* pc, size_t points_count, std::string point_type, const std::vector<double>& data) {
	size_t point_size = (point_type == "XYZIR") ? 5 : 8;
	pc->Init(point_type, points_count);
	size_t ind = 0;
	for (size_t i = 0; i < data.size(); i += point_size) {
		pc->AddPoint(std::vector<double>{data.begin() + point_size * ind, data.begin() + point_size * (ind + 1)}, ind);
		ind++;
	}
}

#endif //POINTCLOUD_H_
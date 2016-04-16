#pragma once

// A simple knn with cv::flann.
// Its document is too obsecured so I have to note this down.

#include <array>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/flann.hpp>

template <typename T /* = float */, size_t Dimension = 3>
class KnnIndex {
public:
    typedef T value_type;
    typedef std::array<value_type, Dimension> point_type;
    static const size_t dimension = Dimension;

    KnnIndex(const std::vector<point_type> &points, bool bruteforce = false) {
        npoints = points.size();
        cv::Mat data((int)npoints, dimension, cv::DataType<value_type>::type, (void*)points[0].data());
        if (bruteforce) {
            flannIndex = std::make_unique<cv::flann::GenericIndex<cvflann::L2<value_type>>>(data, cvflann::LinearIndexParams());
        }
        else {
            flannIndex = std::make_unique<cv::flann::GenericIndex<cvflann::L2<value_type>>>(data, cvflann::KDTreeIndexParams());
        }
    }

    std::vector<int> knn(const point_type &point, int k) {
        std::vector<value_type> distances(k);
        return knn(point, k, distances);
    }

    std::vector<int> knn(const point_type &point, int k, std::vector<value_type> &distances) {
        std::vector<int> indices(k);
        std::vector<value_type> vpoint(point.begin(), point.end());
        distances.resize(k);
        flannIndex->knnSearch(vpoint, indices, distances, k, cvflann::SearchParams());
        return indices;
    }

    std::vector<int> knn(const std::vector<point_type> &points, int k) {
        std::vector<value_type> distances(points.size()*k);
        return knn(points, k, distances);
    }

    std::vector<int> knn(const std::vector<point_type> &points, int k, std::vector<value_type> &distances) {
        std::vector<int> indices(points.size()*k);
        cv::Mat points_data((int)points.size(), dimension, cv::DataType<value_type>::type, (void*)points[0].data());
        cv::Mat indices_data((int)points.size(), k, cv::DataType<int>::type, (void*)indices.data());
        cv::Mat distances_data((int)points.size(), k, cv::DataType<value_type>::type, (void*)distances.data());
        flannIndex->knnSearch(points_data, indices_data, distances_data, k, cvflann::SearchParams());
        return indices;
    }

    std::vector<int> rnn(const point_type& point, value_type radius, size_t max_count = 0) {
        std::vector<value_type> distances(max_count > 0 ? max_count : npoints);
        return rnn(point, radius, distances, max_count);
    }

    std::vector<int> rnn(const point_type& point, value_type radius, std::vector<value_type> &distances, size_t max_count = 0) {
        std::vector<int> indices(max_count > 0 ? max_count : npoints);
        std::vector<value_type> vpoint(point.begin(), point.end());
        int count = flannIndex->radiusSearch(vpoint, indices, distances, radius, cvflann::SearchParams());
        indices.resize(count);
        distances.resize(count);
        return indices;
    }

private:
    size_t npoints;
    std::unique_ptr<cv::flann::GenericIndex<cvflann::L2<value_type>>> flannIndex;
};

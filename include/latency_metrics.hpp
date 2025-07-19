#pragma once
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <deque>

class LatencyMetrics {
    std::deque<double> samples;
    size_t max_samples;
    std::mutex mtx;
public:
    LatencyMetrics(size_t max_samples_ = 1000) : max_samples(max_samples_) {}
    void add(double v) {
        std::lock_guard<std::mutex> lock(mtx);
        if (samples.size() >= max_samples) samples.pop_front();
        samples.push_back(v);
    }
    std::vector<double> get_samples() {
        std::lock_guard<std::mutex> lock(mtx);
        return std::vector<double>(samples.begin(), samples.end());
    }
    double avg() {
        std::lock_guard<std::mutex> lock(mtx);
        if (samples.empty()) return 0.0;
        return std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    }
    double min() {
        std::lock_guard<std::mutex> lock(mtx);
        if (samples.empty()) return 0.0;
        return *std::min_element(samples.begin(), samples.end());
    }
    double max() {
        std::lock_guard<std::mutex> lock(mtx);
        if (samples.empty()) return 0.0;
        return *std::max_element(samples.begin(), samples.end());
    }
};

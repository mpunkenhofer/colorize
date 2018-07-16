//
// Created by necator on 7/10/18.
//

#ifndef COLORS2_UTILITIES_H
#define COLORS2_UTILITIES_H

#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <sstream>
#include <opencv2/core/core.hpp>

std::ostream& print_progress(int value, int end_value, std::ostream& os = std::cout);
std::ostream& print_delta_time_graph(std::ostream& os = std::cout);

struct cvPointiHash {
    std::size_t operator () ( const cv::Point2i& p ) const {
        return static_cast<std::size_t>(p.x) ^ static_cast<std::size_t>(p.y);
    }
};

#endif //COLORS2_UTILITIES_H

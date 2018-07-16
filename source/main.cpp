#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <list>
#include <ctime>
#include <array>
#include <thread>
#include <random>
#include <future>
#include <unordered_set>
#include <experimental/filesystem>
#include <fstream>

#include "utils.h"

std::vector<cv::Vec3b> generate_colors(std::size_t num_colors) {
    std::vector<cv::Vec3b> colors;

    for (std::size_t r = 0; r < num_colors; r++) {
        for (std::size_t g = 0; g < num_colors; g++) {
            for (std::size_t b = 0; b < num_colors; b++) {
                colors.emplace_back(cv::Vec3b(
                        static_cast<uint8_t>(255 * r / (num_colors - 1)),
                        static_cast<uint8_t>(255 * g / (num_colors - 1)),
                        static_cast<uint8_t>(255 * b / (num_colors - 1))
                ));
            }
        }
    }

    return colors;
}

double color_diff(const cv::Vec3b &c1, const cv::Vec3b &c2) {
    return std::pow(c2[0] - c1[0], 2.0) + std::pow(c2[1] - c1[1], 2.0) + std::pow(c2[2] - c1[2], 2.0);
}


bool uncolored_neighbors(const cv::Vec3b &c) {
    return c == cv::Vec3b(0, 0, 0);
}

bool colored_neighbors(const cv::Vec3b &c) {
    return c != cv::Vec3b(0, 0, 0);
}


std::list<cv::Point2i> neighborhood(const cv::Mat &image, const cv::Point2i &p,
                                    bool cmp(const cv::Vec3b &c) = [](const auto &) { return true; }) {
    std::list<cv::Point2i> neighbors;

    for (auto x = std::max(0, p.x - 1); x < std::min(image.cols, p.x + 2); x++) {
        for (auto y = std::max(0, p.y - 1); y < std::min(image.rows, p.y + 2); y++) {
            if (!(x == p.x && y == p.y) && cmp(image.at<cv::Vec3b>(y, x)))
                neighbors.emplace_back(cv::Point2i(x, y));
        }
    }

    return neighbors;
}

double differences_min(const std::vector<double> &differences) {
    auto min_el = std::min_element(differences.begin(), differences.end());
    return min_el != differences.end() ? *min_el : std::numeric_limits<double>::max();
}

double differences_average(const std::vector<double> &differences) {
    auto sum = std::accumulate(differences.begin(), differences.end(), 0.0);
    return sum / std::max(static_cast<std::size_t>(1), differences.size());
}

template<typename Iterator>
Iterator best_square(const cv::Mat &image, const Iterator begin, const Iterator end, const cv::Vec3b &color,
                     double differences_eval(const std::vector<double> &differences) = differences_min) {
    const auto length = std::distance(begin, end);

    if (!length)
        return end;

    Iterator min_it = end;
    double min_diff = std::numeric_limits<double>::max();

    for (auto it = begin; it != end; it++) {
        auto sq_neighbors = neighborhood(image, *it, colored_neighbors);
        std::vector<double> differences;

        std::transform(sq_neighbors.begin(), sq_neighbors.end(), std::back_inserter(differences),
                       [&color, &image](const auto &sq) { return color_diff(image.at<cv::Vec3b>(sq), color); });

        if (auto diff_eval_result = differences_eval(differences); diff_eval_result < min_diff) {
            min_diff = diff_eval_result;
            min_it = it;
        }
    }

    return min_it;
}

std::vector<cv::Mat> colorize(cv::Mat &image, const std::vector<cv::Vec3b>& colors, const cv::Point2i& start_point,
        const std::size_t snapshot_count = 0) {
    std::vector<cv::Mat> snapshots;

    const auto pixel_cnt = image.cols * image.rows;
    const auto snapshot_interval = static_cast<double>(pixel_cnt) / snapshot_count;
    const auto progress_update_interval = static_cast<double>(pixel_cnt) / 100;

    image.at<cv::Vec3b>(start_point) = colors[0];

    auto center_neighbors = neighborhood(image, start_point);
    auto available_squares = std::unordered_set<cv::Point2i, cvPointiHash>(center_neighbors.begin(),
                                                                           center_neighbors.end());
    int last_snapshot, last_progress_update;

    last_progress_update = last_snapshot = 0;

    for (auto i = 1; i < static_cast<int>(colors.size()) && !available_squares.empty(); i++) {

        if (i - last_snapshot > snapshot_interval) {
            snapshots.emplace_back(image.clone());
            last_snapshot = i;
        }

        if (i - last_progress_update > progress_update_interval) {
            print_progress(i, pixel_cnt);
            last_progress_update = i;
        }

        const auto &color = colors[i];

        if (auto best_sq = best_square(image, available_squares.begin(), available_squares.end(), color);
                best_sq != available_squares.end()) {
            image.at<cv::Vec3b>(*best_sq) = color;

            auto best_square_neighbors = neighborhood(image, *best_sq, uncolored_neighbors);
            available_squares.insert(best_square_neighbors.begin(), best_square_neighbors.end());
            available_squares.erase(best_sq);
        }
    }

   print_progress(pixel_cnt, pixel_cnt);

    if(snapshot_count > 0)
        snapshots.emplace_back(image.clone());

    return snapshots;
}

std::string timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm *ptm = std::localtime(&now);
    std::array<char, 32> buffer{};

    auto bytes_written = std::strftime(buffer.begin(), buffer.size(), "%Y-%m-%d_%H:%M:%S", ptm);

    return std::string(buffer.begin(), buffer.begin() + bytes_written);
}

void write_info_file(const std::string& output_filename, const std::string& timestamp, const cv::Size& render_size,
                     double duration, std::size_t snapshots) {
    std::ofstream info_file(output_filename);

    if(info_file) {
        info_file << "Time: " << timestamp << " - "
                  << "Resolution: " << render_size.width << "x" << render_size.height << " - "
                  << "Colorize duration: " << duration << "s - "
                  << "Snapshots: " << snapshots;

        info_file.close();
    }
}

int main() {
    const auto render_size = cv::Size(1920, 1080);
    const auto snapshot_count = 660;
    const auto output_filename{"result"};

    std::cout << "Creating image of size: " << render_size.width << "x" << render_size.height << std::endl;

    cv::Mat out = cv::Mat::zeros(render_size, CV_8UC3);

    const auto pixel_cnt = render_size.height * render_size.width;
    const auto required_colors = static_cast<std::size_t>(std::ceil(std::pow(pixel_cnt, 1.f / 3.f)));

    std::cout << "Generating colors..." << std::endl;
    auto colors = generate_colors(required_colors);

    std::cout << "Randomizing colors..." << std::endl;
    // obtain a time-based seed:
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(colors.begin(), colors.end(), std::default_random_engine(seed));
    const auto start_point = cv::Point2i(render_size.width / 2, render_size.height / 2);

    std::cout << "Colorizing..." << std::endl;
    auto t_before_colorize = std::chrono::high_resolution_clock::now();
    auto snapshots_images = colorize(out, colors, start_point, snapshot_count);
    auto t_after_colorize = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> d = t_after_colorize - t_before_colorize;
    std::cout << std::endl << "Colorize duration: " << d.count() << "s" << std::endl;

    const auto time_stamp = timestamp();
    std::experimental::filesystem::path output_dir{"results/" + time_stamp};

    std::cout << "Creating output directory: " << output_dir << std::endl;

    std::experimental::filesystem::create_directories(output_dir);

    std::cout << "Saving Image..." << std::endl;
    cv::imwrite(output_dir.string() + "/" + output_filename + ".png", out);

    if(!snapshots_images.empty()) {
        std::cout << "Saving " << snapshots_images.size() << " snapshots..." << std::endl;

        for (std::size_t i = 0; i != snapshots_images.size(); i++)
            cv::imwrite(output_dir.string() + "/" + output_filename + "_" + std::to_string(i) + ".png", snapshots_images[i]);

    }

    std::cout << "Writing info file..." << std::endl;
    write_info_file(output_dir.string() + "/" + output_filename + "_info.txt", time_stamp, render_size,
                    d.count(), snapshots_images.size());

    std::cout << "Done." << std::endl;

    return 0;
}
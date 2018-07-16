//
// Created by necator on 7/10/18.
//

#include "utils.h"

std::ostream& print_progress(int value, int end_value, std::ostream& os) {
    if(os) {
        constexpr auto bar_length{20};
        constexpr auto title{"Colorize Progress:"};
        auto percent = static_cast<float>(value) / end_value;
        std::string arrow = std::string(
                static_cast<std::size_t>(std::max(0.0f, std::round(percent * bar_length) - 1)), '-') + '>';
        std::string spaces(bar_length - arrow.size(), ' ');

        os << "\r" << title << " [" << arrow << spaces << "] " << static_cast<int>(std::round(percent * 100));
        os.flush();
    }

    return os;
}

std::ostream& print_delta_time_graph(std::ostream& os) {
    if(os) {
        constexpr auto y_resolution{10};

        static std::vector<std::chrono::high_resolution_clock::time_point> samples;
        samples.push_back(std::chrono::high_resolution_clock::now());

        if(samples.size() > 1) {
            std::chrono::duration<double, std::milli> max_elapsed(0);

            for(std::size_t i = 1; i < samples.size(); i++) {
                std::chrono::duration<double, std::milli> elapsed = samples[i] - samples[i - 1];

                if (elapsed > max_elapsed)
                    max_elapsed = elapsed;
            }

            std::stringstream ss;

            for(std::size_t height = y_resolution; height > 0; height--) {
                for(std::size_t i = 1; i < samples.size(); i++) {
                    std::chrono::duration<double, std::milli> elapsed = samples[i] - samples[i - 1];

                    auto percent = elapsed / max_elapsed;
                    auto current_duration_graph_height = static_cast<std::size_t>(std::round(percent * y_resolution));

                    ss << (height <= current_duration_graph_height ? '|' : ' ');
                }

                ss << "\n";
            }

            os << ss.str();
            os.flush();
        }
    }

    return os;
}

// old / random code
//cv::Vec3b get_random_color(std::vector<cv::Vec4b>& colors) {
//    std::random_device rd;     // only used once to initialise (seed) engine
//    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
//    std::uniform_int_distribution<std::size_t> uni(0, colors.size()); // guaranteed unbiased
//    const auto idx = uni(rng);
//    auto& random_color = colors[idx];
//    // mark as unavailable
//    random_color[3] = 0;
//
//    return cv::Vec3b(random_color[0], random_color[1], random_color[2]);
//}

//template<typename Iterator>
//Iterator parallel_min_diff_color(Iterator begin, Iterator end, const cv::Vec3b& reference) {
//    const auto ref = cv::Vec4b(reference[0], reference[1], reference[2], 1);
//    const auto length = std::distance(begin, end);
//    const auto min_segment_size{1000};
//    const int nr_min_segments = length / min_segment_size;
//
//    const int hardware_concurrency = std::thread::hardware_concurrency();
//    const auto nr_threads = std::min(nr_min_segments, std::max(1, hardware_concurrency));
//
//    const auto segment_size = length / nr_threads;
//
//    auto comp = [&ref] (const auto& a, const auto& b) {
//        return a[3] == 1 && color_diff(a, ref) < color_diff(b, ref);
//    };
//
//    std::vector<std::future<Iterator>> futures;
//
//    auto segment_begin = begin;
//
//    for(auto i = 0; i < (nr_threads - 1); i++) {
//        auto segment_end = segment_begin;
//        std::advance(segment_end, segment_size);
//
//        futures.push_back(std::async(std::launch::async, [&segment_begin, &segment_end, &comp] {
//            return std::min_element(segment_begin, segment_end, comp); }));
//
//        segment_begin = segment_end;
//    }
//
//    std::vector<Iterator> results;
//
//    results.push_back(std::min_element(segment_begin, end, comp));
//
//    for(auto& f : futures)
//        results.push_back(f.get());
//
//    Iterator min_it = results[0];
//
//    for(auto& it : results) {
//        if (comp(*it, ref) < comp(*min_it, ref))
//            min_it = it;
//    }
//
//    return min_it;
//}
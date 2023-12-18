#pragma once
#include <atomic>
#include <functional>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

class ProgressTracker
{
private:
    std::thread t_print;
    std::chrono::_V2::system_clock::time_point start_time;
    std::chrono::_V2::system_clock::time_point end_time;
    std::atomic<bool> running;
    std::atomic<int> total;
    std::atomic<int> progress;

    static std::string get_timer_string(int seconds)
    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << seconds / 3600 // HH
           << ":"
           << std::setw(2) << std::setfill('0') << seconds / 60 % 60 // MM
           << ":"
           << std::setw(2) << std::setfill('0') << seconds % 60; // SS

        return ss.str();
    }

    static void print_progress(size_t i, size_t n, std::chrono::_V2::system_clock::time_point start_time)
    {
        static unsigned short WIDTH = 70;
        auto now = std::chrono::high_resolution_clock::now();
        auto seconds_passed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        float p = (float)i / (float)n;

        std::cout << "[" << get_timer_string(seconds_passed) << "|";

        int pos = WIDTH * p;
        for (int i = 0; i < WIDTH; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(p * 100.0) << "% \r" << std::flush;
    }

    void main()
    {
        while (running.load())
        {
            print_progress(progress.load(), total.load(), start_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        print_progress(1, 1, start_time);

        std::cout << std::endl;
    }

public:
    ProgressTracker() : progress(0), total(1), running(true)
    {
        // Start thread.
        t_print = std::thread(&ProgressTracker::main, this);
        start_time = std::chrono::high_resolution_clock::now();
    }

    ~ProgressTracker()
    {
        if (t_print.joinable())
        {
            running.store(false);
            t_print.join();
        }
    }

    void set(size_t i, size_t n)
    {
        progress.store(i);
        total.store(n);
    }

    auto bind()
    {
        return std::bind(&ProgressTracker::set, this, std::placeholders::_1, std::placeholders::_2);
    }

    void stop()
    {
        end_time = std::chrono::high_resolution_clock::now();

        if (t_print.joinable())
        {
            running.store(false);
            t_print.join();
        }
    }

    inline std::string get_time()
    {
        auto seconds_passed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
        return get_timer_string(seconds_passed);
    }

    inline int get_throughput()
    {
        auto seconds_passed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
        auto iterations_per_second = (float)seconds_passed / (float)total.load();
        return iterations_per_second;
    }
};
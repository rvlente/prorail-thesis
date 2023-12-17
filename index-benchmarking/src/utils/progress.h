#pragma once
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

class ProgressBar
{
private:
    std::thread t_print;
    std::atomic<bool> running;
    std::atomic<int> total;
    std::atomic<int> progress;

    static void print_progress(size_t i, size_t n, std::chrono::_V2::system_clock::time_point start_time)
    {
        static unsigned short WIDTH = 70;
        auto now = std::chrono::high_resolution_clock::now();
        auto seconds_passed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        float p = (float)i / (float)n;

        std::cout << "["
                  << std::setw(2) << std::setfill('0') << seconds_passed / 3600 // HH
                  << ":"
                  << std::setw(2) << std::setfill('0') << seconds_passed / 60 % 60 // MM
                  << ":"
                  << std::setw(2) << std::setfill('0') << seconds_passed % 60 // SS
                  << "|";

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
        auto start_time = std::chrono::high_resolution_clock::now();

        while (running.load())
        {
            print_progress(progress.load(), total.load(), start_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        print_progress(1, 1, start_time);

        std::cout << std::endl;
    }

public:
    ~ProgressBar()
    {
        if (t_print.joinable())
        {
            running.store(false);
            t_print.join();
        }
    }

    void start()
    {
        if (running.load())
        {
            return;
        }

        // Start thread first.
        progress.store(0);
        total.store(1);
        running.store(true);
        t_print = std::thread(&ProgressBar::main, this);
    }

    void set(size_t i, size_t n)
    {
        progress.store(i);
        total.store(n);
    }

    auto bind()
    {
        return std::bind(&ProgressBar::set, this, std::placeholders::_1, std::placeholders::_2);
    }

    void stop()
    {
        if (t_print.joinable())
        {
            running.store(false);
            t_print.join();
        }
    }
};
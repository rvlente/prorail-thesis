#pragma once
#include <chrono>
#include <iostream>

class ProgressBar
{
private:
    const unsigned short WIDTH = 70;
    size_t n_iterations;
    size_t every;
    std::chrono::_V2::system_clock::time_point start_time;

public:
    ProgressBar(size_t n_iterations) : n_iterations(n_iterations)
    {
        every = n_iterations / 10000;
    }

    void start()
    {
        start_time = std::chrono::high_resolution_clock::now();
        update(0, true);
    }

    void update(size_t i, bool force = false)
    {
        if (!force && i % every != 0)
        {
            return;
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto seconds_passed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        const float progress = (float)i / (float)n_iterations;

        std::cout << "["
                  << std::setw(2) << std::setfill('0') << seconds_passed / 3600 // HH
                  << ":"
                  << std::setw(2) << std::setfill('0') << seconds_passed / 60 % 60 // MM
                  << ":"
                  << std::setw(2) << std::setfill('0') << seconds_passed % 60 // SS
                  << "|";

        int pos = WIDTH * progress;
        for (int i = 0; i < WIDTH; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << "% \r";
        std::cout.flush();
    }

    void finish()
    {
        update(n_iterations, true);
        std::cout << std::endl;
    }
};
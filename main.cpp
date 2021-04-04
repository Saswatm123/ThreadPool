#include <iostream>
#include "threadpool.hpp"
#include <atomic>

#ifndef TIMER_CHRONO_HPP
#define TIMER_CHRONO_HPP

    #include <iostream>
    #include <chrono>

    #define TIMING

    #ifdef TIMING
        #define INIT_TIMER auto start = std::chrono::high_resolution_clock::now();
        #define START_TIMER start = std::chrono::high_resolution_clock::now();
        #define STOP_TIMER(name)  std::cout << "RUNTIME of " << name << ": " << \
            std::chrono::duration_cast<std::chrono::milliseconds>( \
                    std::chrono::high_resolution_clock::now()-start \
            ).count() << " ms " << std::endl;
    #else
        #define INIT_TIMER
        #define START_TIMER
        #define STOP_TIMER(name)
    #endif

#endif // TIMER_CHRONO_HPP

#include <vector>

void v_write(std::vector<int>& v, int pos)
{
    v[pos] = pos;
}

int main()
{
    INIT_TIMER
    std::vector<int> v(1000);
    {
        threadpool tp(8);

        START_TIMER

        // First bug: make a < 1000 because v size is 1000
        for(int a = 0; a < 1000; a++)
        {
            tp.submit_task(v_write, v, a);
        }
    }
    STOP_TIMER("v write")

    /*
    std::cout << v.size() << std::endl;

    for(int a = 0; a < 1000; a++)
    {
        std::cout << v[a] << std::endl;
    }*/

    /*
    std::condition_variable cv;
    std::mutex m;
    std::unique_lock<std::mutex> ulock(m);
    cv.wait(ulock, [&](){std::cout << "This is called" << std::endl; return 0;} );*/
}

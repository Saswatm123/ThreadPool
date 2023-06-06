#include <iostream>
#include "threadpool.hpp"

#include "utils/timer.hpp"
#include "utils/construction_logger.hpp"

#include <chrono>
#include <thread>

void demo_func(ConstructionLogger i, bool b = false)
{
    std::cout << "Func called" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200) );
}

int main()
{
    ConstructionLogger c(false);

    INIT_TIMER
    {
        ThreadPool tp(8);

        START_TIMER

        for(int a = 0; a < 16; a++)
        {
            tp.submit_task(demo_func, c);
        }
    }
    STOP_TIMER("multi-threaded")

    START_TIMER
    for(int a = 0; a < 16; a++)
    {
        demo_func(c);
    }
    STOP_TIMER("single-threaded")

    ConstructionLogger::report();
}

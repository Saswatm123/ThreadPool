#include <iostream>
#include "threadpool.hpp"

#include "utils/timer.hpp"
#include "utils/construction_logger.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <unistd.h> // sleep

void demo_func(ConstructionLogger i)
{
    std::cout << "Func called" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200) );
}

void demo_threadpool(unsigned n_threads, unsigned n_tasks = -1)
{
    if(n_tasks == -1)
    {
        n_tasks = n_threads * 2;
    }

    ConstructionLogger c(false);

    INIT_TIMER
    {
        ThreadPool tp(n_threads);

        START_TIMER

        for(int a = 0; a < n_tasks; a++)
        {
            tp.submit_task(demo_func, c);
        }
    }
    STOP_TIMER("multi-threaded")

    START_TIMER
    for(int a = 0; a < n_tasks; a++)
    {
        demo_func(c);
    }
    STOP_TIMER("single-threaded")

    ConstructionLogger::report();
}

int main()
{
    /*
    ThreadPool ptp(80);

    std::string s = "asdf";
    for(int a = 0; a < 8000; a++)
    {
        ptp.submit_task(test, a);
    }

    sleep(1);
    */

    demo_threadpool(4000, 160);
}

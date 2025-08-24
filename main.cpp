#include <iostream>
#include "threadpool.hpp"

#include "utils/timer.hpp"
#include "utils/construction_logger.hpp"

#include <chrono>
#include <string>
#include <thread>

void demo_func(ConstructionLogger i)
{
    std::cout << "Func called" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200) );
}

void demo_threadpool()
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

void test(int i)
{
    //std::cout << i;
    //std::cout.flush();
}

#include <unistd.h>

int main()
{
    // demo_threadpool();
    ThreadPool ptp(80);

    std::string s = "asdf";
    for(int a = 0; a < 8000; a++)
    {
        ptp.submit_task(test, a);
    }

    sleep(1);
}

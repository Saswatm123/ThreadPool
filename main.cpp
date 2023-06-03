#include <iostream>
#include "threadpool.hpp"
#include <atomic>

#include "utils/timer.hpp"

#include <vector>

#include <chrono>
#include <thread>

void v_write(std::vector<int>& v, int pos)
{
    v[pos] = pos;
    std::this_thread::sleep_for(std::chrono::seconds(1) );
}

int main()
{
    INIT_TIMER
    std::vector<int> v(16);
    {
        threadpool tp(8);

        START_TIMER

        for(int a = 0; a < 16; a++)
        {
            tp.submit_task(v_write, v, a);
        }
    }
    STOP_TIMER("threadpool")

    START_TIMER
    for(int a = 0; a < 16; a++)
    {
        v_write(v, a);
    }
    STOP_TIMER("single-threaded")
}

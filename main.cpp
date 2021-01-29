#include <iostream>
#include "threadpool.hpp"
#include <atomic>

class ConstructionLogger
{
    static std::atomic<int> total_default;
    static std::atomic<int> total_copy;
    static std::atomic<int> total_move;

public:
    ConstructionLogger()
    {
        total_default++;
    }

    ConstructionLogger(const ConstructionLogger&)
    {
        total_copy++;
    }

    ConstructionLogger(const ConstructionLogger&&)
    {
        total_move++;
    }

    static void report()
    {
        std::cout << "CONSTRUCTION LOGGER:" << std::endl;
        std::cout << "\tDEFAULT CONSTRUCTOR WAS CALLED:\t" << total_default << " TIMES" << std::endl;
        std::cout << "\tCOPY CONSTRUCTOR WAS CALLED:\t" << total_copy << " TIMES" << std::endl;
        std::cout << "\tMOVE CONSTRUCTOR WAS CALLED:\t" << total_move << " TIMES" << std::endl;
    }
};

std::atomic<int> ConstructionLogger::total_default(0);
std::atomic<int> ConstructionLogger::total_copy(0);
std::atomic<int> ConstructionLogger::total_move(0);

void func(ConstructionLogger d)
{}

int main()
{
    using namespace std::chrono_literals;

    {
        threadpool tp(2);

        for(int a = 0; a < 1; a++)
        {
            tp.submit_task(func, ConstructionLogger() );
        }
        std::cout << "END\n";
    }
    std::this_thread::sleep_for(100ms);
    ConstructionLogger::report();
}

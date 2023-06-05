#ifndef CONSTRUCTION_LOGGER_HPP
#define CONSTRUCTION_LOGGER_HPP

class ConstructionLogger
{
    static std::atomic<int> total_default;
    static std::atomic<int> total_copy;
    static std::atomic<int> total_move;

    bool verbose = 0;

public:
    ConstructionLogger(bool verbose)
    :verbose(verbose)
    {
        if(this->verbose)
        {
            std::cout << "Constructor Being Called" << std::endl;
        }
        total_default++;
    }

    ConstructionLogger(const ConstructionLogger&)
    {
        if(this->verbose)
        {
            std::cout << "Copy Constructor Being Called" << std::endl;
        }
        total_copy++;
    }

    ConstructionLogger(const ConstructionLogger&&)
    {
        if(this->verbose)
        {
            std::cout << "Move Constructor Being Called" << std::endl;
        }
        total_move++;
    }

    static void report()
    {
        std::cout << "CONSTRUCTION LOGGER:" << std::endl;
        std::cout << "\tDEFAULT CONSTRUCTOR WAS CALLED:\t" << total_default << " TIMES" << std::endl;
        std::cout << "\tCOPY CONSTRUCTOR WAS CALLED:\t" << total_copy << " TIMES" << std::endl;
        std::cout << "\tMOVE CONSTRUCTOR WAS CALLED:\t" << total_move << " TIMES" << std::endl;
    }

    ~ConstructionLogger()
    {
        if(this->verbose)
        {
            std::cout << "Destructor Being Called" << std::endl;
        }
    }
};

std::atomic<int> ConstructionLogger::total_default(0);
std::atomic<int> ConstructionLogger::total_copy(0);
std::atomic<int> ConstructionLogger::total_move(0);

#endif // CONSTRUCTION_LOGGER_HPP

#ifndef TIMER_CHRONO_HPP
#define TIMER_CHRONO_HPP

    #include <iostream>
    #include <chrono>

    #define TIMING

    #ifdef TIMING
        #define INIT_TIMER auto start = std::chrono::high_resolution_clock::now();
        #define START_TIMER  start = std::chrono::high_resolution_clock::now();
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

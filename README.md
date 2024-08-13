# ThreadPool C++14

This is a simple C++14 Thread Pool implementation. I will detail how to use it, as well as how it works internally, in this README. 

- [Usage](#usage)
- [Engineering Challenges](#engineering-challenges)
    - [Storing Work, Functions, Arguments](#storing-work-functions-arguments)

## Usage
There is demonstration code written in `main.cpp`, and uses extra classes I wrote for throrough testing. I will go over usage here, but feel free to look
at the extra examples in `main.cpp`. Here is some sample code:

We initialize our `ThreadPool` using the following syntax:
```
int main()
{
    ThreadPool tp(8);
}
```
This makes a pool of 8 threads waiting to be put to work. They are created upon construction & wake up when the pool has work to do. Until then, they are sleeping.

Now we have to create some work for our threadpool to run. Let's make a test function:
```
#include <iostream>
#include "threadpool.hpp"
#include "utils/construction_logger.hpp"

void demo_func(ConstructionLogger i, bool b = false)
{
    std::cout << "Func called" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200) );
}

int main()
{
    ThreadPool tp(8);
}
```
`ConstructionLogger` is a helper class in `utils` that I use for testing and making sure we are using perfect forwarding each time. We will use this helper class in our example. `demo_func` takes in one of these.
We now simply submit this task to our `ThreadPool` as follows:

```
#include <iostream>
#include "threadpool.hpp"

void demo_func(ConstructionLogger i)
{
    std::cout << "Func called" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200) );
}

int main()
{
    ConstructionLogger c(false);
    ThreadPool tp(8);

    for(int a = 0; a < 16; a++)
    {
        tp.submit_task(demo_func, c);
    }
}
```
The method `submit_task` returns a `std::future<ReturnType>` with `ReturnType` being the return type of the function passed in. We can use these futures (https://en.cppreference.com/w/cpp/thread/future) as regular C++ futures, and retrieve values/block when needed. Since the demo function we use returns `void`, we simply don't collect any of the futures returned, since they're meaningless.

## Engineering Challenges

### Storing Work, Functions, Arguments

We store the work required in a Linked List for O(1) addition & removal. This is because we don't know when work will be completed, so we don't know in what order
we will have to remove work - we have to be ready to remove work from anywhere. We get iterators to these Linked List nodes for O(1) retrieval, and store these
in a Queue. Worker threads, if free, upon being notified that there is a new piece of work to be done, remove this "work ticket" from the front of the Queue.
They then get to work completing the function call based on the function pointer & arguments stored in the work object. Upon completion,
they remove its corresponding Work node, containing the function information, as well as the arguments to be passed in, from the linked list, and go back & wait
for more work to be available.

Work itself is stored as a `BoundFunction`, viewable in the `impl/BoundFunction.hpp` file. A `BoundFunction` refers to a function bound to its arguments. For Polymorphism reasons, we actually store it as a pointer to its abstract class `GenericBoundFunction`, but we will talk about `BoundFunction` here. `BoundFunction` is a template class that takes in a function pointer & arguments
to pass into the function pointer. The full implementation can be seen in `bound_function.hpp`.

There are two `BoundFunction` templates - one is the default, that takes in `<ReturnType, ArgPack...>` as its template arguments, and the other is a specialization for functions that return `void` (so `<void, ArgPack...>`). This is because function results are returned via `std::future<ReturnType>`, which expects different behavior if we instantiate a `std::future<void>`. These both inherit from the abstract class `GenericBoundFunction` to provide the same interface. `BoundFunction`s store functions as a const function pointer, and the arguments are stored via `std::tuple<ArgTypes...>`. 

`impl/bound)function.hpp` and `impl/bound_function.tpp` contain the code for this, so feel free to take a look. I will finish this documentation later.


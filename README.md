# ThreadPool C++14

This is a simple C++14 Thread Pool implementation. I will detail how to use it, as well as how it works internally, in this README. 

- [Usage](#usage)
- [Engineering Challenges](#engineering-challenges)
    - [Storing Work, Functions, Arguments](#storing-work,-functions,-arguments)
    - [Default Arguments with Function Pointers](#default-arguments-with-function-pointers)

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
`ConstructionLogger` is a helper class in `utils` that I use for testing and making sure we are using perfect forwarding each time. We will use this helper class in our example. `demo_func` takes in one of these as well as a default boolean argument. Note: most threadpool implementations don't allow for default arguments in functions, because of the issues with function pointers and default arguments, but this one handles those issues.
We now simply submit this task to our `ThreadPool` as follows:

```
#include <iostream>
#include "threadpool.hpp"

void demo_func(ConstructionLogger i, bool b = false)
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

Work itself is stored as a `BoundFunction`, viewable in the `impl/BoundFunction.hpp` file. For Polymorphism reasons, we actually store it as a pointer to its 
abstract class `GenericBoundFunction`, but we will talk about `BoundFunction` here. `BoundFunction` is a template class that takes in a function pointer & arguments
to pass into the function pointer. The full implementation can be seen in `BoundFunction.hpp`.

### Default Arguments with Function Pointers

Function pointers often have the misconception that they can be treated exactly like function calls. If we have the following code:

```
#include <iostream>

void func(int i)
{
    std::cout << i << std::endl;
}

int main()
{
    void(*fptr)(int, bool) = func;
    fptr(0);

    return 0;
}
```

Then this code runs as expected. However, this changes if we add default arguments to our function, as follows:

```
#include <iostream>

void func(int i, bool verbose = false)
{
    std::cout << i << std::endl;
}

int main()
{
    void(*fptr)(int, bool) = func;
    fptr(0);

    return 0;
}
```

This gives us a compilation error, since the function signature of `func` tells the compiler that there should be two arguments. However, we know the last one should be optional. Getting around this with just the information provided above is widely believed to not be possible without creating another function/lambda to get around this issue (https://stackoverflow.com/questions/76351178/when-calling-a-function-through-a-function-pointer-why-is-it-that-default-argum). However, I've found another way to get around this, which allows for added user simplicity, since the user doesn't have to create extra function bindings/lambdas/etc.
In order to get around this, the first step is to perform a `reinterpret_cast` from the type of the function pointer we have to the type of the function pointer that leaves out the default arguments that aren't provided.
Here is a simple example:

```
#include <iostream>

void func(int i, bool verbose = false)
{
    std::cout << i << std::endl;
}

int main()
{

    void(*fptr)(int) = reinterpret_cast<void(*)(int)>(func);
    fptr(0);

    return 0;
}
```

This gets us around the compiler, which now sees nothing wrong with this function call. This call works exactly as expected, too, with the default argument filling in as expected.
This raises some problems, however. How do we know, in general, the types of the arguments passed in, the arguments required (not default), and the type of the function?
We use template metaprogramming to solve these issues.

In order to know the type of the function to cast to, we need to know the *return type of the function*, and the *parameter types of the non-defaults*. In `impl/bound_function.tpp`,
we have a chunk of code that looks like this:
```

template<typename ReturnType, typename... ParamTypes, typename... ArgumentFwdTypes>
auto make_bound_function(ReturnType(func)(ParamTypes...), ArgumentFwdTypes&&... args)
{
    auto shortened_function = shorten_function_paramlist(func, std::forward<ArgumentFwdTypes>(args)...);
    return BoundFunction<ReturnType, decltype(shortened_function), ArgumentFwdTypes...>(shortened_function, std::forward<ArgumentFwdTypes>(args)...);
}

```
that we can call like this:
```
// Assume function "func" from previous code snippets is still in scope
make_bound_function(func, 0)
```
to return an object that, when called, returns `func(0, verbose = false)`. Since the first argument passed into `make_bound_function` is the function in question,
we can deduce the *return type of the function*, as well as the *entire expected parameter list* of the function. The second argument is a variadic argument pack, 
containing everything we would like to pass into our function, with variadic type pack `ArgumentFwdTypes...`. It is important to note that the parameter types the function
expects, and the argument types we pass in are not always the same, whether it is due to user error, expected type-casting, or leaving out default arguments.
This means that, as an issue of safety, we should preserve the original parameter types for the arguments we know. How do we know which parameters we should keep, then?
Default arguments come at the end of a parameter list. This means that if we take the size of our passed in argument pack, referred to as some number `N`, and pick the first
`N` elements of our parameter type list, we can get the non-default expected types of our function, while also preserving the original type info! We can then use
this "shortened parameter pack" to perform a `reinterpret_cast` to our expected function pointer type. The code for this is written in `impl/func_tools.hpp`, and
the important part of it is shown below:

```
// This function assists shorten_function_paramlist, which is the public-facing function.
template<typename ReturnType, typename... TruncatedParamTypes, typename... ParamTypes, typename... ArgTypes>
ReturnType(*_shorten_function_paramlist_impl(TypeHolder<TruncatedParamTypes...>, ReturnType(function)(ParamTypes...), ArgTypes&&... args) )(TruncatedParamTypes...)
{
    return reinterpret_cast<ReturnType(*)(TruncatedParamTypes...)>(function);
}

template<typename ReturnType, typename... ParamTypes, typename... ArgTypes>
auto shorten_function_paramlist(ReturnType(function)(ParamTypes...), ArgTypes&&... args)
{
    return _shorten_function_paramlist_impl(
        typename FirstNTypes<sizeof...(args), ParamTypes...>::type(),
        function,
        std::forward<ArgTypes>(args)...
    );
}
```

There are many pieces of code used in the code snippet above, such as `FirstNTypes<...>`, that are not defined in the snippet (for brevity), but can be viewed at `impl/func_tools.hpp`. 
By using this code above, we can take in our function `func` from the earlier code snippets, which has the signature `void(*)(int, bool)`, pass in the argument
list `0 (or any int)`, then since there is only one type in our passed in arguments, we select the first argument in the original parameter list, which is `(int)`, 
to get our final type, `void(*)(int)`! We can now call this with our original arguments, and the defaults get filled in automatically. We can even include the dafaults
if we want, since this just increases the number of arguments we take from the front, and leaves the rest alone! The full logic chain to learn how to wrangle with
everything I mentioned above can be found starting with the function `make_bound_function` in `impl/bound_function.tpp` - just follow the function calls & 
template metaprogramming code from there.

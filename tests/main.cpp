#include <iostream>
#include <format>
#include <scheduler.hpp>

using namespace std::chrono_literals;

Scheduler sched;

void countdown(int n)
{
    if (n > 0) {
        std::cout << std::format("Down => {}\n", n) << std::endl;
        sched.call_later(4s, Task(countdown, n - 1));
    }
}

void countup(int start, int stop)
{
    if (start < stop) {
        std::cout << std::format("Up => {}\n", start);
        sched.call_later(1s, Task(countup, start + 1, stop));
    }
}


int main()
{
    sched.call_soon(Task(countdown, 5));
    sched.call_soon(Task(countup, 0, 5));
    sched.run();
}
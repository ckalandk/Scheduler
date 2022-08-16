#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <tuple>
#include <queue>
#include <functional>
#include <chrono>
#include <thread>
#include <concepts>
#include <type_traits>
#include <atomic>
    

struct no_op { void operator()() const { } };

struct AbstractTask
{
    virtual ~AbstractTask() = default;
    virtual void call() const = 0;
    virtual AbstractTask* clone() const = 0;
};


template <typename Fn, typename... Args>
requires std::invocable<Fn, Args...>
struct TaskImpl : AbstractTask
{
    using Tuple = std::tuple<std::decay_t<Fn>, std::decay_t<Args>...>;

    TaskImpl() noexcept
        :m_tup{}
    { }

    template <typename Fn_, typename... Args_>
    TaskImpl(Fn_&& fn, Args_&&... args)
        :m_tup(std::forward<Fn>(fn), std::forward<Args>(args)...)
    { }

    void call() const override
    {
        call_impl(std::make_index_sequence<1 + sizeof... (Args)>{});
    }

    AbstractTask* clone() const override
    {
        return new TaskImpl(m_tup);
    }
private:
    Tuple m_tup;

private:
    TaskImpl(Tuple const& tuple)
        :m_tup(tuple)
    { }

    template <size_t...Indices>
    void call_impl(std::index_sequence<Indices...>) const
    {
        std::invoke(std::get<Indices>(m_tup)...);
    }
};

struct Task
{
    Task() noexcept
        :m_call{nullptr}
    {

    }

    template <typename Fn, typename... Args>
    requires (!std::is_same_v<std::remove_cvref_t<Fn>, Task>)
    Task(Fn&& fn, Args&&... args)
    {
        m_call = new TaskImpl<Fn, Args...>(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    Task(Task const& other)
        :m_call(other.m_call->clone())
    { }

    Task& operator=(Task const& other)
    {
        if(this != &other)
            m_call = other.m_call->clone();
        return *this;
    }

    Task(Task&& other) noexcept
        :m_call(other.m_call)
    {
        other.m_call = nullptr;
    }

    Task& operator=(Task&& other) noexcept
    {
        m_call = other.m_call;
        other.m_call = nullptr;
        return *this;
    }

    ~Task() 
    {
        if (m_call)
            delete m_call;
    }


    void operator()() const
    {
        m_call->call();
    }
private:
    AbstractTask* m_call;
};


namespace std
{
    template <>
    struct greater<std::tuple<std::chrono::nanoseconds, size_t, Task>>
    {
        constexpr bool operator()(const std::tuple<std::chrono::nanoseconds, size_t, Task>& left, 
            const std::tuple<std::chrono::nanoseconds, size_t, Task>& right) const {
            if (std::get<0>(left) > std::get<0>(right))
                return true;
            else if (std::get<0>(left) < std::get<0>(right))
                return false;
            return (std::get<1>(left) > std::get<1>(right));
        }
    };
}

class Scheduler
{
    using Tuple = std::tuple<std::chrono::nanoseconds, size_t, Task>;
    using PQueue = std::priority_queue <Tuple, std::vector<Tuple>, std::greater<Tuple>>;
   
public:

    Scheduler() = default;

    void call_soon(Task const& task)
    {
        m_ready.push(task);
    }
    
    template <typename Rep, typename Periode>
    void call_later(std::chrono::duration<Rep, Periode> delay, Task const& task)
    {
        auto const deadline = std::chrono::steady_clock::now().time_since_epoch() + delay;    //expiration time
        m_waiting.push({ deadline, m_index++, task });
    }
            
    void run()
    {
        while (!m_ready.empty() || !m_waiting.empty()) {
            if (m_ready.empty()) {
                // find the nearest deadline
                auto&& [deadline, _, task] = m_waiting.top(); 
                auto const delta = deadline - std::chrono::steady_clock::now().time_since_epoch();
                if (delta.count() > 0)
                    std::this_thread::sleep_for(delta);
                m_ready.push(task);
                m_waiting.pop();
            }
            while (!m_ready.empty()) {
                auto&& task = m_ready.front();
                task();
                m_ready.pop();
            }
        }
    }

private:
    std::queue<Task> m_ready{};
    PQueue m_waiting{};
    inline static size_t m_index{};
};

#endif //scheduler.hpp
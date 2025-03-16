#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <chrono>

using namespace std;

class ThreadPool {
private:
    int m_threads;
    vector<thread> threads;
    queue<function<void()>> tasks;
    mutex mtx;
    condition_variable cv;
    bool stop;

public:
    explicit ThreadPool(int numthreads) : m_threads{numthreads}, stop{false} {
        for (int i = 0; i < m_threads; i++) {
            threads.emplace_back([this]() { //lambda function for thread-->each thread executes the follwing lambda function
                while (true) { //keep the thread running forever 
                    function<void()> currtask;
                    unique_lock<mutex> lock(mtx);
                    cv.wait(lock, [this] {
                        return !tasks.empty() || stop;
                    });/*cv.wait(lock, condition) puts the thread to sleep until the given condition becomes true that is when the queue is empty
                    it just puts thread to sleep until a tasks actually comes
                    Instead of a wasteful busy wait (checking in a loop repeatedly), cv.wait() ensures
                     threads sleep efficiently and only wake up when necessary*
                     */
                    if (stop) return;
                    /* Try removing stop: 
   - If we remove stop, when both (sequential and thread pool) executions are done, the program won’t exit.
   - This is because worker threads are still waiting for a task that will never come.
   - Even when the ThreadPool object is destroyed, threads remain waiting.
*/

/* Why do we need stop?
   - `stop` ensures threads exit gracefully when the destructor is called.
   - In the destructor, `stop = true`, which makes all waiting threads wake up.
   - When threads wake up, they check `if (stop) return;` and exit immediately.
   - The destructor then waits for all threads to finish and joins them.
*/
                    currtask = move(tasks.front());
                    tasks.pop();
                    lock.unlock(); // Unlock before running the task to allow other threads to pick tasks
                    currtask();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            unique_lock<mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto &th : threads) {
            th.join();
        }
    }

    template<class F, class... Args> //any callable dunction with any number of argument
    auto executetasks(F&& f, Args&&... args) -> future<decltype(f(args...))> {//1 & 2
        using return_type = decltype(f(args...));
        auto task = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...));
        future<return_type> res = task->get_future();
        {
            unique_lock<mutex> lock(mtx);
            tasks.emplace([task]() {
                (*task)();
            });
        }
        cv.notify_one();
        return res;
    }
};
/*
1:
template<class F, class... Args> → This makes the function generic, so it can accept any callable (F), like a function, lambda, or functor, with any number of arguments (Args...).
F&& f → The function/task to be executed.
Args&&... args → Arguments for the function.
auto -> future<decltype(f(args...))> → Returns a std::future that holds the result of the function call.
decltype(f(args...)) determines the return type of f(args...).
The function will return std::future<return_type>, allowing the caller to retrieve the result asynchronously.
2:
trailing return type:
Before C++11, the return type of a function was typically specified before the function name. 
However, in some cases, it could be challenging to express complex return types, especially when dealing with template functions 
or functions with decltype.
With the trailing return type syntax, you can specify the return type of a function after the parameter 
list using the auto keyword and the trailing -> arrow. This allows you to use expressions and decltype to deduce the return 
type based on the function’s implementation.
The function's return type is based on decltype(f(args...)). Since F and Args... are templates, 
their exact types are unknown at the function declaration time.
--->not work:template<class F, class... Args>
std::future<decltype(f(args...))> executetasks(F&& f, Args&&... args); as :This won’t compile because decltype(f(args...)) depends on Args..., but f(args...) hasn’t been parsed yet.


*/

// Function that takes time to execute
int func(int a) {
    this_thread::sleep_for(chrono::seconds(1));
    cout << "Task executed by thread ID: " << this_thread::get_id() << endl;
    return a * a;
}

int main() {
    int numTasks =8;
    cout << "Number of tasks: " << numTasks << endl;
    
    // Sequential execution without thread pool
    auto start_seq = chrono::steady_clock::now();
    for (int i = 0; i < numTasks; i++) {
        func(i);
    }
    auto end_seq = chrono::steady_clock::now();
    cout << "Time taken without thread pool: " 
         << chrono::duration_cast<chrono::milliseconds>(end_seq - start_seq).count() 
         << " milliseconds" << endl;

    // Execution using thread pool
    ThreadPool pool(4);  // Using 4 threads
    vector<future<int>> results;

    auto start_pool = chrono::steady_clock::now();
    for (int i = 0; i < numTasks; i++) {
        results.push_back(pool.executetasks(func, i));
    }
    
    for (auto &res : results) {
        res.get();  // Wait for all tasks to complete
    }
    
    auto end_pool = chrono::steady_clock::now();
    cout << "Time taken with thread pool: " 
         << chrono::duration_cast<chrono::milliseconds>(end_pool - start_pool).count() 
         << " milliseconds" << endl;

    return 0;
}

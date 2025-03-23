# **Thread Pool**

### **A simple thread pool implementation in C++**

## **Table of Contents**
- [Introduction](#introduction)  
- [Code Walkthrough](#code-walkthrough)  

## **Introduction**
### **What is a Thread Pool?**
A **Thread Pool** is used to efficiently manage and execute multiple tasks using a fixed number of threads. Instead of creating and destroying threads for each task, the thread pool maintains a set of pre-created threads that can be **reused** to execute tasks concurrently.  

### **Why Use a Thread Pool?**
- **Reduces Overhead:** Eliminates the cost of frequent thread creation and destruction.  
- **Efficient Execution:** Tasks are queued and executed as soon as a thread is available.  
- **Better Resource Management:** Prevents excessive thread creation, which can degrade system performance.  

> **Disclaimer:** This is a beginner-level implementation. More robust and optimized implementations exist with better error handling and performance tuning.  

---

## **Code Walkthrough**

### **main()**
- Initializes `ThreadPool(num)`, where `num` is the number of worker threads.  
- The optimal number of threads depends on the system hardware and workload.  

### **func()**
- Represents a unit of work assigned to a thread.  
- Simulates task execution using sleep to represent computation or I/O operations.  
## **ThreadPool Constructor and Destructor**

### **Constructor: `ThreadPool(int numThreads)`**
- Initializes the thread pool with `numThreads` worker threads.  
- Each thread runs a **lambda function** that continuously checks for available tasks.  
- The threads **sleep efficiently** using a condition variable (`cv.wait()`) instead of using a wasteful busy wait.  
- A thread **wakes up** only when:  
  - A new task is available.  
  - The thread pool is being **stopped**.  
- The `stop` flag is used to **gracefully exit** threads when the thread pool is destroyed.  
- If `stop` was removed, the threads would remain **waiting indefinitely**, preventing the program from exiting.  

---

### **Destructor: `~ThreadPool()`**
- Ensures **all threads exit properly** when the thread pool is destroyed.  
- Sets `stop = true`, which **wakes up all sleeping threads** so they can exit.  
- Calls `cv.notify_all()` to unblock any waiting threads.  
- Uses `join()` to **wait for all threads** to finish execution before destruction.  
- Without `stop`, worker threads would remain **stuck waiting forever**, even after the ThreadPool object is destroyed.  

### Execute Tasks Function

#### Overview
The `execute_tasks` function utilizes generic programming in C++ to allow execution of any callable function with any number of arguments.

#### Template Definition
```cpp
template<class F, class... Args>
```
This template enables passing any callable function (`F`) along with any number of arguments (`Args...`).

#### Trailing Return Type (`auto ->`)
Before C++11, function return types were typically specified before the function name. However, expressing complex return types, especially in template functions, was often challenging.

With the trailing return type syntax, we can specify the return type after the function signature using the `auto` keyword and the `->` arrow notation. This allows us to use `decltype` to deduce the return type based on the function’s implementation.

#### Return Type Deduction
```cpp
using return_type = decltype(f(args...));
```
Since `F` and `Args...` are templates, their exact types are unknown at the time of function declaration. `decltype(f(args...))` deduces the return type dynamically.

### Key Idea of `std::packaged_task`

#### Purpose
- `std::packaged_task` wraps a function so that it does **not execute immediately** when called.
- Instead, it stores the function inside a callable task that can be executed later.
- If the function returns a value, it can be retrieved using `std::future`.

#### Example Use Case
```cpp
std::packaged_task<int()> task([] { return 42; });
std::future<int> result = task.get_future();
task();  // Executes the function
int value = result.get();  // Retrieves the return value (42)
```

### Real-World Use Case: Task Execution in a Thread Pool

#### Why Use `std::packaged_task` in a Thread Pool?
- In a thread pool, tasks are pushed into a queue, and worker threads execute them when they are ready.
- Example:
```cpp
threadPool.enqueue([]{ return add(5, 3); });  // Task added but not executed yet
```
- Worker threads pick up the task later and execute it.

#### Without `std::packaged_task`
- The function would have to execute **immediately** when added to the queue.

#### With `std::packaged_task`
- The function is stored **as a task** and executed later when a worker thread picks it up.
- The result can be retrieved asynchronously using `std::future`.

This mechanism allows efficient and controlled execution of tasks in a multi-threaded environment, making it essential for **thread pool implementations**.




 

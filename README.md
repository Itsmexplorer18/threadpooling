# **Thread Pool**
new implementation work stealing similar to forkjoinpool of java dynamic load balancing 

### **A simple thread pool implementation in C++**

## **Table of Contents**
- [Introduction](#introduction)  
- [Code Walkthrough](#code-walkthrough)  
  - [main()](#main)
  - [func()](#func)
  - [ThreadPool Constructor and Destructor](#threadpool-constructor-and-destructor)
    - [Constructor: `ThreadPool(int numThreads)`](#constructor-threadpoolint-numthreads)
    - [Destructor: `~ThreadPool()`](#destructor-threadpool)
  - [Execute Tasks Function](#execute-tasks-function)
    - [Key Idea of `std::packaged_task`](#key-idea-of-stdpackaged_task)
- [Understanding Thread Pool Performance at Different Time Scales](#understanding-thread-pool-performance-at-different-time-scales)
  - [Why Thread Pool Execution Appears Slower in Microseconds but Faster in Milliseconds](#why-thread-pool-execution-appears-slower-in-microseconds-but-faster-in-milliseconds)
  - [Disclaimer](#disclaimer)

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

### **ThreadPool Constructor and Destructor**

#### **Constructor: `ThreadPool(int numThreads)`**
- Initializes the thread pool with `numThreads` worker threads.  
- Each thread runs a **lambda function** that continuously checks for available tasks.  
- The threads **sleep efficiently** using a condition variable (`cv.wait()`) instead of using a wasteful busy wait.  
- A thread **wakes up** only when:  
  - A new task is available.  
  - The thread pool is being **stopped**.  
- The `stop` flag is used to **gracefully exit** threads when the thread pool is destroyed.  
- If `stop` was removed, the threads would remain **waiting indefinitely**, preventing the program from exiting.  

#### **Destructor: `~ThreadPool()`**
- Ensures **all threads exit properly** when the thread pool is destroyed.  
- Sets `stop = true`, which **wakes up all sleeping threads** so they can exit.  
- Calls `cv.notify_all()` to unblock any waiting threads.  
- Uses `join()` to **wait for all threads** to finish execution before destruction.  
- Without `stop`, worker threads would remain **stuck waiting forever**, even after the ThreadPool object is destroyed.  

### **Execute Tasks Function**

#### **Overview**
The `execute_tasks` function utilizes generic programming in C++ to allow execution of any callable function with any number of arguments.

#### **Template Definition**
```cpp
template<class F, class... Args>
```
This template enables passing any callable function (`F`) along with any number of arguments (`Args...`).

#### **Trailing Return Type (`auto ->`)**
Before C++11, function return types were typically specified before the function name. However, expressing complex return types, especially in template functions, was often challenging.

With the trailing return type syntax, we can specify the return type after the function signature using the `auto` keyword and the `->` arrow notation. This allows us to use `decltype` to deduce the return type based on the function’s implementation.

#### **Return Type Deduction**
```cpp
using return_type = decltype(f(args...));
```
Since `F` and `Args...` are templates, their exact types are unknown at the time of function declaration. `decltype(f(args...))` deduces the return type dynamically.

#### **Key Idea of `std::packaged_task`**

##### **Purpose**
- `std::packaged_task` wraps a function so that it does **not execute immediately** when called.
- Instead, it stores the function inside a callable task that can be executed later.
- If the function returns a value, it can be retrieved using `std::future`.

##### **Example Use Case**
```cpp
std::packaged_task<int()> task([] { return 42; });
std::future<int> result = task.get_future();
task();  // Executes the function
int value = result.get();  // Retrieves the return value (42)
```

##### **Real-World Use Case: Task Execution in a Thread Pool**

###### **Why Use `std::packaged_task` in a Thread Pool?**
- In a thread pool, tasks are pushed into a queue, and worker threads execute them when they are ready.
- Example:
```cpp
threadPool.enqueue([]{ return add(5, 3); });  // Task added but not executed yet
```
- Worker threads pick up the task later and execute it.

###### **Without `std::packaged_task`**
- The function would have to execute **immediately** when added to the queue.

###### **With `std::packaged_task`**
- The function is stored **as a task** and executed later when a worker thread picks it up.
- The result can be retrieved asynchronously using `std::future`.

---

# **Understanding Thread Pool Performance at Different Time Scales**

## **Why Thread Pool Execution Appears Slower in Microseconds but Faster in Milliseconds**

### **1. Microsecond Precision is Too Fine-Grained**
System overhead (thread scheduling, mutex locking, queue operations) distorts execution time at the microsecond level.

### **2. Overhead of Thread Synchronization**
Even for short tasks (~1µs), thread pool overhead includes:
- Mutex locking & unlocking.
- Condition variable wake-ups.
- Task scheduling delays.
- Thread creation overhead.

### **3. Context Switching Overhead**
Thread switches performed by the OS incur small delays. When tasks take only 1µs, context switching can dominate execution time.

### **4. Measuring in Microseconds Increases Jitter**
CPU cache misses, background processes, and system interrupts cause high variance in microsecond-scale measurements.

### A Simple Analogy:
Think of it like a car trip:
If your trip is only 1 meter, the time spent unlocking the car, putting on your seatbelt, and starting the engine is a huge portion of the total time.
But if your trip is 100 kilometers, the startup time is negligible compared to the total journey.
Similarly, at a microsecond scale, overhead dominates execution, but at a millisecond scale, overhead becomes insignificant compared to useful work.
Measuring execution time at microsecond precision introduces misleading results due to system-level overhead. To accurately gauge the benefits of multithreading, tasks should be long enough (milliseconds or more) to make the overhead insignificant. This is why thread pools may appear slower at a microsecond level but show their true advantages when measured at a larger time scale.
## **Disclaimer**
Performance results may vary based on system architecture, processor capabilities, and background processes. Users should experiment with different workloads and timing scales to determine what works best for their specific use case.


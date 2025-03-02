//threadpooling implementation with generic programming
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future> 
using namespace std;

class ThreadPool{
 private:
   int m_threads;
   vector<thread> threads;
   queue<function<void()>> tasks;
   mutex mtx;
   condition_variable cv;
   bool stop;
 public:
   explicit ThreadPool(int numthreads):m_threads{numthreads},stop{false}{
      for(int i=0;i<m_threads;i++){
         threads.emplace_back([this](){
                    function<void()> currtask;
            while(1){
               unique_lock<mutex> lock(mtx);
               cv.wait(lock,[this]{
                return !tasks.empty() or stop;
               });
               if(stop) return;
               currtask=move(tasks.front());
               tasks.pop();
               lock.unlock(); //why unlock?
               currtask();
            }
         });
      }
   }
    ~ThreadPool(){
        //we want wait for all curr running threads to be finished and stop value =true to notify all threads to np more take tasks
        unique_lock<mutex> lock(mtx);
        stop=true;
        lock.unlock();
        cv.notify_all();
        for(auto &th:threads){
            th.join();
        }
    }
    template<class F, class... Args>
auto executetasks(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    using return_type = decltype(f(args...));

    // Create a shared_ptr for packaged_task
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    // Get the future associated with the task
    std::future<return_type> res = task->get_future();

    // Add the task to the queue
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace([task]() {
            (*task)(); // Correctly dereference the shared_ptr and execute the task
        });
    }

    // Notify one thread to pick up the task
    cv.notify_one();

    return res;
}

   /* template<class F,class... Args>
    auto  executetasks(F&& f,Args&&... args)->future<decltype(f(args...))>{
        using return_type=decltype(f(args...));
        auto task=make_shared<packaged_task<return_type()>>(bind(forward<F>(f),forward<Args>(args...)));
        future<return_type> res=task->get_future();
        unique_lock<mutex> lock(mtx);
        tasks.emplace([task]()->void{
            *(task)();
        });
        lock.unlock();
        cv.notify_one();
        return res;
    }*/
   /* void executetasks(F f,Args... args){  //this is for multiple type of functions diff func may take diff no of arguments
    //now we want return type to be the return type of the function trailing return type 
        unique_lock<mutex> lock(mtx);
        tasks.push(func);
        lock.unlock();
        cv.notify_one();
    }
    */
   /* void executetasks(function<void()> func){
        unique_lock<mutex> lock(mtx);
        tasks.push(func);
        lock.unlock();
        cv.notify_one();*/
    };

//};
int func(int a){
    this_thread::sleep_for(chrono::seconds(2));
    cout<<"some tasks of user"<<endl;
    return a*a;
}
int main(){
            ThreadPool pool(8);
            future<int> res=pool.executetasks(func,1);
            cout<<res.get();
    while(1){
       // pool.executetasks(func);
       //cout<<"happening"<<endl;
    }
    return 0;
}

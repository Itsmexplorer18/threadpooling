#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
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
    void executetasks(function<void()> func){
        unique_lock<mutex> lock(mtx);
        tasks.push(func);
        lock.unlock();
        cv.notify_one();
    }

};
void func(){
    this_thread::sleep_for(chrono::seconds(2));
    cout<<"some tasks of user"<<endl;
}
int main(){
         ThreadPool pool(8);

    while(1){
        pool.executetasks(func);
    }
    return 0;
}

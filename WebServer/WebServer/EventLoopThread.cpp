// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
    : loop_(NULL),
      exiting_(false),
      /*
      auto newCallable = bind(callable, arg_list);
      bind函数看做一个通用的函数适配器，它接受一个可调用对象callable，生成一个新的可调用对象newCallable。
　　  它可以把原可调用对象callable的某些参数预先绑定到给定的变量中（也叫参数绑定），然后产生一个新的可调用对象newCallable。
      网络编程中， 经常要使用到回调函数。 当底层的网络框架有数据过来时，往往通过回调函数来通知业务层。 
      这样可以使网络层只专注于数据的收发， 而不必关心业务
      在c语言中，回调函数的实现往往通过函数指针来实现。 但是在c++中，如果回调函数是一个类的成员函数。
      这时想把成员函数设置给一个回调函数指针往往是不行的
      因为类的成员函数，多了一个隐含的参数this。 所以直接赋值给函数指针肯定会引起编译报错。
      Thread thread_;
      thread_的构造函数要求传入一个函数引用和一个string
      */
      thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
      mutex_(),
      cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != NULL) {
    loop_->quit();
    thread_.join();
  }
}
//起线程之后就调用这个
EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();
  {
    MutexLockGuard lock(mutex_);
    // 一直等到threadFun在Thread里真正跑起来
    //loop_构造时初始化为NULL
    //wait的时候释放锁
    while (loop_ == NULL) cond_.wait();
  }
  return loop_;
}
//这是在线程中运行的函数
void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();
  // assert(exiting_);
  loop_ = NULL;
}
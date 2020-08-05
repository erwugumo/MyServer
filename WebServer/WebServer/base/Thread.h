// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <string>
#include "CountDownLatch.h"
#include "noncopyable.h"

class Thread : noncopyable {
 public:
  typedef std::function<void()> ThreadFunc;//这个是thread中跑的函数
  /*首先, C++中的explicit关键字只能用于修饰只有一个参数的类构造函数, 它的作用是表明该构造函数是显示的,
   而非隐式的, 跟它相对应的另一个关键字是implicit, 意思是隐藏的,类构造函数默认情况下即声明为implicit(隐式).
   explicit关键字的作用就是防止类构造函数的隐式自动转换.*/
  explicit Thread(const ThreadFunc&, const std::string& name = std::string());
  ~Thread();
  void start();
  int join();
  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }

 private:
  void setDefaultName();
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;
};
// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include "Util.h"
#include "base/Logging.h"

using namespace std;

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
  /*eventfd() 函数会创建一个 eventfd 对象，用户空间的应用程序可以用这个 eventfd 来实现事件的等待或通知机制，
  也可以用于内核通知新的事件到用户空间应用程序。 这个对象包含一个 64-bit 的整形计数器，内核空间维护这个计数器，
  创建这个计数器的时候使用第一个入参 initial_value 来初始化计数器。
  只要 flags 设置了 EFD_NONBLOCK，不管计数器初始值是什么值，读到计数器的值为 0 后，再继续读，会直接返回一个错误值，不会阻塞；
  除非有写入操作使计数器的值为非 0，才能续正常地进行读操作。
  只要 flags 没有设置 EFD_NONBLOCK，不管计数器初始值是什么值，读到计数器的值为 0 后，再继续读，会阻塞；
  除非有写入操作使计数器的值为非 0，才能续正常地进行读操作。
  如果 flags 设置为 0，并且子进程中往文件描述符写入 4、5、6 时，父进程读取并打印的结果是 15，这个时候计数器置为 0；
  如果 flags 设置 EFD_NONBLOCK，并且子进程中不往文件描述符写入（注释 write 代码），会打印一个错误，同时父进程退出（代码中执行 exit 函数）。
  */
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      poller_(new Epoll()),
      wakeupFd_(createEventfd()),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      pwakeupChannel_(new Channel(this, wakeupFd_)) {
  if (t_loopInThisThread) {
    // LOG << "Another EventLoop " << t_loopInThisThread << " exists in this
    // thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);//设置epoll的事件类型
  pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
  //bind就是为了将函数handleRead作为参数传入这个函数，handleRead有一个隐藏参数this，是当前Loop的指针，这样才能正常调用里面的数据成员
  pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
  poller_->epoll_add(pwakeupChannel_, 0);//把Channel加入到epoll，注册事件
}

void EventLoop::handleConn() {
  // poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET |
  // EPOLLONESHOT), 0);
  updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  // wakeupChannel_->disableAll();
  // wakeupChannel_->remove();
  close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
  if (n != sizeof one) {
    LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof one);//读一个数，64位，16字节
  if (n != sizeof one) {
    LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  // pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread())
    cb();
  else
    queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
  {
    MutexLockGuard lock(mutex_);
    //emplace_back也是向尾部添加元素
    pendingFunctors_.emplace_back(std::move(cb));//std::move并不能移动任何东西，它唯一的功能是将一个左值强制转化为右值引用，继而可以通过右值引用使用该值，以用于移动语义。
  }

  if (!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::loop() {
  assert(!looping_);
  assert(isInLoopThread());
  looping_ = true;
  quit_ = false;
  // LOG_TRACE << "EventLoop " << this << " start looping";
  std::vector<SP_Channel> ret;
  while (!quit_) {
    // cout << "doing" << endl;
    ret.clear();
    /*void clear()：删除存储在vector中的所有元素
    一、
　　1.如果vector的元素是一些object，则它将为当前存储的每个元素调用它们各自的析构函数。
　　2.如果vector存储的是指向对象的指针，此函数并不会调用到对应的析构函数。会造成内存泄漏。想要删除vector中的元素则应遍历vector使用delete，然后再clear
　　for(int i = 0; i < vec.size(); ++i)
　　{
　　　　delete vec[i];
　　}
　　vec.clear();
　　调用clear后，vector的size将变成0，但是它的容量capacity并未发生改变，clear只是删除数据，并未释放vector的内存
　　vector的clear不影响capacity
　　如果想要清空vector的元素，使用clear，如果想要释放vector的容量，可以使用swap
    二、使用swap释放vector的容量
　　vector<A>().swap(vec);
　　或者vec.swap(vector<A>());
　　重点：如果vector容器的元素是指针，先遍历容器，delete每个元素指向的内存，然后再用swap*/
    //得到所有发起连接的对应Channel，保存在ret里
    ret = poller_->poll();
    eventHandling_ = true;
    //对每一个连接代表的channel，调用handle进行处理
    //包括对新连接进行accept，对老连接进行处理
    for (auto& it : ret) it->handleEvents();
    eventHandling_ = false;
    //把函数数组中的函数一个个执行完
    doPendingFunctors();
    poller_->handleExpired();
  }
  looping_ = false;
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) functors[i]();
  callingPendingFunctors_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}
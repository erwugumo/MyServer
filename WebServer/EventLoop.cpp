#include "EventLoop.h"
#include "base/Logging.h"
#include "Util.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <iostream>
using namespace std;
/*
__thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。
__thread变量每一个线程有一份独立实体，各个线程的值互不干扰。
可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
__thread使用规则：只能修饰POD类型(类似整型指针的标量，不带自定义的构造、拷贝、赋值、析构的类型，二进制内容可以任意复制
memset,memcpy,且内容可以复原)，不能修饰class类型，因为无法自动调用构造函数和析构函数，可以用于修饰全局变量，
函数内的静态变量，不能修饰函数的局部变量或者class的普通成员变量，且__thread变量值只能初始化为编译器常量
(值在编译器就可以确定const int i=5,运行期常量是运行初始化后不再改变const int i=rand()).
如果希望修饰带自定义构造和析构函数的类，需要用到指针。
*/
__thread EventLoop* t_loopInThisThread = 0;

int createEventfd()
{
    /*
    eventfd 是 Linux 的一个系统调用，创建一个文件描述符用于事件通知，自 Linux 2.6.22 以后开始支持。
    eventfd() 创建一个 eventfd 对象，可以由用户空间应用程序实现事件等待/通知机制，或由内核通知用户空间应用程序事件。
    该对象包含了由内核维护的无符号64位整数计数器 count 。使用参数 initval 初始化此计数器。
    flags 可以是以下值的 OR 运算结果，用以改变 eventfd 的行为。

    EFD_CLOEXEC (since Linux 2.6.27)
    文件被设置成 O_CLOEXEC，创建子进程 (fork) 时不继承父进程的文件描述符。
    EFD_NONBLOCK (since Linux 2.6.27)
    文件被设置成 O_NONBLOCK，执行 read / write 操作时，不会阻塞。
    EFD_SEMAPHORE (since Linux 2.6.30)
    提供类似信号量语义的 read 操作，简单说就是计数值 count 递减 1。
    */
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG << "Failed in eventfd";
        /*
        abort()
        立即终止当前进程，产生异常程序终止
        进程终止时不会销毁任何对象
        */
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
:   looping_(false),
    poller_(new Epoll()),
    wakeupFd_(createEventfd()),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    pwakeupChannel_(new Channel(this, wakeupFd_))
    /*初始化列表好长*/
{
    if (t_loopInThisThread)
    {
        //LOG << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
        /*this 是 C++ 中的一个关键字，也是一个 const 指针，它指向当前对象，通过它可以访问当前对象的所有成员。*/
    }
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
    pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
    poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn()
{
    //poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET | EPOLLONESHOT), 0);
    updatePoller(pwakeupChannel_, 0);
}


EventLoop::~EventLoop()
{
    //wakeupChannel_->disableAll();
    //wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    if (n != sizeof one)
    {
        LOG<< "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_)
        wakeup();
}

void EventLoop::loop()
{
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;
    //LOG_TRACE << "EventLoop " << this << " start looping";
    std::vector<SP_Channel> ret;
    while (!quit_)
    {
        //cout << "doing" << endl;
        ret.clear();
        ret = poller_->poll();
        eventHandling_ = true;
        for (auto &it : ret)
            it->handleEvents();
        eventHandling_ = false;
        doPendingFunctors();
        poller_->handleExpired();
    }
    looping_ = false;
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    callingPendingFunctors_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}
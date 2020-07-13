#pragma once
//#pragma once 用于保证头文件只被编译一次
#include "base/Thread.h"
#include "Epoll.h"
#include "base/Logging.h"
#include "Channel.h"
#include "base/CurrentThread.h"
#include "Util.h"
#include <vector>
#include <memory>
#include <functional>

#include <iostream>
using namespace std;


class EventLoop
{
    /*
    private: 只能由该类中的函数、其友元函数访问,不能被任何其他访问，该类的对象也不能访问. 
    protected: 可以被该类中的函数、子类的函数、以及其友元函数访问,但不能被该类的对象访问
    public: 可以被该类中的函数、子类的函数、其友元函数访问,也可以由该类的对象访问
    注：友元函数包括两种：设为友元的全局函数，设为友元类中的成员函数
    */
public:
    typedef std::function<void()> Functor;
    /*
    类模版std::function是一种通用、多态的函数封装。
    std::function的实例可以对任何可以调用的目标实体进行存储、复制、和调用操作，
    这些目标实体包括普通函数、Lambda表达式、函数指针、以及其它函数对象等。
    std::function对象是对C++中现有的可调用实体的一种类型安全的包裹（我们知道像函数指针这类可调用实体，是类型不安全的）。
    通常std::function是一个函数对象类，它包装其它任意的函数对象，被包装的函数对象具有类型为T1, …,TN的N个参数，并且返回一个可转换到R类型的值。std::function使用 模板转换构造函数接收被包装的函数对象；特别是，闭包类型可以隐式地转换为std::function。

    最简单的理解就是：

　　　通过std::function对C++中各种可调用实体（普通函数、Lambda表达式、函数指针、以及其它函数对象等）的封装，形成一个新的可调用的std::function对象；让我们不再纠结那么多的可调用实体。一切变的简单粗暴。　
    */
    EventLoop();
    /*
    类的构造函数是类的一种特殊的成员函数，它会在每次创建类的新对象时执行。
    构造函数的名称与类的名称是完全相同的，并且不会返回任何类型，也不会返回 void。构造函数可用于为某些成员变量设置初始值。
    默认的构造函数没有任何参数，但如果需要，构造函数也可以带有参数。这样在创建对象时就会给对象赋初始值。
    与其他函数不同，构造函数除了有名字，参数列表和函数体之外，还可以有初始化列表，
    初始化列表以冒号开头，后跟一系列以逗号分隔的初始化字段。
    从概念上来讲，构造函数的执行可以分成两个阶段，初始化阶段和计算阶段，初始化阶段先于计算阶段.

    初始化阶段
    所有类类型（class type）的成员都会在初始化阶段初始化，即使该成员没有出现在构造函数的初始化列表中.

    计算阶段
    一般用于执行构造函数体内的赋值操作。

    使用初始化列表的原因
    初始化类的成员有两种方式，一是使用初始化列表，二是在构造函数体内进行赋值操作。
    主要是性能问题，对于内置类型，如int, float等，使用初始化类表和在构造函数体内初始化差别不是很大，但是对于类类型来说，
    最好使用初始化列表，为什么呢？使用初始化列表少了一次调用默认构造函数的过程，这对于数据密集型的类来说，是非常高效的。

    必须使用初始化列表的时候
    除了性能问题之外，有些时候初始化列表是不可或缺的，以下几种情况时必须使用初始化列表
    1.常量成员，因为常量只能初始化不能赋值，所以必须放在初始化列表里面
    2.引用类型，引用必须在定义的时候初始化，并且不能重新赋值，所以也要写在初始化列表里面
    3.没有默认构造函数的类类型，因为使用初始化列表可以不必调用默认构造函数来初始化，而是直接调用拷贝构造函数初始化
    */
    ~EventLoop();
    /*
    c++中函数前加~是表示此函数是析构函数。
    析构函数介绍
    1.析构函数(destructor) 与构造函数相反，当对象脱离其作用域时（例如对象所在的函数已调用完毕），系统自动执行析构函数。
    析构函数往往用来做“清理善后” 的工作（例如在建立对象时用new开辟了一片内存空间，应在退出前在析构函数中用delete释放）。
    2.以C++语言为例：析构函数名也应与类名相同，只是在函数名前面加一个位取反符~，例如~stud( )，以区别于构造函数。
    它不能带任何参数，也没有返回值（包括void类型）。只能有一个析构函数，不能重载。
    如果用户没有编写析构函数，编译系统会自动生成一个缺省的析构函数（即使自定义了析构函数，编译器也总是会为我们合成一个析构函数，
    并且如果自定义了析构函数，编译器在执行时会先调用自定义的析构函数再调用合成的析构函数），它也不进行任何操作。
    所以许多简单的类中没有用显示的析构函数。
    */
    void loop();
    void quit();
    /*
    我们无法在赋值函数里区分传入的是一个临时对象还是非临时对象，因此只能执行复制操作。为了解决这一问题，
    c++中引入了一个新的赋值函数的重载形式：
    container& operator = (container&& other)
    这个赋值函数通常称为移动赋值函数，和老版本的相比，它有两点区别：
    入参不是const型，因此它是可以更改入参的值的，从而实现交换操作
    入参前面有两个&号，这个是C++11引入的新语法，称为右值引用，它的使用方式和普通引用是一样的，唯一的区别是可以指向临时变量。
    现在，我们就有两个版本的赋值函数了，C++11在语法级别也做了适应：
    如果入参是临时变量，则执行移动赋值函数，如果没有定义移动赋值函数，则执行复制赋值函数（以保证老版本代码能编译通过）
    如果入参不是临时变量，则执行普通的复制赋值函数
    说到临时变量，我们大家也许都挺熟悉， 但是我自己对临时变量的理解却一直存在一个误区。
    通常情况下，我会把为了做某一件事情而临时创建的一个变量叫做临时变量。
    比如说在交换两个变量的值时，通常我们会创建第三个变量来达到我们最终的目的，而我们称之为“临时变量”。
    然而，大师Scott Meyers告诉我们，事实不是这么简单！
    在C/C++中，真正意义上的临时变量是看不见的，就是说它们不会出现在你的代码中，
    */
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void assertInLoopThread()
    {
        /*assert的作用是现计算表达式 expression ，如果其值为假（即为0），那么它先向stderr打印一条出错信息，
        然后通过调用 abort 来终止程序运行。*/
        assert(isInLoopThread());
    }
    void shutdown(shared_ptr<Channel> channel)
    {
        shutDownWR(channel->getFd());
    }
    void removeFromPoller(shared_ptr<Channel> channel)
    {
        //shutDownWR(channel->getFd());
        poller_->epoll_del(channel);
    }
    void updatePoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        poller_->epoll_mod(channel, timeout);
    }
    void addToPoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        poller_->epoll_add(channel, timeout);
    }
    
private:
    // 声明顺序 wakeupFd_ > pwakeupChannel_
    bool looping_;
    /*只要将 new 运算符返回的指针 p 交给一个 shared_ptr 对象“托管”，就不必担心在哪里写delete p语句——实际上根本不需要编写这条语句，
    托管 p 的 shared_ptr 对象在消亡时会自动执行delete p。而且，该 shared_ptr 对象能像指针 p —样使用，
    即假设托管 p 的 shared_ptr 对象叫作 ptr，那么 *ptr 就是 p 指向的对象。*/
    shared_ptr<Epoll> poller_;
    int wakeupFd_;
    bool quit_;
    bool eventHandling_;
    /*mutable的中文意思是“可变的，易变的”，跟constant（既C++中的const）是反义词。
　　在C++中，mutable也是为了突破const的限制而设置的。被mutable修饰的变量，将永远处于可变的状态，即使在一个const函数中。
　　我们知道，如果类的成员函数不会改变对象的状态，那么这个成员函数一般会声明成const的。
    但是，有些时候，我们需要在const的函数里面修改一些跟类状态无关的数据成员，那么这个数据成员就应该被mutalbe来修饰。*/
    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_;
    const pid_t threadId_; 
    shared_ptr<Channel> pwakeupChannel_;
    
    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};

RAII:
什么是RAII？

RAII是Resource Acquisition Is Initialization（wiki上面翻译成 “资源获取就是初始化”）的简称，是C++语言的一种管理资源、避免泄漏的惯用法。利用的就是C++构造的对象最终会被销毁的原则。RAII的做法是使用一个对象，在其构造时获取对应的资源，在对象生命期内控制对资源的访问，使之始终保持有效，最后在对象析构的时候，释放构造时获取的资源。

为什么要使用RAII？

上面说到RAII是用来管理资源、避免资源泄漏的方法。那么，用了这么久了，也写了这么多程序了，口头上经常会说资源，那么资源是如何定义的？在计算机系统中，资源是数量有限且对系统正常运行具有一定作用的元素。比如：网络套接字、互斥锁、文件句柄和内存等等，它们属于系统资源。由于系统的资源是有限的，就好比自然界的石油，铁矿一样，不是取之不尽，用之不竭的，所以，我们在编程使用系统资源时，都必须遵循一个步骤：
1 申请资源；
2 使用资源；
3 释放资源。
第一步和第三步缺一不可，因为资源必须要申请才能使用的，使用完成以后，必须要释放，如果不释放的话，就会造成资源泄漏。

一个最简单的例子：
```cpp
#include <iostream> 
using namespace std; 
int main() 
{ 
    int *testArray = new int [10]; 
    // Here, you can use the array 
    delete [] testArray; 
    testArray = NULL ; 
    return 0; 
}
```

我们在用动态内存分配时，经常是用new来定义一块内存空间，比如说 int* p = new int(1)；这时会在堆上分配一块内存，当作int类型使用，内存中存储的值为1并将内存地址赋值给在栈中的int*类型的p。（注意：p只是一个变量，就像是int a=1中的a一样，不过a是整形变量，而p是指针变量）当我们不用p指针时，往往需要用delete p将其释放，我们需要注意的是释放一个指针p（delete p;）实际意思是删除了p所指的目标（变量或对象），释放了它所占的堆空间，而不是删除p本身（指针p本身并没有撤销，它自己仍然存在，该指针所占内存空间并未释放，指针p的真正释放是随着函数调用的结束而消失），释放堆空间后，p成了"空指针"。如果我们在delete p后没有进行指针p的制空（p=NULL)的话，其实指针p这时会成为野指针，为了使用的安全，我们一般在delete p之后还会加上p=NULL这一语句。

小结：
但是如果程序很复杂的时候，需要为所有的new 分配的内存delete掉，导致极度臃肿，效率下降，更可怕的是，程序的可理解性和可维护性明显降低了，当操作增多时，处理资源释放的代码就会越来越多，越来越乱。如果某一个操作发生了异常而导致释放资源的语句没有被调用，怎么办？这个时候，RAII机制就可以派上用场了。

如何使用RAII?

当我们在一个函数内部使用局部变量，当退出了这个局部变量的作用域时，这个变量也就被销毁了；当这个变量是类对象时，这个时候，就会自动调用这个类的析构函数，而这一切都是自动发生的，不要程序员显示的去调用完成。这个也太好了，RAII就是这样去完成的。

由于系统的资源不具有自动释放的功能，而C++中的类具有自动调用析构函数的功能。如果把资源用类进行封装起来，对资源操作都封装在类的内部，在析构函数中进行释放资源。当定义的局部变量的生命结束时，它的析构函数就会自动的被调用，如此，就不用程序员显示的去调用释放资源的操作了。

使用RAII 机制的代码：
```cpp
#include <iostream> 
using namespace std; 
class ArrayOperation 
{ 
public : 
    ArrayOperation() 
    { 
        m_Array = new int [10]; 
    }  
    void InitArray() 
    { 
        for (int i = 0; i < 10; ++i) 
        { 
            *(m_Array + i) = i; 
        } 
    }  
    void ShowArray() 
    { 
        for (int i = 0; i <10; ++i) 
        { 
            cout<<m_Array[i]<<endl; 
        } 
    } 
    ~ArrayOperation() 
    { 
        cout<< "~ArrayOperation is called" <<endl; 
        if (m_Array != NULL ) 
        { 
            delete[] m_Array;  
            m_Array = NULL ; 
        } 
    } 
private : 
    int *m_Array; 
};  
bool OperationA(); 
bool OperationB(); 
int main() 
{ 
    ArrayOperation arrayOp; 
    arrayOp.InitArray(); 
    arrayOp.ShowArray(); 
    return 0;
}
```
在使用多线程时，经常会涉及到共享数据的问题，C++中通过实例化std::mutex创建互斥量，通过调用成员函数lock()进行上锁，unlock()进行解锁。不过这意味着必须记住在每个函数出口都要去调用unlock()，也包括异常的情况，这非常麻烦，而且不易管理。C++标准库为互斥量提供了一个RAII语法的模板类std::lock_guard，其会在构造函数的时候提供已锁的互斥量，并在析构的时候进行解锁，从而保证了一个已锁的互斥量总是会被正确的解锁。上面的代码正式\<mutex>>头文件中的源码，其中还使用到很多C++11的特性，比如delete/noexcept等，有兴趣的同学可以查一下。

线程同步四项原则：
尽量最低限度的共享对象，减少需要同步的场合：一个对象能不暴露给别的线程就不要暴露；如果要暴露，优先考虑immutable对象；实在不行再暴露可修改的对象，使用同步措施来充分保护它。
使用高级的并发编程构建：TaskQueue、Producer-Consumer Queue、CountDownLatch等。
必须使用底层同步原语时，使用非递归的互斥器和条件变量，慎用读写锁，不用信号量。
除了使用atomic整数之外，不自己编写lock-free代码，不用内核级同步原语。

互斥器：
使用最多的同步原语，保护临界区，保证任意时刻最多只能有一个线程在此mutex划出的临界区活动。单独使用mutex主要为了保护共享数据。

使用RAII手法封装mutex的创建、销毁、加锁、解锁。保证锁的生效期间等于一个作用域。

只使用非递归的mutex。

不手工调用lock和unlock函数，一切交给栈上的Guard对象的构造和析构函数负责。Guard对象的生命期正好等于临界区。避免在foo里加锁，然后跑到bar里解锁。避免在不同的分支分别加锁解锁。

每次构造guard对象的时候，思考调用栈上已经持有的锁，防止因加锁顺序不同而导致死锁。

只使用非递归的mutex：

mutex分递归和非递归两种，也叫可重入和非可重入。它们作为线程间的同步工具时没有区别，唯一区别在于：同一个进程可以重复对递归的mutex加锁，但对非递归的mutex不能。在同一个进程里再次对非递归的mutex加锁将导致死锁。

使用递归的mutex可能会导致一些问题：
线程A调用函数a，先获取锁，然后改变对象foo；调用函数b，先获取锁，然后遍历vector\<foo>，遍历时调用a；这时，非递归的mutex由于两次调用而死锁，递归的mutex则不会。但是，若在遍历vector时执行了push_back，则可能导致迭代器失效，程序crash。

举这个例子是为了表示出非递归mutex可以一定程度上暴露出程序的逻辑错误：遍历vector的时候对vector进行了修改——对于非递归mutex，直接死锁，可以立即发现错误；对于非递归则只能在crash时候发现错误。

那怎么改呢？可以把修改推后，先记住哪些要修改，退出循环后再修改；或者copy-on-write。就是发现要修改，那么复制一个vector，在复制后的vector上修改，改完了用复制后的代替之前的。

如果一个函数既可能在加锁的情况下调用，又可能在不加锁的情况下调用，那么就拆成两个函数。

死锁：一般出现在多个mutex请求顺序不当的时候。
比如，进程A调用函数a，请求mutex1，然后遍历vector1，对每个vector1中的元素，请求元素中的mutex2，然后处理；此时又有进程B将vector中的元素进行销毁，销毁首先请求mutex2，然后请求mutex1将vector中的元素删除。这时进程A和进程B分别获得了mutex1和mutex2，造成死锁。

怎么解决呢？要不就准备一个vector的副本，printf处理，要不就用shared_ptr；但是都没有解决vector中的元素析构的话产生的竞态条件。

解决析构不但需要用shared_ptr管理vector，还需要用shared_ptr管理vector中的资源。

条件变量：

一个或多个线程等待某个布尔值为真，等待别的线程唤醒该线程，这里等待的就是条件变量，学名为管程。

条件变量只有一种正确的使用方式，在wait端：
必须与mutex一起使用，该表达式的读写需要受到mutex保护。
在mutex已经上锁的时候才能调用wait。
把判断布尔条件和wait()放到while循环中。
eg.
```cpp
muduo::MutexLock mutex;
muduo::Condition cond(mutex);
std::deque<int> queue;//双端队列

int dequeue()
{
    MutexLockGuard lock(mutex);
    while(queue.empty())//必须是while，必须是判断之后再wait，布尔值，或者说条件，condition
    {
        cond.wait();//wait函数包括解锁unlock和wait，是原子操作
        //wait执行后返回时继续加锁，wait叫条件变量
    }
    //lock
    //while 防止虚假唤醒
    //unlock+wait 防止错过signal
    //waitreturn+lock 对应第一个lock
    //run
    assert(!queue.empty());
    int top=queue.front();
    queue.pop_front();
    return top;
}
```

这里必须使用while而不能用if。原因是spurious wakeup，即虚假唤醒。
虚假唤醒的定义是“This means that when you wait on a condition variable,the wait may (occasionally) return when no thread  pecifically broadcast or signaled that condition variable.”，即即使没有线程broadcast 或者signal条件变量，wait也可能偶尔返回。

也就是说，使用while而不是if，是为了处理发生虚假唤醒之后不要有错误的行为，而不是避免虚假唤醒的。

为什么wait前要解锁？
不解锁的话其他线程得不到锁，没法修改队列。
为什么是原子操作？
若不是原子操作，先解锁；然后其他进程发给该进程一个结束等待的信号，但此时wait还没开始，信号被错过，之后再一直等待。先解锁再wait成为原子操作可以防止任何信号的错过。
真正的wait函数是夹在俩互斥量之间的。

对于signal/broadcast端：
不一定要在mutex已上锁的情况下调用signal(理论上)。
在signal之前一般要修改布尔表达式
修改布尔表达式一般要mutex保护
注意区分signal和broadcast：signal一般表示资源可用，看做单播；broadcast一般表示状态变化，广播。

```cpp
void enqueue(int x)
{
    MutexLockGuard lock(mutex);
    queue.push_back(x);
    cond.notify();//可以移出临界区
}
```

若wait返回和加锁不是原子操作，那存在这样一种情况：线程A在wait，线程B发broadcast，此时线程A应加锁然后操作，但是这个时候线程B的锁用完了被线程C抢走，线程C将资源夺走，队列变为空，之后释放锁，这时线程A抢回锁，但是队列为空，又得继续执行while，等待。这就是一次虚假唤醒。

若notify的条件改为从0到1才notify而不是每pushback才notify，则会出现以下情况：
考虑下面的执行顺序: //queue 里有 0 个元素 t1 deq ---> wait t2 deq ---> wait t4 enq ----> queue 0 -> 1 signal //唤醒 t1, 但 t1 没有立刻执行,而是 t4 继续 enq t4 enq ----> queue 1 -> 2 no signal t1 继续deq ----> queue 2 -> 1 t2 still waits //unexpected behavior! // queue 1 容器里有东西,但 t2 还在 wait 。正确的行为本应该是t2被唤醒继续 deq 的。

简言之，如果enqueue线程执行过快，如果有一个dequeue线程没有来得及唤醒（其他dequeue线程抢先了），那么它将一直被阻塞（饿死），直到queue的size再次重复0到1的过程

为什么条件变量要配合mutex使用？
因为条件变量一定有着判断某个布尔值的操作。这个布尔值是全局变量，其他进程可见。，所以在查看该布尔值的时候要锁住，防止查看完进wait前布尔值变了导致不应该进wait的发生。

总而言之，为了避免因条件判断语句与其后的正文或wait语句之间的间隙而产生的漏判或误判，所以用一个mutex来保证: 对于某个cond的包括(判断,修改)在内的任何有关操作某一时刻只有一个线程在访问。也就是说条件变量本身就是一个竞争资源，这个资源的作用是对其后程序正文的执行权，于是用一个锁来保护。这样就关闭了条件检查和线程进入休眠状态等待条件改变这两个操作之间的时间通道，这样线程就不会有任何变化。

条件变量是非常低级的同步原语，很少直接使用，一般都是用它来实现高层的同步措施，如BlockingQueue或者CountDownLatch(倒计时)。

倒计时很常用：
主线程发起多个子线程，直到所有子线程全结束，主线程继续进行，多用于初始化
主线程发起多个子线程，子线程都在等主线程，主线程完成其他任务通知所有子线程一起执行，类似起跑命令

```cpp
class CountDownLatch:boost::noncopyable
{
    public:
    explicit CountDownLatch(int count);//倒数几次
    void wait();
    void countDown();

    private:
    mutable MytexLock mutex_;
    Condition condition_;
    int count_;
}

void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while(count_>0)
        condition_.wait();
}
void CountDownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    --count;
    if(count_==0)
    condition_.notifyALL();
}
```
主线程用wait阻塞，子线程每个都用countDown，可以实现第一种情况；
子线程用wait阻塞，主线程用countDown，可以实现第二种情况。

MutexLock的实现
```cpp
class MutexLock:boost::noncopyable
{
    public:
    MutexLock():holder_(0){
        pthread_mutex_init(&mutex_,NULL);
    }
    ~MutexLock()
    {
        assert(holder_==0);
        pthread_mutex_destory(&mutex);
    }
    bool isLockedByThisThread()
    {
        return holder_==CurrentThread::tid();
    }
    void assertLocked()
    {
        assert(isLockedByThisThread());
    }
    void lock()//仅供MutexLockGuard调用，严禁用户代码调用
    {
        pthread_mutex_lock(&mutex);
        holder_=CurrentThread::tid();//这两行不能反
    }
    void unlock()//仅供MutexLockGuard调用，严禁用户代码调用
    {
        holder_=0；
        pthread_mutex_unlocked(&mutex_);//这两行不能反，保证holder_的改变在mutex锁内
    }
    pthread_mutex_t* getPthreadMutex()
    {
        return &mutex_;
    }
    private:
    pthread_mutex_t mutex_;
    pid_t holder_;
}

class MutexLockGuard:boost::noncopyable
{
    public:
    explicit MutexLockGuard(MutexLock& mutex):mutex_(mutex)
    {
        mutex_.lock();
    }
    ~MutexLockGuard()
    {
        mutex_.unlock();
    }
    private:
    MutexLock& mutex_;
}
```

条件变量：
```cpp
class Condition:boost::noncopyable
{
    public:
    explicit Condition(MutexLock& mutex):mutex_(mutex)
    {
        pthread_cond_init(&pcond_,NULL);
    }
    ~Condition(){ pthread_cond_destroy(&pcond_); }
    void wait() { pthread_cond_wait(&pcond_,mutex_.getPthreadMutex()); }
    void notify() { pthread_cond_signal(&pcond_); }
    void notifyAll() { pthread_cond_broadcast(&pcond_) };

    private:
    MutexLock& mutex_;
    pthread_cond_t pcond_;3
}
```

注意初始化顺序要和声明顺序保持一致。

线程安全的Singleton实现

Singleton，就是单例模式。
单例 Singleton 是设计模式的一种，其特点是只提供唯一一个类的实例,具有全局变量的特点，在任何位置都可以通过接口获取到那个唯一实例;
具体运用场景如：

设备管理器，系统中可能有多个设备，但是只有一个设备管理器，用于管理设备驱动;
数据池，用来缓存数据的数据结构，需要在一处写，多处读取或者多处写，多处读取;

C++单例的实现

2.1 基础要点
全局只有一个实例：static 特性，同时禁止用户自己声明并定义实例（把构造函数设为 private）
线程安全
禁止赋值和拷贝
用户通过接口获取实例：使用 static 类成员函数

2.2 C++ 实现单例的几种方式
2.2.1 有缺陷的懒汉式
懒汉式(Lazy-Initialization)的方法是直到使用时才实例化对象，也就说直到调用get_instance() 方法的时候才 new 一个单例的对象。好处是如果被调用就不会占用内存。
```cpp
#include <iostream>
// version1:
// with problems below:
// 1. thread is not safe
// 2. memory leak

class Singleton{
private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
    Singleton(Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Singleton* m_instance_ptr;
public:
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    static Singleton* get_instance(){
        if(m_instance_ptr==nullptr){
              m_instance_ptr = new Singleton;
        }
        return m_instance_ptr;
    }
    void use() const { std::cout << "in use" << std::endl; }
};

Singleton* Singleton::m_instance_ptr = nullptr;

int main(){
    Singleton* instance = Singleton::get_instance();
    Singleton* instance_2 = Singleton::get_instance();
    return 0;
}
```

可以看到，获取了两次类的实例，却只有一次类的构造函数被调用，表明只生成了唯一实例，这是个最基础版本的单例实现，他有哪些问题呢？

线程安全的问题,当多线程获取单例时有可能引发竞态条件：第一个线程在if中判断 m_instance_ptr是空的，于是开始实例化单例;同时第2个线程也尝试获取单例，这个时候判断m_instance_ptr还是空的，于是也开始实例化单例;这样就会实例化出两个对象,这就是线程安全问题的由来; 解决办法:加锁

内存泄漏. 注意到类中只负责new出对象，却没有负责delete对象，因此只有构造函数被调用，析构函数却没有被调用;因此会导致内存泄漏。解决办法： 使用共享指针;

线程安全、内存安全的懒汉式单例 （智能指针，锁）
```cpp
#include <iostream>
#include <memory> // shared_ptr
#include <mutex>  // mutex

// version 2:
// with problems below fixed:
// 1. thread is safe now
// 2. memory doesn't leak

class Singleton{
public:
    typedef std::shared_ptr<Singleton> Ptr;
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    Singleton(Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Ptr get_instance(){

        // "double checked lock"
        if(m_instance_ptr==nullptr){
            std::lock_guard<std::mutex> lk(m_mutex);
            if(m_instance_ptr == nullptr){
              m_instance_ptr = std::shared_ptr<Singleton>(new Singleton);
            }
        }
        return m_instance_ptr;
    }


private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
    static Ptr m_instance_ptr;
    static std::mutex m_mutex;
};

// initialization static variables out of class
Singleton::Ptr Singleton::m_instance_ptr = nullptr;
std::mutex Singleton::m_mutex;

int main(){
    Singleton::Ptr instance = Singleton::get_instance();
    Singleton::Ptr instance2 = Singleton::get_instance();
    return 0;
}
```
shared_ptr和mutex都是C++11的标准，以上这种方法的优点是

基于 shared_ptr, 用了C++比较倡导的 RAII思想，用对象管理资源,当 shared_ptr 析构的时候，new 出来的对象也会被 delete掉。以此避免内存泄漏。
加了锁，使用互斥量来达到线程安全。这里使用了两个 if判断语句的技术称为双检锁；好处是，只有判断指针为空的时候才加锁，避免每次调用 get_instance的方法都加锁，锁的开销毕竟还是有点大的。

不足之处在于： 使用智能指针会要求用户也得使用智能指针，非必要不应该提出这种约束; 使用锁也有开销; 同时代码量也增多了，实现上我们希望越简单越好。

还有更加严重的问题，在某些平台（与编译器和指令集架构有关），双检锁会失效！具体可以看这篇文章，解释了为什么会发生这样的事情。

因此这里还有第三种的基于 Magic Staic的方法达到线程安全
```cpp
#include <iostream>

class Singleton
{
public:
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    Singleton(const Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Singleton& get_instance(){
        static Singleton instance;
        return instance;

    }
private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
};

int main(int argc, char *argv[])
{
    Singleton& instance_1 = Singleton::get_instance();
    Singleton& instance_2 = Singleton::get_instance();
    return 0;
}
```

所用到的特性是在C++11标准中的Magic Static特性：

If control enters the declaration concurrently while the variable is being initialized, the concurrent execution shall wait for completion of the initialization.
如果当变量在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待初始化结束。

这样保证了并发线程在获取静态局部变量的时候一定是初始化过的，所以具有线程安全性。

C++静态变量的生存期 是从声明到程序结束，这也是一种懒汉式。

这是最推荐的一种单例实现方式：

通过局部静态变量的特性保证了线程安全 (C++11, GCC > 4.3, VS2015支持该特性);
不需要使用共享指针，代码简洁；
注意在使用的时候需要声明单例的引用 Single& 才能获取对象。

返回对象和返回引用的最主要的区别就是函数原型和函数头。

Car run(const Car &) //返回对象

Car & run(const Car &) //返回引用

第二种方法是在主程序中调用构造函数，堆上构造一个变量；第三种方法是在主程序中调用get_instance，这个函数中再构造一个变量，返回引用。前者变量的作用域是整个主函数，后者则是get_instance，但是返回引用所以可见。

用mutex替换读写锁。

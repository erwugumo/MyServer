多线程与IO

多线程处理多socket通常可以提高效率，多线程同时处理一个socket呢？

操作文件描述符的系统调用本身是线程安全的，不用担心多个线程同时操作文件描述符会造成进程崩溃或内核崩溃。

socket读写的特点是不保证完整性，读100字节可能只返回20字节，写操作也一样

多线程程序应该遵循的原则是：每个文件描述符只由一个线程操作。一个线程可以操作多个文件描述符，但一个线程不能操作别的线程拥有的文件描述符

考虑如下一种情况：socket对应的文件描述符是3，线程A正在read3，线程B同时关闭了3，线程C又打开了一个新socket，fd是3，那么有可能线程A读取的是线程C打开的socket中的内容，完全错误。TCP连接的数据只能读一次，这造成了无法挽回的错误。

可以加锁，但会极大降低效率。

C++中使用RAII来解决这个问题。用socket对象来包装fd，所有对fd的读写通过对象进行。在析构函数中关闭文件描述符。这样就不是直接读取fd，避免了上面的情况。

RAII与fork

如果用了fork，RAII就很难，很容易出现主程序声明一个对象，fork后在主进程和子进程中析构两次的情况。

子进程不会继承父进程的内存锁、文件锁、定时器等。加上多线程与fork的协作很差，导致fork一般不在多线程中调用。

多线程与signal

在多线程中不要使用signal和基于signal的定时函数。不主动处理各种异常信号，只使用默认语义，结束进程。唯一的例外是SIGPIPE，选择忽略此信号。在没有其他替代方式的情况下，采用signalfd把信号转换为文件描述符事件。

高效的多线程日志(logging)

这里指的是诊断日志，通常用于故障诊断和追踪。

对于关键进程，日志要记录：
收到的每条内部消息的id
收到的每条外部消息的全文
发出的每条消息的全文，每条消息有全局唯一的id
关键内部状态的变更，等等

多线程写日志要异步处理

使用两个缓冲区，前端往A缓冲区中写入，后端从B缓冲区中读取，写满了就交换，没写满每隔3秒也交换1

代码实现：
实际上使用了四个缓冲区
```cpp
typedef boost::ptr_vector<LargeBuffer> BufferVector;
/*
上面也可以用vector<shared_ptr<LargeBuffer> >,但还是用ptr_vector好一点。因为：
第一，反复声明 boost::shared_ptr 需要更多的输入。 
其次，将 boost::shared_ptr拷进，拷出，或者在容器内部做拷贝，需要频繁的增加或者减少内部引用计数，这肯定效率不高。
由于这些原因，Boost C++ 库提供了指针容器专门用来管理动态分配的对象。
*/
typedef BufferVector::auto_type BufferPtr;
muduo::MutexLock mutex_;
muduo::Condition cond_;
BufferPtr currentBuffer_;//当前缓冲
BufferPtr nextBuffer_;//预备缓冲
BufferVector buffers_;//待写入文件的已写满的缓冲
//前端写入
void AsyncLogging::append(const char* logline,int len)
{
    //注意，每写一条日志都要请求一次锁
    muduo::MutexLockGuard lock(mutex_);
    if(currentBuffer_->avail()>len)//当前缓冲区没满，直接写
        currentBuffer_->append(logline,len);
    else
    {
        buffers_.push_back(currentBuffer_.release());//当前缓冲区放入后台队列
        if(nextBuffer_)//预备缓冲变为当前缓冲
        {
            currentBuffer_=boost::ptr_container::move(nextBuffer_);
        }
        else//预备缓冲也用完了
        {
            currentBuffer_.reset(new LargeBuffer);
            //带参数的reset使得原指针引用计数减1的同时改为管理另一个指针。
            //这句话这么理解，currentBuffer_是一个智能指针，指向一块内存，有许多智能指针指向这块内存
            //reset之后，currentBuffer_就指向新的对象，原有的那些智能指针的引用计数减一
        }
        currentBuffer_->append(logline,len);
        cond_.notify();
    }
}
//后端写入
void AsyncLogging::threadFunc()
{
    //在后端操作时是不可写入的
    BufferPtr newBuffer1(new LargeBuffer);
    BufferPtr newBuffer2(new LargeBuffer);
    BufferVector buffersToWrite;
    while(running_)
    {
        {
            muduo::MutexLockGuard lock(mutex_);
            if(buffers_.empty())
            {
                cond_.waitForSeconds(flushInterval_);//注意这里，前端随时有可能向buffers_中push元素，因此等待条件要是超时或者有元素
            }
            buffers_.push_back(currentBuffer_.release());
            currentBuffer_=boost::ptr_container::move(newBuffer1);//新的作为当前的
            buffersToWrite.swap(buffers_);//交换两个队列，这样可以在临界区外读取队列
            if(!nextBuffer_)
            {
                nextBuffer_=boost::ptr_container::move(newBuffer2);
            }
        }
    }
}
```
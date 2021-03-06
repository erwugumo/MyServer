多线程服务器的适用场合与常用编程模型

进程：可以共享代码段但不能共享数据，fork

线程：特点是共享存储空间，pthread_create

在linux系统中，posix线程可以“看做”为一种轻量级的进程，pthread_create创建线程和fork创建进程都是在内核中调用__clone函数创建的，只不过创建线程或进程的时候选项不同，比如是否共享虚拟地址空间、文件描述符等。

我们知道通过fork创建的一个子进程几乎但不完全与父进程相同。子进程得到与父进程用户级虚拟地址空间相同的（但是独立的）一份拷贝，包括文本、数据和bss段、堆以及用户栈等。子进程还获得与父进程任何打开文件描述符相同的拷贝，这就意味着子进程可以读写父进程中任何打开的文件，父进程和子进程之间最大的区别在于它们有着不同的PID。

但是有一点需要注意的是，在Linux中，fork的时候只复制当前线程到子进程，在fork(2)-Linux Man Page中有着这样一段相关的描述：

The child process is created with a single thread--the one that called fork(). The entire virtual address space of the parent is replicated in the child, including the states of mutexes, condition variables, and other pthreads objects; the use of pthread_atfork(3) may be helpful for dealing with problems that this can cause.

也就是说除了调用fork的线程外，其他线程在子进程中“蒸发”了。

这就是多线程中fork所带来的一切问题的根源所在了。

互斥锁，就是多线程fork大部分问题的关键部分。

在大多数操作系统上，为了性能的因素，锁基本上都是实现在用户态的而非内核态（因为在用户态实现最方便，基本上就是通过原子操作或者之前文章中提到的memory barrier实现的），所以调用fork的时候，会复制父进程的所有锁到子进程中。

memory barrier：内存屏障，大概意思就是，虽然你的代码是这么一行一行写下去，但是不管是编译器，还是CPU，都会按照自己的意思，看着高兴用不同的顺序来执行你的代码。所以有可能写在前面的代码被放在后面执行。因此你需要通过一系列的barrier和其他标记，来告诉它们，哪些移动是不行的，什么时候要刷cache，什么时候要同步。

问题就出在这了。从操作系统的角度上看，对于每一个锁都有它的持有者，即对它进行lock操作的线程。假设在fork之前，一个线程对某个锁进行的lock操作，即持有了该锁，然后另外一个线程调用了fork创建子进程。可是在子进程中持有那个锁的线程却"消失"了，从子进程的角度来看，这个锁被“永久”的上锁了，因为它的持有者“蒸发”了。

那么如果子进程中的任何一个线程对这个已经被持有的锁进行lock操作话，就会发生死锁。

当然了有人会说可以在fork之前，让准备调用fork的线程获取所有的锁，然后再在fork出的子进程的中释放每一个锁。先不说现实中的业务逻辑以及其他因素允不允许这样做，这种做法会带来一个问题，那就是隐含了一种上锁的先后顺序，如果次序和平时不同，就会发生死锁。

如果你说自己一定可以按正确的顺序上锁而不出错的话，还有一个隐含的问题是你所不能控制的，那就是库函数。

因为你不能确定你所用到的所有库函数都不会使用共享数据，即他们都是完全线程安全的。有相当一部分线程安全的库函数都是在内部通过持有互斥锁的方式来实现的，比如几乎所有程序都会用到的C/C++标准库函数malloc、printf等等。

比如一个多线程程序在fork之前难免会分配动态内存，这就必然会用到malloc函数；而在fork之后的子进程中也难免要分配动态内存，这也同样要用到malloc，可这却是不安全的，因为有可能malloc内部的锁已经在fork之前被某一个线程所持有了，而那个线程却在子进程中消失了。

11个最基本的Pthread函数：
2个：线程的创建和等待结束(join)。
4个：mutex的创建、销毁、加锁、解锁。
5个：条件变量的创建、销毁、等待、通知、广播。

线程安全是不可组合的。一个函数调用两个线程安全的函数，这个函数可能不是线程安全的。

因此STL的标准库容器都不是线程安全的，因为做了也没用。

凡是非共享的对象都是彼此独立的，如果一个对象从始至终只被一个线程用到，那么它就是安全的。
对共享对象的read-only操作是线程安全的，前提是没有并发的写操作。

C++的iostream不是线程安全的，
cout<<"123"<<"456"<<endl;
在两次<<之间可能会向cout输出其他字符。
要想用线程安全的stdout，可以用printf，但这等价于用了全局锁，任何时刻只能有一个线程调用printf。

linux上的线程标识：
POSIX threads库提供了pthread_self函数来返回当前线程的标识符，其类型为pthread_t。这个pthread_t类型不确定，可能是整数、指针或结构体。因此要比较俩pthread_t是否相等，要用pthread_equal函数。
由于pthread_t类型不确定，因此不能打印它；无法比较大小，无法作为关联容器的key；无法定义非法值；只在进程内有定义，在/proc中找不到对应的task。
pthread_t这个标识符不好用。

最好使用gettid系统调用。注意，只能在linux上用，不能在其他系统上用。gettid返回值的类型是pid_t，通常是一个小整数，最大值默认是32768；
在现代linux中，它直接表示内核的任务调度id，在/proc中可以轻易找到对应项；在top中可以找到；任何时刻全局唯一；0是非法值。

高效使用gettid最好使用缓存而不是频繁使用系统调用，状态频繁切换浪费资源。仅在本线程第一次调用gettid的时候才进行系统调用，以后都是直接从thread local缓存的线程id拿到结果。

线程的创建和销毁的守则：

线程的创建：
程序库不应该在未提前告知的情况下创建自己的背景线程

比如库私自创建了一个线程，fork就不安全了

尽量使用相同的方式创建线程

程序中的线程用同一个class创建，那么在线程启动和销毁的时候就容易记录

在进入main之前不要启动线程

这会影响全局对象(namespace级全局对象、文件级全局对象、class的静态对象、函数内的静态对象)的构造。

线程的创建最好在初始化阶段全部完成

最好在程序的初始化阶段创建全部工作线程，在程序运行期间不再创建或销毁线程。当有新的连接时，分配给线程池中的已有线程而不是新建线程。

线程的销毁：
自然死亡：从线程的主函数返回，线程正常退出
非正常死亡：从线程主函数抛出异常或线程触发segfault信号等非法操作
自杀：在县城中调用pthread_exit()来立即退出线程
他杀：其他线程调用pthread_cancel()来强制终止某个线程

只有自然死亡是线程正常退出的方式

无论是自杀还是他杀，线程都没有机会清理资源，也没有机会释放已经持有的锁。这样，若其他线程申请相同的锁，会立即死锁

若确实需要终止一个耗时很长的计算任务，又不想周期性的检查某个全局退出标志，可以考虑把那一部分代码fork为新的进程，杀一个进程比杀本进程内的线程安全得多。

exit()不是线程安全的。例如一个全局对象的某个函数持有了一把锁，随后执行exit，那么exit会析构全局对象和函数内的静态对象，析构函数中再次请求这把锁，于是产生死锁。

善用__thread关键字

__thread是GCC内置的线程局部存储设施。只能用于修饰POD类型，不能修饰class类型，因为无法自动调用构造函数和析构函数。

POD类型：POD，是Plain Old Data的缩写，普通旧数据类型，是C++中的一种数据类型概念

POD类型与C编程语言中使用的类型兼容，POD数据类型可以使用C库函数进行操作，也可以使用std::malloc创建，可以使用std::memmove等进行复制，并且可以使用C语言库直接进行二进制形式的数据交换

__thread可以用于修饰全局变量、函数内的静态变量，但不能用于修饰函数的局部变量或class的普通成员变量。

__thread变量的初始化只能用于编译期常量。

__thread变量在每个线程都有一份独立实体，各个线程的变量值互不干扰。它可以修饰那些“值可能会变，带有全局性，但又不值得用全局锁保护的变量”



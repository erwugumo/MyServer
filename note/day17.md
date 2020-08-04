pthread

#### 1. 第一个例子
在Linux编写多线程程序需要包含头文件
＃include <pthread.h> 
当然，进包含一个头文件是不能搞定线程的，还需要连接libpthread.so这个库，因此在程序链接阶段应该有类似 
gcc -o pthread_1 pthread_1.cpp -lpthread
在Linux下创建的线程的API接口是pthread_create()，它的完整定义是：
```cpp
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*)，void *arg);
```
当你的程序调用了这个接口之后，就会产生一个线程，而这个线程的入口函数就是start_routine()。如果线程创建成功，这个接口会返回0。
start_routine()函数有一个参数，这个参数就是pthread_create的最后一个参数arg。这种设计可以在线程创建之前就帮它准备好一些专有数据，最典型的用法就是使用C++编程时的this指针。start_routine()有一个返回值，这个返回值可以通过pthread_join()接口获得。
pthread_create()接口的第一个参数是一个返回参数。当一个新的线程调用成功之后，就会通过这个参数将线程的句柄返回给调用者，以便对这个线程进行管理。
pthread_create()接口的第二个参数用于设置线程的属性。这个参数是可选的，当不需要修改线程的默认属性时，给它传递NULL就行。具体线程有那些属性，我们后面再做介绍。
好，那么我们就利用这些接口，来完成在Linux上的第一个多线程程序，见代码1所示：
```cpp
#include <stdio.h>
#include <pthread.h>
void* start_thread( void *arg )
{
    printf( "This is a thread and arg = %d.\n", *(int*)arg);
    *(int*)arg = 0;
    return arg;
}
int main( int argc, char *argv[] )
{
    pthread_t th;
    int ret;
    int arg = 10;
    int *thread_ret = NULL;
    ret = pthread_create( &th, NULL, start_thread, &arg );
    if( ret != 0 ){
        printf( "Create thread error!\n");
        return -1;
    }
    printf( "This is the main process.\n" );
    pthread_join( th, (void**)&thread_ret );
    printf( "thread_ret = %d.\n", *thread_ret );
    return 0;
}
```

输出

This is the main process.
This is a thread and arg = 10.
thread_ret = 0.

在第30行调用pthread_create()接口创建了一个新的线程，这个线程的入口函数是start_thread()，并且给这个入口函数传递了一个参数，且参数值为10。这个新创建的线程要执行的任务非常简单，只是将显示“This is a thread and arg = 10”这个字符串，因为arg这个参数值已经定义好了，就是10。之后线程将arg参数的值修改为0，并将它作为线程的返回值返回给系统。与此同时，主进程做的事情就是继续判断这个线程是否创建成功了。在我们的例子中基本上没有创建失败的可能。主进程会继续输出“This is the main process”字符串，然后调用pthread_join()接口与刚才的创建进行合并。这个接口的第一个参数就是新创建线程的句柄了，而第二个参数就会去接受线程的返回值。pthread_join()接口会阻塞主进程的执行，直到合并的线程执行结束。由于线程在结束之后会将0返回给系统，那么pthread_join()获得的线程返回值自然也就是0。输出结果“thread_ret = 0”也证实了这一点。
那么现在有一个问题，那就是pthread_join()接口干了什么？什么是线程合并呢？

#### 2. 线程的合并与分离
我们首先要明确的一个问题就是什么是线程的合并。从前面的叙述中读者们已经了解到了，pthread_create()接口负责创建了一个线程。那么线程也属于系统的资源，这跟内存没什么两样，而且线程本身也要占据一定的内存空间。众所周知的一个问题就是C或C++编程中如果要通过malloc()或new分配了一块内存，就必须使用free()或delete来回收这块内存，否则就会产生著名的内存泄漏问题。既然线程和内存没什么两样，那么有创建就必须得有回收，否则就会产生另外一个著名的资源泄漏问题，这同样也是一个严重的问题。那么线程的合并就是回收线程资源了。
线程的合并是一种主动回收线程资源的方案。当一个进程或线程调用了针对其它线程的pthread_join()接口，就是线程合并了。这个接口会阻塞调用进程或线程，直到被合并的线程结束为止。当被合并线程结束，pthread_join()接口就会回收这个线程的资源，并将这个线程的返回值返回给合并者。
与线程合并相对应的另外一种线程资源回收机制是线程分离，调用接口是pthread_detach()。线程分离是将线程资源的回收工作交由系统自动来完成，也就是说当被分离的线程结束之后，系统会自动回收它的资源。因为线程分离是启动系统的自动回收机制，那么程序也就无法获得被分离线程的返回值，这就使得pthread_detach()接口只要拥有一个参数就行了，那就是被分离线程句柄。
线程合并和线程分离都是用于回收线程资源的，可以根据不同的业务场景酌情使用。不管有什么理由，你都必须选择其中一种，否则就会引发资源泄漏的问题，这个问题与内存泄漏同样可怕。

#### 3. 线程的属性

线程是有属性的，这个属性由一个线程属性对象来描述。线程属性对象由pthread_attr_init()接口初始化，并由pthread_attr_destory()来销毁，它们的完整定义是：
```cpp
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destory(pthread_attr_t *attr);
```
那么线程拥有哪些属性呢？一般地，Linux下的线程有：绑定属性、分离属性、调度属性、堆栈大小属性和满占警戒区大小属性。下面我们就分别来介绍这些属性。

##### 3.1 绑定属性
说到这个绑定属性，就不得不提起另外一个概念：轻进程（Light Weight Process，简称LWP）。轻进程和Linux系统的内核线程拥有相同的概念，属于内核的调度实体。一个轻进程可以控制一个或多个线程。默认情况下，对于一个拥有n个线程的程序，启动多少轻进程，由哪些轻进程来控制哪些线程由操作系统来控制，这种状态被称为非绑定的。那么绑定的含义就很好理解了，只要指定了某个线程“绑”在某个轻进程上，就可以称之为绑定的了。被绑定的线程具有较高的相应速度，因为操作系统的调度主体是轻进程，绑定线程可以保证在需要的时候它总有一个轻进程可用。绑定属性就是干这个用的。
设置绑定属性的接口是pthread_attr_setscope()，它的完整定义是：
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
它有两个参数，第一个就是线程属性对象的指针，第二个就是绑定类型，拥有两个取值：PTHREAD_SCOPE_SYSTEM（绑定的）和PTHREAD_SCOPE_PROCESS（非绑定的）。代码2演示了这个属性的使用。
```cpp
#include <stdio.h>
#include <pthread.h>
……
int main( int argc, char *argv[] )
{
    pthread_attr_t attr;
    pthread_t th;
    ……
    pthread_attr_init( &attr );
    pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
    pthread_create( &th, &attr, thread, NULL );
    ……
}
```
不知道你是否在这里发现了本文的矛盾之处。就是这个绑定属性跟我们之前说的NPTL有矛盾之处。在介绍NPTL的时候就说过业界有一种m:n的线程方案，就跟这个绑定属性有关。但是笔者还说过NPTL因为Linux的“蠢”没有采取这种方案，而是采用了“1:1”的方案。这也就是说，Linux的线程永远都是绑定。对，Linux的线程永远都是绑定的，所以PTHREAD_SCOPE_PROCESS在Linux中不管用，而且会返回ENOTSUP错误。
既然Linux并不支持线程的非绑定，为什么还要提供这个接口呢？答案就是兼容！因为Linux的NTPL是号称POSIX标准兼容的，而绑定属性正是POSIX标准所要求的，所以提供了这个接口。如果读者们只是在Linux下编写多线程程序，可以完全忽略这个属性。如果哪天你遇到了支持这种特性的系统，别忘了我曾经跟你说起过这玩意儿

##### 3.2 分离属性

前面说过线程能够被合并和分离，分离属性就是让线程在创建之前就决定它应该是分离的。如果设置了这个属性，就没有必要调用pthread_join()或pthread_detach()来回收线程资源了。
设置分离属性的接口是pthread_attr_setdetachstate()，它的完整定义是：
```cpp
pthread_attr_setdetachstat(pthread_attr_t *attr, int detachstate);
```
它的第二个参数有两个取值：PTHREAD_CREATE_DETACHED（分离的）和PTHREAD_CREATE_JOINABLE（可合并的，也是默认属性）。

##### 3.3 调度属性
线程的调度属性有三个，分别是：算法、优先级和继承权。
Linux提供的线程调度算法有三个：轮询、先进先出和其它。其中轮询和先进先出调度算法是POSIX标准所规定，而其他则代表采用Linux自己认为更合适的调度算法，所以默认的调度算法也就是其它了。轮询和先进先出调度算法都属于实时调度算法。轮询指的是时间片轮转，当线程的时间片用完，系统将重新分配时间片，并将它放置在就绪队列尾部，这样可以保证具有相同优先级的轮询任务获得公平的CPU占用时间；先进先出就是先到先服务，一旦线程占用了CPU则一直运行，直到有更高优先级的线程出现或自己放弃。
设置线程调度算法的接口是pthread_attr_setschedpolicy()，它的完整定义是：
```cpp
pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
```
它的第二个参数有三个取值：SCHED_RR（轮询）、SCHED_FIFO（先进先出）和SCHED_OTHER（其它）。
Linux的线程优先级与进程的优先级不一样，进程优先级我们后面再说。Linux的线程优先级是从1到99的数值，数值越大代表优先级越高。而且要注意的是，只有采用SHCED_RR或SCHED_FIFO调度算法时，优先级才有效。对于采用SCHED_OTHER调度算法的线程，其优先级恒为0。
设置线程优先级的接口是pthread_attr_setschedparam()，它的完整定义是：
```cpp
struct sched_param {
    int sched_priority;
}
int pthread_attr_setschedparam(pthread_attr_t *attr, struct sched_param *param);
```
sched_param结构体的sched_priority字段就是线程的优先级了。
此外，即便采用SCHED_RR或SCHED_FIFO调度算法，线程优先级也不是随便就能设置的。首先，进程必须是以root账号运行的；其次，还需要放弃线程的继承权。什么是继承权呢？就是当创建新的线程时，新线程要继承父线程（创建者线程）的调度属性。如果不希望新线程继承父线程的调度属性，就要放弃继承权。
设置线程继承权的接口是pthread_attr_setinheritsched()，它的完整定义是：
```cpp
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
```
它的第二个参数有两个取值：PTHREAD_INHERIT_SCHED（拥有继承权）和PTHREAD_EXPLICIT_SCHED（放弃继承权）。新线程在默认情况下是拥有继承权。
##### 3.4 堆栈大小属性
从前面的这些例子中可以了解到，线程的主函数与程序的主函数main()有一个很相似的特性，那就是可以拥有局部变量。虽然同一个进程的线程之间是共享内存空间的，但是它的局部变量确并不共享。原因就是局部变量存储在堆栈中，而不同的线程拥有不同的堆栈。Linux系统为每个线程默认分配了8MB的堆栈空间，如果觉得这个空间不够用，可以通过修改线程的堆栈大小属性进行扩容。
修改线程堆栈大小属性的接口是pthread_attr_setstacksize()，它的完整定义为：
```cpp
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
```
它的第二个参数就是堆栈大小了，以字节为单位。需要注意的是，线程堆栈不能小于16KB，而且尽量按4KB(32位系统)或2MB（64位系统）的整数倍分配，也就是内存页面大小的整数倍。此外，修改线程堆栈大小是有风险的，如果你不清楚你在做什么，最好别动它（其实我很后悔把这么危险的东西告诉了你:）。

##### 3.5 满栈警戒区属性
既然线程是有堆栈的，而且还有大小限制，那么就一定会出现将堆栈用满的情况。线程的堆栈用满是非常危险的事情，因为这可能会导致对内核空间的破坏，一旦被有心人士所利用，后果也不堪设想。为了防治这类事情的发生，Linux为线程堆栈设置了一个满栈警戒区。这个区域一般就是一个页面，属于线程堆栈的一个扩展区域。一旦有代码访问了这个区域，就会发出SIGSEGV信号进行通知。
虽然满栈警戒区可以起到安全作用，但是也有弊病，就是会白白浪费掉内存空间，对于内存紧张的系统会使系统变得很慢。所有就有了关闭这个警戒区的需求。同时，如果我们修改了线程堆栈的大小，那么系统会认为我们会自己管理堆栈，也会将警戒区取消掉，如果有需要就要开启它。
修改满栈警戒区属性的接口是pthread_attr_setguardsize()，它的完整定义为：
```cpp
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);
```
它的第二个参数就是警戒区大小了，以字节为单位。与设置线程堆栈大小属性相仿，应该尽量按照4KB或2MB的整数倍来分配。当设置警戒区大小为0时，就关闭了这个警戒区。
虽然栈满警戒区需要浪费掉一点内存，但是能够极大的提高安全性，所以这点损失是值得的。而且一旦修改了线程堆栈的大小，一定要记得同时设置这个警戒区。
#### 4. 线程本地存储
内线程之间可以共享内存地址空间，线程之间的数据交换可以非常快捷，这是线程最显著的优点。但是多个线程访问共享数据，需要昂贵的同步开销，也容易造成与同步相关的BUG，更麻烦的是有些数据根本就不希望被共享，这又是缺点。可谓：“成也萧何，败也萧何”，说的就是这个道理。
C程序库中的errno是个最典型的一个例子。errno是一个全局变量，会保存最后一个系统调用的错误代码。在单线程环境并不会出现什么问题。但是在多线程环境，由于所有线程都会有可能修改errno，这就很难确定errno代表的到底是哪个系统调用的错误代码了。这就是有名的“非线程安全（Non Thread-Safe）”的。
此外，从现代技术角度看，在很多时候使用多线程的目的并不是为了对共享数据进行并行处理（在Linux下有更好的方案，后面会介绍）。更多是由于多核心CPU技术的引入，为了充分利用CPU资源而进行并行运算（不互相干扰）。换句话说，大多数情况下每个线程只会关心自己的数据而不需要与别人同步。
为了解决这些问题，可以有很多种方案。比如使用不同名称的全局变量。但是像errno这种名称已经固定了的全局变量就没办法了。在前面的内容中提到在线程堆栈中分配局部变量是不在线程间共享的。但是它有一个弊病，就是线程内部的其它函数很难访问到。目前解决这个问题的简便易行的方案是线程本地存储，即Thread Local Storage，简称TLS。利用TLS，errno所反映的就是本线程内最后一个系统调用的错误代码了，也就是线程安全的了。
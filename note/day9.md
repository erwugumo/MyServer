C语言#pragma使用方法
一、总结
1、#pragma用于指示编译器完成一些特定的动作
2、#pragma所定义的很多指示字是编译器特有的(每种编译可能都不一样)
      （1） #pragma message 用于自定义编译信息
      （2）#pragma once 用于保证头文件只被编译一次
      （3）#pragama pack用于指定内存对齐(一般用在结构体)
               struct占用内存大小
                    1）第一个成员起始于0偏移处
                    2）每个成员按其类型大小和pack参数中较小的一个进行对齐
                             ——偏移地址必须能被对齐参数整除
                             ——结构体成员的对齐参数(注意是对齐参数，而不是结构体长度)取其内部长度最大的数据成员作为其大小
                    3）结构体总长度必须为所有对齐参数的整数倍
                            编译器在默认情况下按照4字节对齐
3、#pragma在不同的编译器间是不可移植的
      （1）预处理器将忽略它不认识#pragma指令
      （2）不同的编译器可能以不同的方式解释同一条#pragma指令

Functor与Function，哪个更好？？
　　Function几乎是任何语言的元素之一，从Pascal，Fortran到C++，VB，几乎任何时代的语言都支持它。在C++里，随着 C++标准库的推出，
人们开始渐渐的接触到另一种定义函数的方式：Functor。所谓Functor，其实就是重载了operator () 的类，其使用方式和普通函数差不多
（这正是C++处处体现的一种思想：只在定义上有区别，在使用上毫无区别）。
　　譬如说，如果我们要定义一个函数，将传入的整型引用加一，我们可以有两种方法定义：
inline void increase_one_func(int& i)
{
　　++i;
}

class increase_one_functor
{
public:
　　void operator()(int& i)
　　{
　　　　++i;
　　}
}
increase_one_functor;
使用起来则没什么区别：
int i=0;
increase_one_func(i);
increase_one_functor(i);

　　那Function和Functor到底有什么区别呢？其实他们定义方式的区别已经决定了他们使用上的区别了。

　　首先，Functor相比Function来说，可以传递更多的信息：
因为Functor是以类的方式存在的，它可以包含任意多的信息。除了传入参数以外，你还可以在类内预设一些其它的信息。
譬如说，下面这个Functor：
class increase_one_functor
{
　　int mIncrement;
public:
　　increase_one_functor(int increment=1):mIncrement(increment)
　　{}
　　void operator()(int& i)
　　{
　　　　++i;
　　}
}
　　可以很方便的实现将一个整数增加任意值的功能：
increase_one_functor(100)(i);// 将i加100。
　　而Function就很难实现这样的可扩展性。

　　其次，在作为参数传递时，Functor的效率往往比Function要高。这是因为，在把Functor作为参数传递时，你实际上传递的是Functor对象，
在整个编译过程中，编译器始终知道它所在处理的Functor对象是哪个Functor类的，也就是说，它可以做编译时的优化。
而对于Function来说，它往往以指针的方式传递，对于编译器来说，很难做（并不是不可能）编译时的优化。
　　下面这段程序或许可以说明问题：
#include <windows.h>
#include <iostream>

using namespace std;

inline void increase_one_func(int& i)
{
　　++i;
}

class increase_one_functor
{
public:
　　void operator()(int& i)
　　{
　　　　++i;
　　}
}
increase_one_functor;

const int VECTOR_SIZE=1024*16;

void treat_vector_func(int ai[VECTOR_SIZE],void (*func)(int& i))
{
　　for(int i=0;i<VECTOR_SIZE;++i)
　　　　func(ai[i]);
}

template<typename T>
void treat_vector_func(int ai[VECTOR_SIZE],T functor)
{
　　for(int i=0;i<VECTOR_SIZE;++i)
　　　　functor(ai[i]);
}

void main(void)
{
　　static int ai[VECTOR_SIZE];
　　const int CALL_TIMES=10240;
　　{
　　　　long l=GetTickCount();
　　　　for(int i=0;i<CALL_TIMES;++i)
　　　　　　treat_vector_func(ai,increase_one_func);
　　　　l=GetTickCount()-l;
　　　　cerr<<"function "<<l<<endl;
　　}
　　{
　　　　long l=GetTickCount();
　　　　for(int i=0;i<CALL_TIMES;++i)
　　　　　　treat_vector_func(ai,increase_one_functor);
　　　　l=GetTickCount()-l;
　　　　cerr<<"functor "<<l<<endl;
　　}
}
　　运行结果如下：
　　Debug模式下（也就是说，没有任何优化的情况下）：
function 37623
functor 36825
Release模式下：
function 4573
functor 1726
　　可见，functor比function要快很多。

1.close()函数

#include<unistd.h>
int close(int sockfd);     //返回成功为0，出错为-1.
close 一个套接字的默认行为是把套接字标记为已关闭，然后立即返回到调用进程，该套接字描述符不能再由调用进程使用，也就是说它不能再作为read或write的第一个参数，然而TCP将尝试发送已排队等待发送到对端的任何数据，发送完毕后发生的是正常的TCP连接终止序列。
在多进程并发服务器中，父子进程共享着套接字，套接字描述符引用计数记录着共享着的进程个数，当父进程或某一子进程close掉套接字时，描述符引用计数会相应的减一，当引用计数仍大于零时，这个close调用就不会引发TCP的四路握手断连过程。

2.shutdown()函数

#include<sys/socket.h>
int shutdown(int sockfd,int howto);  //返回成功为0，出错为-1.
该函数的行为依赖于howto的值

1.SHUT_RD：值为0，关闭连接的读这一半。

2.SHUT_WR：值为1，关闭连接的写这一半。

3.SHUT_RDWR：值为2，连接的读和写都关闭。

终止网络连接的通用方法是调用close函数。但使用shutdown能更好的控制断连过程（使用第二个参数）。

3.两函数的区别
close与shutdown的区别主要表现在：
close函数会关闭套接字ID，如果有其他的进程共享着这个套接字，那么它仍然是打开的，这个连接仍然可以用来读和写，
并且有时候这是非常重要的 ，特别是对于多进程并发服务器来说。

子进程close套接字后，套接字对于父进程来说仍然是可读和可写的，尽管父进程永远都不会写入数据。
因此，此socket的断连过程没有发生，因此，服务器端就不会检测到EOF标识，会一直等待从客户端来的数据。
而此时父进程也不会检测到服务器端发来的EOF标识。这样服务器端和客户端陷入了死锁（deadlock）。
如果用shutdown代替close，则会避免死锁的发生。


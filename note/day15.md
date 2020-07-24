WebServer主要类：
Channel：每个channel对象自始至终只属于一个eventloop，每个channel对象只属于某一个IO线程，只负责一个文件描述符的IO事件分发，但它不拥有这个fd。
Epoll
EventLoop：每个线程只有一个eventloop对象，创建了eventloop对象的是IO线程
EventLoopThread
EventLoopThreadPool
HttpData
Server
ThreadPool
TimerNode
TimerManager


Server的构造函数里执行bind和listen，start函数里执行handNewConn，handNewConn函数里执行accept
Server里面也有eventloop，还有eventloop的线程池，池子里有若干线程，都在跑eventloop

Server的eventloop负责接收新request，线程池的eventloop负责处理已有socket

每个eventloop对应一个channel，eventloop里进行循环，循环里面调用poll函数，poll函数里面调用的epoll_wait，之后调用channel，channel中收集过来的request，返回一个vector，在loop里处理，之后再循环

Channel

Java Nio中Channel的作用相当于Java IO中的Stream,网络IO操作是在Channel上进行的。Netty中的Channel是对Java Nio中Channel的进一步包装，不仅仅提供了基本的IO操作，如bind,connect,read,write之外，还提供了Netty框架的一些功能，如获取channel的pipeline，eventloop等。

Java Nio中的SocketChannel和ServerSocketChannel没有统一的视图，使用不方便，而Netty通过包装Java Nio中的Channel提供了统一的视图，并且在不同的子类中实现不同的功能，公共功能在父类中进行实现，最大程度实现了接口的重用。并且将Netty相关的功能类聚合在channel中，由channel进行统一调度和分配，功能实现更加灵活。

EventLoop

Netty是事件驱动模型，使用不同的时间来通知我们状态的改变或者是操作状态的改变，EventLoop定义了在生命周期中有事件发生，用来处理连接的一个核心抽象。在内部，会为每一个channel分配一个EventLoop。

EventLoop本身只由一个线程驱动，其处理了一个Channel的所有I/O事件，并且在该EventLoop的整个生命周期内都不会改变。这个简单而强大的设计消除了你可能有的在ChannelHandler实现中需要进行同步的任何顾虑。　

Netty应用程序尽可能的重用EventLoop，以减少创建线程带来的开销。

1、一般有这个场景，当前有个EventLoop loopA在时间循环中。
2、我们这时候有个读事件想在loopA中监听，好，先创建一个channel，弄好描述符，还有loop_也指向loopA，设置好感兴趣的事件类型，还有回调函数。其中index为kNew，用来等会告诉EPollPoller说，这个channel注册情况是一片空白，既不在epollfd上监听，也没再那个map上。然后enableXXX，里面就会update一下，调用了loopA中的updatechannel方法。
3、loopA一看，什么？要我帮你update，这里可能是注册，也可能是del啥的，具体干嘛他不知道，但他知道他的手下—-成员变量poller_是干这类事情的，他会吩咐poller说，这事情交给你去办。而channel交给loopA时候把“登记材料”（index、events）也准备好了，所以在epollfd上登记这个读事件就会在EPollPoller上实际进行。
4、登记完后，过了会，读事件来了，这时候最先知道这件事情的是epollfd，于是他把这事情告诉了loopA手下activeChannels，让他转告loopA，让他拿主意，loopA听到后，就吩咐这事件所属的channel说，你们的读事件来了，快去解决一下。
5、然后channel就检测revent，是epollfd上检测出的事件的类型，然后再调用相应的读或写或出错回调函数。

HttpData::newEvent执行添加epoll的event任务
Server::handNewConn调用newEvent
channel的setReadHandler调用Server::handNewConn

Server类里有一个loop一个channel，channel中进行新事件的注册
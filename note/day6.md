EAGAIN-（一般用于非阻塞的系统调用）
非阻塞的系统调用，由于资源限制/不满足条件，导致返回值为EAGAIN

在Linux环境下开发经常会碰到很多错误(设置errno)，其中EAGAIN是其中比较常见的一个错误(比如用在非阻塞操作中)。

如：首先是把套接字设置为异步的了，然后在使用write发送数据时采取的方式是循环发送大量的数据；
由于是异步的，write\send将要发送的数据提交到发送缓冲区后是立即返回的，并不需要对端确认数据已接收。
在这种情况下是很有可能出现发送缓冲区被填满，导致write\send无法再向缓冲区提交要发送的数据。
因此就产生了Resource temporarily unavailable的错误（资源暂时不可用），EAGAIN 的意思也很明显，就是要你再次尝试。

从字面上来看，是提示再试一次。这个错误经常出现在当应用程序进行一些非阻塞(non-blocking)操作(对文件或socket)的时候。

如：以 O_NONBLOCK的标志打开文件/socket/FIFO，如果连续做read操作而没有数据可读。
此时程序不会阻塞起来等待数据准备就绪返回，read函数会返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。
又例如，当一个系统调用(比如fork)因为没有足够的资源(比如虚拟内存)而执行失败，返回EAGAIN提示其再调用一次(也许下次就能成功)。

Linux - 非阻塞socket编程处理EAGAIN错误
在linux进行非阻塞的socket接收数据时经常出现Resource temporarily unavailable，errno代码为11(EAGAIN)，这是什么意思？
 ⇒ ⇒ ⇒ 这表明在非阻塞模式下调用了阻塞操作，在该操作没有完成就返回这个错误，这个错误不会破坏socket的同步，不用管它，下次循环接着recv就可以。
对非阻塞socket而言，EAGAIN不是一种错误。在VxWorks和Windows上，EAGAIN的名字叫做EWOULDBLOCK。

这是我在讨论区看到的一个回答，写的很好，让我明白了为什么单个服务器程序可承受最大连接数可以达到几十W

要写网络程序就必须用Socket，这是程序员都知道的。而且，面试的时候，我们也会问对方会不会Socket编程？
一般来说，很多人都会说，Socket编程基本就是listen，accept以及send，write等几个基本的操作。是的，就跟常见的文件操作一样，只要写过就一定知道。

对于网络编程，我们也言必称TCP/IP，似乎其它网络协议已经不存在了。对于TCP/IP，我们还知道TCP和UDP，前者可以保证数据的正确和可靠性，后者则允许数据丢失。
最后，我们还知道，在建立连接前，必须知道对方的IP地址和端口号。除此，普通的程序员就不会知道太多了，很多时候这些知识已经够用了。
最多，写服务程序的时候，会使用多线程来处理并发访问。

我们还知道如下几个事实：

1。一个指定的端口号不能被多个程序共用。比如，如果IIS占用了80端口，那么Apache就不能也用80端口了。
2。很多防火墙只允许特定目标端口的数据包通过。
3。服务程序在listen某个端口并accept某个连接请求后，会生成一个新的socket来对该请求进行处理。


于是，一个困惑了我很久的问题就产生了。
如果一个socket创建后并与80端口绑定后，是否就意味着该socket占用了80端口呢？
如果是这样的，那么当其accept一个请求后，生成的新的socket到底使用的是什么端口呢（我一直以为系统会默认给其分配一个空闲的端口号）？
如果是一个空闲的端口，那一定不是80端口了，于是以后的TCP数据包的目标端口就不是80了--防火墙一定会阻止其通过的！
实际上，我们可以看到，防火墙并没有阻止这样的连接，而且这是最常见的连接请求和处理方式。
我的不解就是，为什么防火墙没有阻止这样的连接？它是如何判定那条连接是因为connet80端口而生成的？
是不是TCP数据包里有什么特别的标志？或者防火墙记住了什么东西？
后来，我又仔细研读了TCP/IP的协议栈的原理，对很多概念有了更深刻的认识。
比如，在TCP和UDP同属于传输层，共同架设在IP层（网络层）之上。而IP层主要负责的是在节点之间（End to End）的数据包传送，这里的节点是一台网络设备，比如计算机。
因为IP层只负责把数据送到节点，而不能区分上面的不同应用，所以TCP和UDP协议在其基础上加入了端口的信息，端口于是标识的是一个节点上的一个应用。
除了增加端口信息，UDP协议基本就没有对IP层的数据进行任何的处理了。
而TCP协议还加入了更加复杂的传输控制，比如滑动的数据发送窗口（Slice Window），以及接收确认和重发机制，以达到数据的可靠传送。
不管应用层看到的是怎样一个稳定的TCP数据流，下面传送的都是一个个的IP数据包，需要由TCP协议来进行数据重组。

所以，我有理由怀疑，防火墙并没有足够的信息判断TCP数据包的更多信息，除了IP地址和端口号。
而且，我们也看到，所谓的端口，是为了区分不同的应用的，以在不同的IP包来到的时候能够正确转发。

TCP/IP只是一个协议栈，就像操作系统的运行机制一样，必须要具体实现，同时还要提供对外的操作接口。
就像操作系统会提供标准的编程接口，比如Win32编程接口一样，TCP/IP也必须对外提供编程接口，这就是Socket编程接口--原来是这么回事啊！


在Socket编程接口里，设计者提出了一个很重要的概念，那就是socket。这个socket跟文件句柄很相似，实际上在BSD系统里就是跟文件句柄一样存放在一样的进程句柄表里。
这个socket其实是一个序号，表示其在句柄表中的位置。
这一点，我们已经见过很多了，比如文件句柄，窗口句柄等等。这些句柄，其实是代表了系统中的某些特定的对象，用于在各种函数中作为参数传入，
以对特定的对象进行操作--这其实是C语言的问题，在C++语言里，这个句柄其实就是this指针，实际就是对象指针啦。

现在我们知道，socket跟TCP/IP并没有必然的联系。Socket编程接口在设计的时候，就希望也能适应其他的网络协议。
所以，socket的出现只是可以更方便的使用TCP/IP协议栈而已，其对TCP/IP进行了抽象，形成了几个最基本的函数接口。比如create，listen，accept，connect，read和write等等。

现在我们明白，如果一个程序创建了一个socket，并让其监听80端口，其实是向TCP/IP协议栈声明了其对80端口的占有。
以后，所有目标是80端口的TCP数据包都会转发给该程序（这里的程序，因为使用的是Socket编程接口，所以首先由Socket层来处理）。
所谓accept函数，其实抽象的是TCP的连接建立过程。accept函数返回的新socket其实指代的是本次创建的连接，而一个连接是包括两部分信息的，一个是源IP和源端口，另一个是宿IP和宿端口。
所以，accept可以产生多个不同的socket，而这些socket里包含的宿IP和宿端口是不变的，变化的只是源IP和源端口。
这样的话，这些socket宿端口就可以都是80，而Socket层还是能根据源/宿对来准确地分辨出IP包和socket的归属关系，从而完成对TCP/IP协议的操作封装！
而同时，放火墙的对IP包的处理规则也是清晰明了，不存在前面设想的种种复杂的情形。
明白socket只是对TCP/IP协议栈操作的抽象，而不是简单的映射关系，这很重要！

 本人从事TCP的socket编程多年，趟过很多坑，对于TCP是“全双工的字节流”这几个字的含义有深刻理解。
 这几个字，文字虽少，但字字精辟。如果没有深刻理解，编程中可能知其然不知其所以然，难有大作为。“全双工的字节流”详解如下：

(1) 全双工：意味着，TCP的收发是可以同时进行的。亦即接收的时候可以发送，发送的时候也可以接收，两者互不冲突，可同时进行。
而OpenSSL则不同，OpenSSL是单工的，亦即收和发不能同时进行，同一时刻只能其中之一进行，也就意味着，OpenSSL的收和发之间必须要加锁互斥，两者不能同时工作。理
解了这一点，就很容易明白，OpenSSL的性能必然是低下的，即使不考虑加密带来的损失，理论上也应比普通的socket性能至少低50%，这一工作原理是很重要的原因。
(2) 字节：意味着，无论物理层或链路层收到的数据是否为一个个二进制位的数据，在TCP层接收到的数据一定是一个个字节。
也就是说，我们在进行socket编程时，只需要考虑接收一个个字节，而不是一个个位的数据。
(3) 流：意味着，socket的数据无头无尾，就像流水一样，如果从中间任意位置起，你无法知道一个消息包确切的开始或结束位置，除非从TCP的头开始算起。
① 也就是意味着，我们在应用层编程时，必须定义一个应用层的包头，从收到的第一个字节开始，通过该包头能确定一个包的长度，然后根据包的长度，确定一个个包的起止位置。
这也就是我们看到所有的TCP的socket编程中，都需要定义一个包头，并在其中可以获得应用层包的总长度的原因。
② 同时也意味着，数据流不是一个个应用层的数据包，对于接收，可能收到的是一个完整的数据包，也可能了收到1/3个数据包，也可能收到4/5个数据包，
或者一个完整的数据包，也可能收到1.5个或3.2个数据包，这是不确定的。需要自己在编程中，自己解析确定收到的是否完整的数据包，如果不足，需要继续接收；如果多余，需要分拆成多个包。

tcp建立连接三次握手，主动方发送请求syn，server接收到信息，返回带有数据包的信息syn_sent，然后接收到信息的一方再发送确认信息ACK给server，第三次握手失败（超时）时，
服务器并不会重传ack报文，server会发送RTS报文段并主动关闭至closed，以防止syn洪泛攻击。

syn洪泛攻击

通俗的理解是：当第三次握手没有发送确认信息时，等待一段时间后，主机就会断开之前的半开连接并回收资源，这为dos（deny of service）攻击埋下隐患，
当主动方主动发送大量的syn数据包，但并不做出第三次握手响应，server就会为这些syn包分配资源（但并未使用），就会使server占用大量内存，使server连接环境耗尽，这就是syn洪泛攻击

GET / HTTP/1.1
Host: gumo.online:3000
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8
Cookie: wp-settings-1=libraryContent%3Dbrowse%26editor%3Dhtml%26hidetb%3D1%26mfold%3Do; wp-settings-time-1=1590339643



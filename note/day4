1.closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket：

BOOL bReuseaddr=TRUE;

setsockopt(s,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL));

 

2. 如果要已经处于连接状态的soket在调用closesocket后强制关闭，不经历

TIME_WAIT的过程：

BOOL bDontLinger = FALSE;

setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL));

 

3.在send(),recv()过程中有时由于网络状况等原因，发收不能预期进行,而设置收发时限：

int nNetTimeout=1000;//1秒

//发送时限

setsockopt(socket，SOL_S0CKET,SO_SNDTIMEO，(char *)&nNetTimeout,sizeof(int));

//接收时限

setsockopt(socket，SOL_S0CKET,SO_RCVTIMEO，(char *)&nNetTimeout,sizeof(int));

 

4.在send()的时候，返回的是实际发送出去的字节(同步)或发送到socket缓冲区的字节

(异步);系统默认的状态发送和接收一次为8688字节(约为8.5K)；在实际的过程中发送数据

和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发：

// 接收缓冲区

int nRecvBuf=32*1024;//设置为32K

setsockopt(s,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));

//发送缓冲区

int nSendBuf=32*1024;//设置为32K

setsockopt(s,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

 

5. 如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响

程序的性能：

int nZero=0;

setsockopt(socket，SOL_S0CKET,SO_SNDBUF，(char *)&nZero,sizeof(nZero));

 

6.同上在recv()完成上述功能(默认情况是将socket缓冲区的内容拷贝到系统缓冲区)：

int nZero=0;

setsockopt(socket，SOL_S0CKET,SO_RCVBUF，(char *)&nZero,sizeof(int));

 

7.一般在发送UDP数据报的时候，希望该socket发送的数据具有广播特性：

BOOL bBroadcast=TRUE;

setsockopt(s,SOL_SOCKET,SO_BROADCAST,(const char*)&bBroadcast,sizeof(BOOL));

 

8.在client连接服务器过程中，如果处于非阻塞模式下的socket在connect()的过程中可

以设置connect()延时,直到accpet()被呼叫(本函数设置只有在非阻塞的过程中有显著的

作用，在阻塞的函数调用中作用不大)

BOOL bConditionalAccept=TRUE;

setsockopt(s,SOL_SOCKET,SO_CONDITIONAL_ACCEPT,(const char*)&bConditionalAccept,sizeof(BOOL));

 

9.如果在发送数据的过程中(send()没有完成，还有数据没发送)而调用了closesocket(),以前我们

一般采取的措施是"从容关闭"shutdown(s,SD_BOTH),但是数据是肯定丢失了，如何设置让程序满足具体

应用的要求(即让没发完的数据发送出去后在关闭socket)？

struct linger {

u_short l_onoff;

u_short l_linger;

};

linger m_sLinger;

m_sLinger.l_onoff=1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)

// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;

m_sLinger.l_linger=5;//(容许逗留的时间为5秒)

setsockopt(s,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));

SO_LINGER

 

   此选项指定函数close对面向连接的协议如何操作（如TCP）。缺省close操作是立即返回，如果有数据残留在套接口缓冲区中则系统将试着将这些数据 发送给对方。

 

 

SO_LINGER选项用来改变此缺省设置。使用如下结构：

 

struct linger {

 

     int l_onoff; 

 

     int l_linger; 

 

};

 

 

有下列三种情况：

 

l_onoff为0，则该选项关闭，l_linger的值被忽略，等于缺省情况，close立即返回；

 

l_onoff为非0，l_linger为0，则套接口关闭时TCP夭折连接，TCP将丢弃保留在套接口发送缓冲区中的任何数据并发送一个RST 给对方，而不是通常的四分组终止序列，这避免了TIME_WAIT状态；

 

l_onoff 为非0，l_linger为非0，当套接口关闭时内核将拖延一段时间（由l_linger决定）。如果套接口缓冲区中仍残留数据，进程将处于睡眠状态，直 到（a）所有数据发送完且被对方确认，之后进行正常的终止序列（描述字访问计数为0）或（b）延迟时间到。此种情况下，应用程序检查close的返回值是 非常重要的，如果在数据发送完并被确认前时间到，close将返回EWOULDBLOCK错误且套接口发送缓冲区中的任何数据都丢失。close的成功返 回仅告诉我们发送的数据（和FIN）已由对方TCP确认，它并不能告诉我们对方应用进程是否已读了数据。如果套接口设为非阻塞的，它将不等待close完 成。

 

 

l_linger的单位依赖于实现，4.4BSD假设其单位是时钟滴答（百分之一秒），但Posix.1g规定单位为秒。

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

设置套接口的选项。

 

  #include <winsock.h>

 

  int PASCAL FAR setsockopt( SOCKET s, int level, int optname,

  const char FAR* optval, int optlen);

 

  s：标识一个套接口的描述字。

  level：选项定义的层次；目前仅支持SOL_SOCKET和IPPROTO_TCP层次。

  optname：需设置的选项。

  optval：指针，指向存放选项值的缓冲区。

  optlen：optval缓冲区的长度。

 

注释：

  setsockopt()函数用于任意类型、任意状态套接口的设置选项值。尽管在不同协议层上存在选项，但本函数仅定义了最高的“套接口”层次上的选项。选项影响套接口的操作，诸如加急数据是否在普通数据流中接收，广播数据是否可以从套接口发送等等。

  有两种套接口的选项：一种是布尔型选项，允许或禁止一种特性；另一种是整形或结构选项。允许一个布尔型选项，则将optval指向非零整形数；禁止一个选项optval指向一个等于零的整形数。对于布尔型选项，optlen应等于sizeof(int)；对其他选项，optval指向包含所需选项的整形数或结构，而optlen则为整形数或结构的长度。SO_LINGER选项用于控制下述情况的行动：套接口上有排队的待发送数据，且closesocket()调用已执行。参见closesocket()函数中关于SO_LINGER选项对closesocket()语义的影响。应用程序通过创建一个linger结构来设置相应的操作特性：

  struct linger {

        int l_onoff;

        int l_linger;

  };

  为了允许SO_LINGER，应用程序应将l_onoff设为非零，将l_linger设为零或需要的超时值（以秒为单位），然后调用setsockopt()。为了允许SO_DONTLINGER（亦即禁止SO_LINGER），l_onoff应设为零，然后调用setsockopt()。

  缺省条件下，一个套接口不能与一个已在使用中的本地地址捆绑（参见bind()）。但有时会需要“重用”地址。因为每一个连接都由本地地址和远端地址的组合唯一确定，所以只要远端地址不同，两个套接口与一个地址捆绑并无大碍。为了通知WINDOWS套接口实现不要因为一个地址已被一个套接口使用就不让它与另一个套接口捆绑，应用程序可在bind()调用前先设置SO_REUSEADDR选项。请注意仅在bind()调用时该选项才被解释；故此无需（但也无害）将一个不会共用地址的套接口设置该选项，或者在bind()对这个或其他套接口无影响情况下设置或清除这一选项。

  一个应用程序可以通过打开SO_KEEPALIVE选项，使得WINDOWS套接口实现在TCP连接情况下允许使用“保持活动”包。一个WINDOWS套接口实现并不是必需支持“保持活动”，但是如果支持的话，具体的语义将与实现有关，应遵守RFC1122“Internet主机要求－通讯层”中第4.2.3.6节的规范。如果有关连接由于“保持活动”而失效，则进行中的任何对该套接口的调用都将以WSAENETRESET错误返回，后续的任何调用将以WSAENOTCONN错误返回。

  TCP_NODELAY选项禁止Nagle算法。Nagle算法通过将未确认的数据存入缓冲区直到蓄足一个包一起发送的方法，来减少主机发送的零碎小数据包的数目。但对于某些应用来说，这种算法将降低系统性能。所以TCP_NODELAY可用来将此算法关闭。应用程序编写者只有在确切了解它的效果并确实需要的情况下，才设置TCP_NODELAY选项，因为设置后对网络性能有明显的负面影响。TCP_NODELAY是唯一使用IPPROTO_TCP层的选项，其他所有选项都使用SOL_SOCKET层。

  如果设置了SO_DEBUG选项，WINDOWS套接口供应商被鼓励（但不是必需）提供输出相应的调试信息。但产生调试信息的机制以及调试信息的形式已超出本规范的讨论范围。

  setsockopt()支持下列选项。其中“类型”表明optval所指数据的类型。

选项        类型  意义

SO_BROADCAST    BOOL    允许套接口传送广播信息。

SO_DEBUG    BOOL    记录调试信息。

SO_DONTLINER    BOOL    不要因为数据未发送就阻塞关闭操作。设置本选项相当于将SO_LINGER的l_onoff元素置为零。

SO_DONTROUTE    BOOL    禁止选径；直接传送。

SO_KEEPALIVE    BOOL    发送“保持活动”包。

SO_LINGER   struct linger FAR*  如关闭时有未发送数据，则逗留。

SO_OOBINLINE    BOOL    在常规数据流中接收带外数据。

SO_RCVBUF   int 为接收确定缓冲区大小。

SO_REUSEADDR    BOOL    允许套接口和一个已在使用中的地址捆绑（参见bind()）。

SO_SNDBUF   int 指定发送缓冲区大小。

TCP_NODELAY BOOL    禁止发送合并的Nagle算法。

 

  setsockopt()不支持的BSD选项有：

选项名      类型    意义

SO_ACCEPTCONN   BOOL    套接口在监听。

SO_ERROR    int 获取错误状态并清除。

SO_RCVLOWAT int 接收低级水印。

SO_RCVTIMEO int 接收超时。

SO_SNDLOWAT int 发送低级水印。

SO_SNDTIMEO int 发送超时。

SO_TYPE     int 套接口类型。

IP_OPTIONS      在IP头中设置选项。

 

返回值：

  若无错误发生，setsockopt()返回0。否则的话，返回SOCKET_ERROR错误，应用程序可通过WSAGetLastError()获取相应错误代码。

错误代码：

  WSANOTINITIALISED：在使用此API之前应首先成功地调用WSAStartup()。

  WSAENETDOWN：WINDOWS套接口实现检测到网络子系统失效。

  WSAEFAULT：optval不是进程地址空间中的一个有效部分。

  WSAEINPROGRESS：一个阻塞的WINDOWS套接口调用正在运行中。

  WSAEINVAL：level值非法，或optval中的信息非法。

  WSAENETRESET：当SO_KEEPALIVE设置后连接超时。

  WSAENOPROTOOPT：未知或不支持选项。其中，SOCK_STREAM类型的套接口不支持SO_BROADCAST选项，SOCK_DGRAM类型的套接口不支持SO_DONTLINGER 、SO_KEEPALIVE、SO_LINGER和SO_OOBINLINE选项。

  WSAENOTCONN：当设置SO_KEEPALIVE后连接被复位。

  WSAENOTSOCK：描述字不是一个套接口。

  fcntl:
  （1）F_DUPFD

与dup函数功能一样，复制由fd指向的文件描述符，调用成功后返回新的文件描述符，与旧的文件描述符共同指向同一个文件。

（2）F_GETFD

读取文件描述符close-on-exec标志

（3）F_SETFD

将文件描述符close-on-exec标志设置为第三个参数arg的最后一位

（4）F_GETFL

获取文件打开方式的标志，标志值含义与open调用一致，比如只读啊，可写啊什么的

（5）F_SETF

设置文件打开方式为arg指定方式

 

文件记录锁是fcntl函数的主要功能。

记录锁：实现只锁文件的某个部分，并且可以灵活的选择是阻塞方式还是立刻返回方式

当fcntl用于管理文件记录锁的操作时，第三个参数指向一个struct flock *lock的结构体

struct flock
{
    short_l_type;    /*锁的类型*/
    short_l_whence;  /*偏移量的起始位置：SEEK_SET,SEEK_CUR,SEEK_END*/
    off_t_l_start;     /*加锁的起始偏移*/
    off_t_l_len;    /*上锁字节*/
    pid_t_l_pid;   /*锁的属主进程ID */
}; 
 

short_l_type用来指定设置共享锁（F_RDLCK,读锁）还是互斥锁（F_WDLCK,写锁）.

当short_l_type的值为F_UNLCK时，传入函数中将解锁。

每个进程可以在该字节区域上设置不同的读锁。

但给定的字节上只能设置一把写锁，并且写锁存在就不能再设其他任何锁，且该写锁只能被一个进程单独使用。

这是多个进程的情况。

单个进程时，文件的一个区域上只能有一把锁，若该区域已经存在一个锁，再在该区域设置锁时，新锁会覆盖掉旧的锁，无论是写锁还时读锁。

l_whence,l_start,l_len三个变量来确定给文件上锁的区域。

l_whence确定文件内部的位置指针从哪开始，l_star确定从l_whence开始的位置的偏移量，两个变量一起确定了文件内的位置指针先所指的位置，即开始上锁的位置，然后l_len的字节数就确定了上锁的区域。

特殊的，当l_len的值为0时，则表示锁的区域从起点开始直至最大的可能位置，就是从l_whence和l_start两个变量确定的开始位置开始上锁，将开始以后的所有区域都上锁。

为了锁整个文件，我们会把l_whence,l_start,l_len都设为0。

(6)F_SETLK

此时fcntl函数用来设置或释放锁。当short_l_type为F_RDLCK为读锁，F_WDLCK为写锁，F_UNLCK为解锁。

如果锁被其他进程占用，则返回-1;

这种情况设的锁遇到锁被其他进程占用时，会立刻停止进程。

(7)F_SETLKW

此时也是给文件上锁，不同于F_SETLK的是，该上锁是阻塞方式。当希望设置的锁因为其他锁而被阻止设置时，该命令会等待相冲突的锁被释放。

(8)F_GETLK

第3个参数lock指向一个希望设置的锁的属性结构，如果锁能被设置，该命令并不真的设置锁，而是只修改lock的l_type为F_UNLCK,然后返回该结构体。如果存在一个或多个锁与希望设置的锁相互冲突，则fcntl返回其中的一个锁的flock结构。

3.1.4.open函数的flag详解1
3.1.4.1、读写权限：O_RDONLY O_WRONLY O_RDWR
(1)linux中文件有读写权限，我们在open打开文件时也可以附带一定的权限说明（譬如O_RDONLY就表示以只读方式打开，O_WRONLY表示以只写方式打开，O_RDWR表示以可读可写方式打开）
(2)当我们附带了权限后，打开的文件就只能按照这种权限来操作。

3.1.4.2、打开存在并有内容的文件时：O_APPEND、O_TRUNC
(1)思考一个问题：当我们打开一个已经存在并且内部有内容的文件时会怎么样？
可能结果1：新内容会替代原来的内容（原来的内容就不见了，丢了）
可能结果2：新内容添加在前面，原来的内容继续在后面
可能结果3：新内容附加在后面，原来的内容还在前面
可能结果4：不读不写的时候，原来的文件中的内容保持不变
(2)O_TRUNC属性去打开文件时，如果这个文件中本来是有内容的，则原来的内容会被丢弃。这就对应上面的结果1
(3)O_APPEND属性去打开文件时，如果这个文件中本来是有内容的，则新写入的内容会接续到原来内容的后面，对应结果3
(4)默认不使用O_APPEND和O_TRUNC属性时就是结果4
(5)如果O_APPEND和O_TRUNC同时出现会怎么样？

3.1.4.3、exit、_exit、_Exit退出进程
(1)当我们程序在前面步骤操作失败导致后面的操作都没有可能进行下去时，应该在前面的错误监测中结束整个程序，不应该继续让程序运行下去了。
(2)我们如何退出程序？
第一种；在main用return，一般原则是程序正常终止return 0，如果程序异常终止则return -1。
第一种：正式终止进程（程序）应该使用exit或者_exit或者_Exit之一。


3.1.5.open函数的flag详解2
3.1.5.1、打开不存在的文件时：O_CREAT、O_EXCL
(1)思考：当我们去打开一个并不存在的文件时会怎样？当我们open打开一个文件时如果这个文件名不存在则会打开文件错误。
(2)vi或者windows下的notepad++，都可以直接打开一个尚未存在的文件。
(3)open的flag O_CREAT就是为了应对这种打开一个并不存在的文件的。O_CREAT就表示我们当前打开的文件并不存在，我们是要去创建并且打开它。
(4)思考：当我们open使用了O_CREAT，但是文件已经存在的情况下会怎样？结果是报错吗？
(5)结论：open中加入O_CREAT后，不管原来这个文件存在与否都能打开成功，如果原来这个文件不存在则创建一个空的新文件，如果原来这个文件存在则会重新创建这个文件，原来的内容会被消除掉（有点类似于先删除原来的文件再创建一个新的）
(6)这样可能带来一个问题？我们本来是想去创建一个新文件的，但是把文件名搞错了弄成了一个老文件名，结果老文件就被意外修改了。我们希望的效果是：如果我CREAT要创建的是一个已经存在的名字的文件，则给我报错，不要去创建。
(7)这个效果就要靠O_EXCL标志和O_CREAT标志来结合使用。当这连个标志一起的时候，则没有文件时创建文件，有这个文件时会报错提醒我们。
(8)open函数在使用O_CREAT标志去创建文件时，可以使用第三个参数mode来指定要创建的文件的权限。mode使用4个数字来指定权限的，其中后面三个很重要，对应我们要创建的这个文件的权限标志。譬如一般创建一个可读可写不可执行的文件就用0666

3.1.5.2、O_NONBLOCK
(1)阻塞与非阻塞。如果一个函数是阻塞式的，则我们调用这个函数时当前进程有可能被卡住（阻塞住，实质是这个函数内部要完成的事情条件不具备，当前没法做，要等待条件成熟），函数被阻塞住了就不能立刻返回；如果一个函数是非阻塞式的那么我们调用这个函数后一定会立即返回，但是函数有没有完成任务不一定。
(2)阻塞和非阻塞是两种不同的设计思路，并没有好坏。总的来说，阻塞式的结果有保障但是时间没保障；非阻塞式的时间有保障但是结果没保障。
(3)操作系统提供的API和由API封装而成的库函数，有很多本身就是被设计为阻塞式或者非阻塞式的，所以我们应用程度调用这些函数的时候心里得非常清楚。
(4)我们打开一个文件默认就是阻塞式的，如果你希望以非阻塞的方式打开文件，则flag中要加O_NONBLOCK标志。
(2)只用于设备文件，而不用于普通文件。比如说串口、IIC等都是通过文件来访问的，就存在阻塞式和非阻塞式区别。

3.1.5.3、O_SYNC
(1)write阻塞等待底层完成写入才返回到应用层。
(2)无O_SYNC时write只是将内容写入底层缓冲区即可返回，然后底层（操作系统中负责实现open、write这些操作的那些代码，也包含OS中读写硬盘等底层硬件的代码）在合适的时候会将buf中的内容一次性的同步到硬盘中。这种设计是为了提升硬件操作的性能和销量，提升硬件寿命；但是有时候我们希望硬件不要等待，直接将我们的内容写入硬盘中，这时候就可以用O_SYNC标志。

一. 概述
epoll 是 Linux 内核为处理大批量文件描述符而作了改进的 poll，是 Linux 下多路复用 IO接口 select/poll 的增强版本

在 linux 的网络编程中，很长时间都在使用 select 来做事件触发。在 2.6 内核中，有一种替换它的机制，就是 epoll。

select 与 epoll 区别概述
(1) 函数使用上：epoll 使用一组函数来完成任务，而不是单个函数

(2) 效率：select 使用轮询来处理，随着监听 fd 数目的增加而降低效率。而 epoll 把用户关心的文件描述符事件放在内核里的一个事件表中，只需要一个额外的文件描述符来标识内核中的这个事件表即可。

二. epoll 接口
epoll 事件触发并发处理操作过程总共需要三个接口，下面详细说明这三个接口的使用。

头文件
#include <sys/epoll.h>
1
1 int epoll_create(int size);
前面说到 epoll 使用内核事件表来实现 I/O 复用，所以需要一个额外的文件描述符来标识使用的内核事件表。

epoll_create 函数就是用来获取内核事件表的特殊文件描述符，该函数返回的文件描述符将用作其他 epoll 系统调用的第一个参数，以指定要访问的内核事件表。

2 int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
epoll 的事件注册函数，用来操作内核事件表。它不同与 select() 是在监听事件时告诉内核要监听什么类型的事件，而是在这里先注册要监听的事件类型。

参数含义:
1. epfd： 要操作的内核事件表的文件描述符，即 epoll_create 的返回值
2. op：指定操作类型，操作类型有三种：
    -> EPOLL_CTL_ADD：往内核事件表中注册指定fd 相关的事件
    -> EPOLL_CTL_MOD：修改指定 fd 上的注册事件
    -> EPOLL_CTL_DEL：删除指定 fd 的注册事件
3. fd：所要操作的文件描述符，也就是要内核事件表中监听的 fd
4. event：指定所要监听的事件类型，epoll_event 结构指针类型。

struct epoll_even 结构如下：
typedef union epoll_data {
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
 } epoll_data_t;

 struct epoll_event {
    __uint32_t events; /* Epoll events */
    epoll_data_t data; /* User data variable */
};

其中 events 成员描述事件类型，可以是以下几种类型宏的集合：

EPOLLIN：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
EPOLLOUT：表示对应的文件描述符可以写；
EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
EPOLLERR：表示对应的文件描述符发生错误；
EPOLLHUP：表示对应的文件描述符被挂断；
EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
返回值
epoll_ctl 成功时返回 0，失败则返回 -1，并设置 errno

3 int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
等待事件的发生，它在一段超时时间之内等待一组文件描述符上的事件，epoll_wait 函数如果检测到事件，就将所有就绪的事件从内核事件表(epfd 参数决定)中复制到第二个参数 events 指向的数组中。

参数
(1) epfd
要操作的内核事件表的文件描述符，即 epoll_create 的返回值

(2) events
内核事件表中得到的检测事件集合

(3) maxevents & timeout
maxevents 告诉内核 events 的最大 size，timeout 指定超时时间

返回值
成功时返回就绪的文件描述符的个数，失败返回 -1 并设置 errno

三. epoll 工作模式
epoll 对文件描述符的操作有两种模式：LT（level trigger）和 ET（edge trigger）。LT 模式是默认模式，LT 模式与 ET 模式的区别如下：

　　LT模式：电平触发，当 epoll_wait 检测到描述符事件发生并将此事件通知应用程序，应用程序可以不立即处理该事件。下次调用 epoll_wait 时，会再次响应应用程序并通知此事件。

　　ET模式：边沿触发，当 epoll_wait 检测到描述符事件发生并将此事件通知应用程序，应用程序必须立即处理该事件。如果不处理，下次调用 epoll_wait 时，不会再次响应应用程序并通知此事件。

　　ET 模式在很大程度上减少了 epoll 事件被重复触发的次数，因此效率要比 LT 模式高。epoll 工作在 ET 模式的时候，必须使用非阻塞套接口，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死。

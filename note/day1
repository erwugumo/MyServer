socket 中存储了特定的四元组： 源ip+port，目的ip+port；
1> bind 到特定 ip 和 port 的socket 对应 [src ip, src port) <=> (*, *)] ；
2> connect 到特定目的ip+port 的 socket 对应 [src ip, src port) <=> (dst ip,  dst port)]；
3> accept 返回了的 socket 对应  [src ip, src port) <=> (dst ip,  dst port)]；

在使用socket编程时，我们都知道在网络通信以前首先要建立连接，而连接的建立是通过对socket的一些操作来完成的。
那么，建立连接的过程大致可以分为以下几步：
1） 建立socket套接字。
2） 给套接字赋予地址，这个地址不是通常的网络地址的概念。
3） 建立socket连接。

以下详细解释
1． 建立socket套接字。
使用socket建立套接字的时候，我们实际上是建立了一个数据结构。
这个数据结构最主要的信息是指定了连接的种类和使用的协议，此外还有一些关于连接队列操作的结构字段(这里就先不涉及他们了）。
当我们使用socket函数以后，如果成功的话会返回一个int型的描述符，它指向前面那个被维护在内核里的socket数据结构。
我们的任何操作都是通过这个描述符而作用到那个数据结构上的。
这就像是我们在建立一个文件后得到一个文件描述符一样，对文件的操作都是通过文件描述符来进行的，而不是直接作用到inode数据结构上。
我之所以用文件描述符举例，是因为socket数据结构也是和inode数据结构密切相关，它不是独立存在于内核中的，而是位于一个VFS inode结构中。
所以，有一些比较抽象的特性，我们可以用文件操作来不恰当的进行类比以加深理解。
如前所述，当建立了这个套接字以后，我们可以获得一个象文件描述符那样的套接字描述符。
就象我们对文件进行操作那样，我们可以通过向套接字里面写数据将数据传送到我们指定的地方，
这个地方可以是远端的主机，也可以是本地的主机。
如果你有兴趣的话，还可以用socket机制来实现IPC，不过效率比较低，试试也就行了（没有试过）。

2． 给套接字赋予地址。
依照建立套接字的目的不同，赋予套接字地址的方式有两种：服务器端使用bind，客户端使用connect。
Bind:
我们都知道，只要使用IP, prot就可以区分一个tcp/ip连接（当然这个连接指的是一个连接通道，如果要区分特定的主机间的连接，
还需要第三个属性 hostname）。
我们可以使用bind函数来为一个使用在服务器端例程中的套接字赋予通信的地址和端口。
在这里我们称通信的IP地址和端口合起来构成了一个socket地址，而指定一个socket使用特定的IP和port组合来进行通行的过程就是
赋予这个socket一个地址。 要赋予socket地址，就得使用一个数据结构来指明特定的socket地址，这个数据结构就是struct sockaddr。
对它的使用我就不说了，因为这篇文档的目的是澄清概念而不是说明使用方法。
Bind函数的作用就是将这个特定的标注有socket地址信息的数据结构和socket套接字联系起来，即赋予这个套接字一个地址。
但是在具体实现上，他们两个是怎么联系在一起的，我还不知道。
一个特定的socket的地址的生命期是bind成功以后到连接断开前。
你可以建立一个socket数据结构和socket地址的数据结构，但是在没有bind以前他们两个是没有关系的，在bind以后他们两个才有了关系。
这种关系一直维持到连接的结束，当一个连接结束时，socket数据结构和socket地址的数据结构还都存在，但是他们两个已经没有关系了。
如果你要是用这个套接字在socket地址上重新进行连接时，需重新bind他们两个。
再注明一次，我说的这个连接是一个连接通道，而不是特定的主机之间的连接。
Bind指定的IP通常是本地IP（一般不特别指定，而使用INADDR_ANY来声明），而最主要的作用是指定端口。
在服务器端的socket进行了bind以后就是用listen来在这个socket地址上准备进行连接。
connect:
对于客户端来说，是不会使用bind的（并不是不能用，但没什么意义），他们会通过connet函数来建立socket和socket地址之间的关系。
其中的socket地址是它想要连接的服务器端的socket地址。在connect建立socket和socket地址两者关系的同时，它也在尝试着建立远端的连接。

3． 建立socket连接。
对于准备建立一个连接，服务器端要两个步骤：bind, listen；客户端一个步骤：connct。
如果服务器端accept一个connect，而客户端得到了这个accept的确认，那么一个连接就建立了。

Socket一般应用模式(服务器端和客户端)

服务器端的Socket(至少需要两个)

01.一个负责接收客户端连接请求(但不负责与客户端通信)

02.每成功接收到客户端的连接便在服务器端产生一个对应的复杂通信的Socket
    021.在接收到客户端连接时创建
    022. 为每个连接成功的客户端请求在服务器端都创建一个对应的Socket(负责和客户端通信)

客户端的Socket

必须指定要连接的服务器地址和端口
通过创建一个Socket对象来初始化一个到服务器端的TCP连接
 

      通过上图，我们可以看出，首先服务器会创建一个负责监听的socket，然后客户端通过socket连接到服务器指定端口，

最后服务器端负责监听的socket，监听到客户端有连接过来了，就创建一个负责和客户端通信的socket。

下面我们来看下Socket更具体的通信过程：

Socket的通讯过程

  服务器端：

    01，申请一个socket

    02，绑定到一个IP地址和一个端口上

    03，开启侦听，等待接收连接

  客户端：

    01，申请一个socket

   02，连接服务器(指明IP地址和端口号)

   服务器端接收到连接请求后，产生一个新的socket(端口大于1024)与客户端建立连接并进行通信，原监听socket继续监听。

  注意：负责通信的Socket不能无限创建，创建的数量和操作系统有关。

 7.Socket的构造函数

    Public Socket(AddressFamily addressFamily,SocketType  socketType,ProtocolType  protocolTYpe)

    AddressFamily：指定Socket用来解析地址的寻址方案。例如：InterNetWork指示当Socket使用一个IP版本4地址连接

   SocketType：定义要打开的Socket的类型

   Socket类使用ProtocolType枚举向Windows  Sockets  API通知所请求的协议

注意：

   1，端口号必须在 1 和 65535之间，最好在1024以后。

   2，要连接的远程主机必须正在监听指定端口，也就是说你无法随意连接远程主机。

如：

IPAddress addr = IPAddress.Parse("127.0.0.1");

IPEndPoint endp = new IPEndPoint(addr,,9000);

         服务端先绑定：serverWelcomeSocket.Bind(endp)

         客户端再连接：clientSocket.Connect(endp)

   3,一个Socket一次只能连接一台主机

   4,Socket关闭后无法再次使用

  5,每个Socket对象只能与一台远程主机连接。如果你想连接到多台远程主机，你必须创建多个Socket对象。

8.Socket常用类和方法

  相关类：

   IPAddress：包含了一个IP地址

   IPEndPoint：包含了一对IP地址和端口号

 方法：

   Socket():创建一个Socket

   Bind():绑定一个本地的IP和端口号（IPEndPoint）

   Listen():让Socket侦听传入的连接吃那个病，并指定侦听队列容量

   Connect():初始化与另一个Socket的连接

   Accept（）：接收连接并返回一个新的Socket
   Send（）：输出数据到Socket
   Receive():从Socket中读取数据
   Close():关闭Socket，销毁连接

结构体大小计算
(1)设结构体成员对齐值为ZP
(2)设结构体当前数据成员对齐值zp=min(当前数据成员类型大小,ZP)
(3)设结构体自身对齐值stAlign=min(max(数据成员1类型大小,.....数据成员n类型大小),ZP)
(4)设置结构体某成员距离结构体首地址的偏移为offset
(5)每个成员的位置偏移(也就是offset)要对zp取余,如果余数不为0,则要调整位置偏移,
在大于当前偏移值中找一个最小的位置偏移,使之能够对zp取余且余数为0,最后结构体
的总大小要对stAlign取余,如果余数不为0,采用相同的方法调整结构体大小

注意:如果结构体中有成员为数组,例如:

     struct tagTest
     {
         char m_ch;
         int  m_nAry[10];
     };
    那么m_nAry的数据类型大小为4字节,而不是40字节

    如果结构体A中有另一个结构体B作为作为结构体A的数据成员,例如:
     #pragma pack(push,8)
     struct B
     {
         char m_ch;
         int  m_nAry[10];
     };

     struct A
     {
         B    m_b;
         char m_ch;
         int  m_nAry[10];
     };
     #pragma pack(pop)
    那么在结构体A中m_b的对齐值为为结构体B的自身对齐值,也就是4,在计算结构体A  
    的自身对齐值时,并不是将sizeof(m_b)参与计算,而是取结构体B中宽度最大的基本  
    数据类型所占字节参与计算,例如结构体B中宽度最大的基本数据类型为int,也就是  
    4字节,所以结构体A的自身对齐值为min(max(4,1,4),8)=4

例1:

   #pragma pack(push,8)
   typedef struct tagTestA
   {
     char m_chTest;
     int m_nTest;
     float m_fTest;
     char m_chAry[13];
   };
  #pragma pack(pop)
结构体成员对齐值为ZP=8,结构体自身对齐值stAlign为:
min(max(sizeof(char),sizeof(int),sizeof(int),sizeof(float),sizeof(char)),ZP)=4

m_chTest的对齐值zp=min(sizeof(char),ZP)=1,m_chTest为结构体第一个成员,所以其相对结
构体首地址偏移量offset=0,offset对zp取余结果为0,所以无需调整,则下一个成员m_nTest相对
于结构体首地址的偏移为offset+sizeof(m_chTest)=1

m_nTest的对齐值zp=min(sizeof(int),ZP)=4,m_nTest相对首地址的偏移量offset=1,
offset%zp=1,取余结果不为0,当offset调整为4时,取余结果为0,则下一个成员m_fTest相对于
结构体首地址的偏移为offset+sizeof(m_nTest)=8

m_fTest的对齐值zp=min(sizeof(float),ZP)=4,m_nTest相对首地址的偏移量offset=8,
offset%zp=0,取余结果为0,无需调整,则下一个成员m_chAry相对于结构体首地址的偏移为
offset+sizeof(m_fTest)=12

m_chAry的对齐值zp=min(sizeof(char),ZP)=1,m_chAry相对首地址的偏移量offset=12,
offset%zp=0,无需调整,至此结构体tagTestA最后一个成员的偏移计算完毕,结构体大小为
offset+sizeof(m_chAry)=25, 25%stAlign != 0,所以结构体总大小需要调整,那么大于25且能
够对4取余为0的最小值为28,所以结构体大小为28,那么下面来验证下计算结果:
结构体大小以及偏移计算1.png

测试代码中计算结构体偏移使用了宏函数offsetof,其定义在stddef.h中:

#if defined(_MSC_VER) && !defined(_CRT_USE_BUILTIN_OFFSETOF)
  #ifdef __cplusplus
    #define offsetof(s,m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
  #else
    #define offsetof(s,m) ((size_t)&(((s*)0)->m))
  #endif
#else
  #define offsetof(s,m) __builtin_offsetof(s,m)
#endif
((size_t)&(((s*)0)->m)):(s*)0先是将0地址强行解释为s类型的指针,然后(s*)0)->m引用结构体的
成员m,&(((s*)0)->m))取得成员m的内存地址,因为该指针指向0地址,所
以强转后就得到成员m相对于结构体首地址的偏移,虽然是指向0地址的指针
但是并未使用该指针对结构体成员进行赋值和取值操作,所以不会引发崩溃。

例2:

  #pragma pack(push,8)
  typedef struct tagTestA
  {
  char m_chTest;
  int m_nTest;
  float m_fTest;
  char m_chAry[13];
  };
  #pragma pack(pop)


  #pragma pack(push,4)
  struct tagTestB
  {
  double m_dbTest1;
  char   m_chTest2;
  tagTestA m_stA;
  char   m_ch[7];
  };
  #pragma pack(pop)
结构体tagTestB的计算有点复杂,首先它的结构体成员对齐值设置为4,其次,它的一个数据成员为结构体tagTestA类型,下面开始计算:

根据例1中的计算结构体tagTestA的自身对齐值为4,tagTestA的的大小为28;
tagTestB的成员对齐值ZP=4
tagTestB的自身对齐值stAlign=min(max(sizeof(double),sizeof(char),4,sizeof(char)),ZP)=4

m_dbTest1为结构体第一个成员,距离结构体的首地址偏移offset=0,m_dbTest1的自身对齐值
zp=min(sizeof(double),ZP)=4,offset%zp=0,所以无需调整offset,那么下一个成员m_chTest2
相对于结构体首地址的偏移为offset+sizeof(double)=8

根据上一步,m_chTest2距离结构体首地址的偏移offset=8,m_chTest2的自身对齐值
zp=min(sizeof(char),ZP)=1,offset%zp=0,所以offset无需调整,则下一个数据成员m_stA
距离结构体首地址的偏移为offset+sizeof(char)=9

根据上一步,m_stA距离结构体首地址的偏移offset=9,m_stA的自身对齐值zp=min(4,ZP)=4,
offset%zp!=0,offset需要调整,将offset调整为12时满足offset%zp=0,则下一个成员m_ch
距离结构体首地址的偏移为offset+sizeof(m_ch)=40

根据上一步,m_ch距离结构体首地址的偏移offset=40,m_ch的自身对齐值为zp=min(sizeof(char),ZP)=1,
offset%zp=0,所以offset无需调整,则结构体tagTestB的大小为offset+sizeof(m_ch)=47,
且47%stAlign!=0,所以结构体大小需要调整,当结构体大小为48时,满足要求

通过以上计算可以得出m_dbTest1的偏移为0,m_chTest2的偏移为8,m_stA的偏移为12,m_ch的偏移为40,
结构体总大小为48,下面在VS中验证结果:
结构体大小以及偏移计算2.png

注意事项:

以上是VC++编译器根据不同结构体成员对齐值实现内存布局的解析,其他编译器的实现可能有所不同

当结构体成员对齐值为1时,表示结构体成员不进行内存对齐,各个成员紧紧相邻,不会有填充字节,
那么此时结构体大小为sizeof(member1)+..........sizeof(membern)


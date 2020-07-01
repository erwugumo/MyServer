#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include "helper.h"           /*  our own helper functions  */

#include <stdlib.h>
#include <stdio.h>


/*  Global constants  */

#define ECHO_PORT          (2002)
#define MAX_LINE           (1000)

/*
    argc是参数个数，argv是所有参数。argv[0]为文件或者指令名。
    这里可以加一个参数port。
*/
int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */


    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */

    if ( argc == 2 ) {
	port = strtol(argv[1], &endptr, 0);
    /*long int strtol(const char *str, char **endptr, int base)
    str -- 要转换为长整数的字符串。
    endptr -- 对类型为 char* 的对象的引用，其值由函数设置为 str 中数值后的下一个字符。
    base -- 基数，必须介于 2 和 36（包含）之间，或者是特殊值 0，base就是几进制的合法值。比如base=2，合法值就是1,2。
    当字符合法时，‘0’，……‘9’依次被转换为十进制的0～9，‘a’，……‘z’一次北转换为十进制的10～35。
    strtol()函数检测到第一个非法字符时，立即停止检测，其后的所有字符都会被当作非法字符处理。
    合法字符串会被转换为long int, 作为函数的返回值。
    非法字符串，即从第一个非法字符的地址，被赋给*endptr。**endptr是个双重指针，即指针的指针。
    strtol()函数就是通过它改变*endptr的值，即把第一个非法字符的地址传给endptr。
    */
	if ( *endptr ) {
	    fprintf(stderr, "ECHOSERV: Invalid port number.\n");
	    exit(EXIT_FAILURE);
	}
    }
    else if ( argc < 2 ) {
	port = ECHO_PORT;
    }
    else {
	fprintf(stderr, "ECHOSERV: Invalid arguments.\n");
	exit(EXIT_FAILURE);
    }

	
    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
	exit(EXIT_FAILURE);
    }
    /*
    函数原型：
    int socket(int domain, int type, int protocol);
    参数说明：

    domain：协议域，又称协议族（family）。常用的协议族有AF_INET、AF_INET6、AF_LOCAL（或称AF_UNIX，Unix域Socket）、AF_ROUTE等。
    协议族决定了socket的地址类型，在通信中必须采用对应的地址，如AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合、AF_UNIX决定了要用一个绝对路径名作为地址。
    type：指定Socket类型。常用的socket类型有SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等。
    流式Socket（SOCK_STREAM）是一种面向连接的Socket，针对于面向连接的TCP服务应用。
    数据报式Socket（SOCK_DGRAM）是一种无连接的Socket，对应于无连接的UDP服务应用。
    protocol：指定协议。常用协议有IPPROTO_TCP、IPPROTO_UDP、IPPROTO_STCP、IPPROTO_TIPC等，
    分别对应TCP传输协议、UDP传输协议、STCP传输协议、TIPC传输协议。
    注意：
    1.type和protocol不可以随意组合，如SOCK_STREAM不可以跟IPPROTO_UDP组合。当第三个参数为0时，会自动选择第二个参数类型对应的默认协议。
    2.WindowsSocket下protocol参数中不存在IPPROTO_STCP
    返回值：
    如果调用成功就返回新创建的套接字的描述符，如果失败就返回INVALID_SOCKET（Linux下失败返回-1）。
    套接字描述符是一个整数类型的值。每个进程的进程空间里都有一个套接字描述符表，该表中存放着套接字描述符和套接字数据结构的对应关系。
    该表中有一个字段存放新创建的套接字的描述符，另一个字段存放套接字数据结构的地址，因此根据套接字描述符就可以找到其对应的套接字数据结构。
    每个进程在自己的进程空间里都有一个套接字描述符表但是套接字数据结构都是在操作系统的内核缓冲里。
    */

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);
    /*在C/C++写网络程序的时候，往往会遇到字节的网络顺序和主机顺序的问题。
    这就可能用到htons(), ntohl(), ntohs()，htons()这4个函数。
    网络字节顺序与本地字节顺序之间的转换函数：

        htonl()--"Host to Network Long"
        ntohl()--"Network to Host Long"
        htons()--"Host to Network Short"
        ntohs()--"Network to Host Short"

    之所以需要这些函数是因为计算机数据表示存在两种字节顺序：NBO与HBO

    网络字节顺序NBO(Network Byte Order): 按从高到低的顺序存储，在网络上使用统一的网络字节顺序，可以避免兼容性问题。

    主机字节顺序(HBO，Host Byte Order): 不同的机器HBO不相同，与CPU设计有关，数据的顺序是由cpu决定的,而与操作系统无关。
    如 Intel x86结构下, short型数0x1234表示为34 12, int型数0x12345678表示为78 56 34 12  
    如 IBM power PC结构下, short型数0x1234表示为12 34, int型数0x12345678表示为12 34 56 78
   
    由于这个原因不同体系结构的机器之间无法通信,所以要转换成一种约定的数序,也就是网络字节顺序,其实就是如同power pc那样的顺序. 
    */


    /*  Bind our socket addresss to the 
	listening socket, and call listen()  */

    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error calling bind()\n");
	exit(EXIT_FAILURE);
    }
    /*函数原型：
    int bind(SOCKET socket, const struct sockaddr* address, socklen_t address_len);
    参数说明：

    socket：是一个套接字描述符。
    address：是一个sockaddr结构指针，该结构中包含了要结合的地址和端口号。
    address_len：确定address缓冲区的长度。
    返回值：
    如果函数执行成功，返回值为0，否则为SOCKET_ERROR。
    */

    if ( listen(list_s, LISTENQ) < 0 ) {
	fprintf(stderr, "ECHOSERV: Error calling listen()\n");
	exit(EXIT_FAILURE);
    }
    /* int listen(int sockfd, int backlog);
    listen函数把一个未连接的套接字转换成一个被动套接字，指示内核应该接受指向该套接字的连接请求。
    调用listen导致套接字从CLOSED状态转换到LISTEN状态。
    listen函数的第二个参数规定了内核应为相应套接字排队的最大连接个数。
    为了理解其中的backlog参数，我们必须认识到内核为任何一个给定的监听套接字维护两个队列：
    （1）未完成连接队列，每个这样的SYN分节对应其中的一项：已由某个客户发出并到达服务器，
    而服务器正在等待完成相应的TCP三路握手过程。这些套接字处于SYN_RCVD状态。
    （2）已完成连接队列，每个已完成TCP三路握手过程的客户对应其中一项。这些套接字处于ESTABLISHED状态。
    每当在未完成连接队列中创建一项时，来自监听套接字的参数就复制到即将建立的连接中。连接的创建机制是完全自动的，无需服务器进程插手。
    当来自客户的SYN到达时，TCP在未完成连接队列中创建一个新项，然后响应三路握手的第二个分节：服务器的SYN响应，其中捎带对客户SYN的ACK。
    这一项一直保留在未完成连接队列中，直到三路握手的第三个节点到达或者该项超时为止。
    如果三次握手正常完成，该项就从未完成连接队列移到已完成连接队列的队尾。
    当进程调用accept时，已完成连接队列中的对头将返回给进程，或者如果队列为空，那么进程将被投入睡眠，直到TCP在该队列中放入一项才唤醒它。

    */
    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */


    while ( 1 ) {

	/*  Wait for a connection, then accept() it  */

	if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
	    fprintf(stderr, "ECHOSERV: Error calling accept()\n");
	    exit(EXIT_FAILURE);
	}
    /*int accept( int fd, struct socketaddr* addr, socklen_t* len);
    fd：套接字描述符。
    addr：返回连接着的地址
    len：接收返回地址的缓冲区长度
    返回值：成功返回客户端的文件描述符，失败返回-1。
    返回值是一个新的套接字描述符，它代表的是和客户端的新的连接，
    可以把它理解成是一个客户端的socket,这个socket包含的是客户端的ip和port信息 。
    （当然这个new_socket会从sockfd中继承 服务器的ip和port信息，两种都有了），
    而参数中的int fd包含的是服务器的ip和port信息 。
    */

	/*  Retrieve an input line from the connected socket
	    then simply write it back to the same socket.     */

	Readline(conn_s, buffer, MAX_LINE-1);
	Writeline(conn_s, buffer, strlen(buffer));


	/*  Close the connected socket  */

	if ( close(conn_s) < 0 ) {
	    fprintf(stderr, "ECHOSERV: Error calling close()\n");
	    exit(EXIT_FAILURE);
	}
    }
}

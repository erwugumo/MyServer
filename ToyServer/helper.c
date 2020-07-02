#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	/*ssize_t read(int fd, void * buf, size_t count);
	函数说明：read()会把参数fd所指的文件传送count个字节到buf指针所指的内存中。
	返回值：返回值为实际读取到的字节数, 如果返回0, 表示已到达文件尾或是无可读取的数据。若参数count为0, 则read()不会有作用并返回0。
	注意：read时fd中的数据如果小于要读取的数据，就会引起阻塞。*/
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	/*
	如果read（）读到数据为0，那么就表示文件读完了，如果在读的过程中遇到了中断则read()应该返回-1，同时置errno为EINTR。
	*/
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 ) {
	/*函数定义：ssize_t write (int fd, const void * buf, size_t count); 
	函数说明：write()会把参数buf所指的内存写入count个字节到所指的文件fd内。也就是写入socket的写缓冲。
	返回值：如果顺利write()会返回实际写入的字节数。当有错误发生时则返回-1，错误代码存入errno中。
	write()函数从buf写数据到fd中时，若buf中数据无法一次性读完，那么第二次读buf中数据时，
	其读位置指针（也就是第二个参数buf）不会自动移动，需要程序员编程控制。
	*/
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	/*如果 write（）返回0，那么就表示出错，也就是无法写入了;
	而如果在写的过程中遇到了中断，那么write（）会返回-1，同时置errno为EINTR。
	*/
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		return -1;
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}
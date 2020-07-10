
/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include "dbg.h"

int open_listenfd(int port) 
{
    if (port <= 0) {
        port = 3000;
    }

    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1;
    /*closesocket（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket：

    BOOL bReuseaddr=TRUE;

    setsockopt(s,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL));*/
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
		   (const void *)&optval , sizeof(int)) < 0)
	    return -1;
    /*
    setsockopt( int socket, int level, int option_name,const void *option_value, size_t option_len);  
    第一个参数socket是套接字描述符。

    第二个参数level是被设置的选项的级别，如果想要在套接字级别上设置选项，就必须把level设置为SOL_SOCKET。

    第三个参数 option_name指定准备设置的选项，option_name可以有哪些常用取值，这取决于level，以linux 2.6内核为例（在不同的平台上，这种关系可能会有不同）
    */
    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    /*置字节字符串s的前n个字节为零。*/
    /*bzero()好记忆：2个参数；
    memset()易出错：3个参数，且第二、三个参数易记混淆，如若出现位置互换的情况，C编译器并不能察觉。*/
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	    return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
	    return -1;

    return listenfd;
}

/*
    非阻塞make a socket non blocking. If a listen socket is a blocking socket, after it comes out from epoll and accepts the last connection, the next accpet will block, which is not what we want
*/
int make_socket_non_blocking(int fd) {
    int flags, s;
    /*fcntl系统调用可以用来对已打开的文件描述符进行各种控制操作以改变已打开文件的的各种属性*/
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        log_err("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1) {
        log_err("fcntl");
        return -1;
    }

    return 0;
}

/*
* Read configuration file
* TODO: trim input line
*/
int read_conf(char *filename, zv_conf_t *cf, char *buf, int len) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_err("cannot open config file: %s", filename);
        return ZV_CONF_ERROR;
    }

    int pos = 0;
    char *delim_pos;
    int line_len;
    char *cur_pos = buf+pos;
    /*char *fgets(char *str, int n, FILE *stream);
    从指定的流 stream 读取一行，并把它存储在 str 所指向的字符串内。
    当读取 (n-1) 个字符时，或者读取到换行符时，或者到达文件末尾时，它会停止，*/
    while (fgets(cur_pos, len-pos, fp)) {
        /*strstr(str1,str2) 函数用于判断字符串str2是否是str1的子串。
        如果是，则该函数返回 str1字符串从 str2第一次出现的位置开始到 str1结尾的字符串；否则，返回NULL。*/
        delim_pos = strstr(cur_pos, DELIM);
        line_len = strlen(cur_pos);
        
        /*
        debug("read one line from conf: %s, len = %d", cur_pos, line_len);
        */
        if (!delim_pos)
            return ZV_CONF_ERROR;
        
        if (cur_pos[strlen(cur_pos) - 1] == '\n') {
            cur_pos[strlen(cur_pos) - 1] = '\0';
        }
        /*int strncmp ( const char * str1, const char * str2, size_t n );
        功能是把 str1 和 str2 进行比较，最多比较前 n 个字节，
        若str1与str2的前n个字符相同，则返回0；
        若s1大于s2，则返回大于0的值；若s1 小于s2，则返回小于0的值*/
        if (strncmp("root", cur_pos, 4) == 0) {
            cf->root = delim_pos + 1;
        }

        if (strncmp("port", cur_pos, 4) == 0) {
            cf->port = atoi(delim_pos + 1);     
        }

        if (strncmp("threadnum", cur_pos, 9) == 0) {
            cf->thread_num = atoi(delim_pos + 1);
        }

        cur_pos += line_len;
    }

    fclose(fp);
    return ZV_CONF_OK;
}

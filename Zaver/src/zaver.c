
/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */

#include <stdint.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "util.h"
#include "timer.h"
#include "http.h"
#include "epoll.h"
#include "threadpool.h"

#define CONF "zaver.conf"
#define PROGRAM_VERSION "0.1"
/*extern可以置于变量或者函数前，以标示变量或者函数的定义在别的文件中，
提示编译器遇到此变量和函数时在其他模块中寻找其定义。此外extern也可用来进行链接指定。*/
extern struct epoll_event *events;
/*const规定了一个变量在它初始化值之后，值不能再改变，也就是只读
static在函数内的时候，表明这个变量在函数的生命周期结束之后也不会被释放
static在函数外的时候，表明这个变量的作用域只在该.c文件里，不能作用于整个工程
函数外的全局变量若想在其他文件使用需要extern
struct option {
const char *name; //name表示的是长参数名
int has_arg；
//has_arg有3个值，no_argument(或者是0)，表示该参数后面不跟参数值
// required_argument(或者是1),表示该参数后面一定要跟个参数值
// optional_argument(或者是2),表示该参数后面可以跟，也可以不跟参数值
int *flag;    
//用来决定，getopt_long()的返回值到底是什么。
//如果这个指针为NULL，那么getopt_long()返回该结构val字段中的数值。
//如果该指针不为NULL，getopt_long()会使得它所指向的变量中填入val字段中的数值，并且getopt_long()返回0。
//如果flag不是NULL，但未发现长选项，那么它所指向的变量的数值不变。
int val; 
//和flag联合决定返回值 这个值是发现了长选项时的返回值，或者flag不是 NULL时载入*flag中的值。
//典型情况下，若flag不是NULL，那么val是个真／假值，譬如1 或0；另一方面，如 果flag是NULL，那么val通常是字符常量，
//若长选项与短选项一致，那么该字符常量应该与optstring中出现的这个选项的参数相同。
*/
static const struct option long_options[]=
{
    {"help",no_argument,NULL,'?'},
    {"version",no_argument,NULL,'V'},
    {"conf",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};

static void usage() {
   fprintf(stderr,
	"zaver [option]... \n"
	"  -c|--conf <config file>  Specify config file. Default ./zaver.conf.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
}

int main(int argc, char* argv[]) {
    int rc;
    int opt = 0;
    int options_index = 0;
    char *conf_file = CONF;

    /*
    * parse argv 
    * more detail visit: http://www.gnu.org/software/libc/manual/html_node/Getopt.html
    */

    if (argc == 1) {
        usage();
        return 0;
    }
    /*int getopt_long(int argc, char* cons4targv[], 
                     const char*optstring,
                     const struct option*longopts, 
                     int*longindex);
    argc、argv直接从main函数中获取。

    optionstring是选项参数组成的字符串，由下列元素组成：
    （1）单个字符，表示选项，
    （2）单个字符后接一个冒号：表示该选项后必须跟一个参数。参数紧跟在选项后或者以空格隔开。该参数的指针赋给optarg。
    （3）单个字符后跟两个冒号，表示该选项后可以有参数也可以没有参数。如果有参数，参数必须紧跟在选项后不能以空格隔开。
    该参数的指针赋给optarg。（这个特性是GNU的扩张）。

    全局变量：
    （1）optarg：表示当前选项对应的参数值。
    （2）optind：表示的是下一个将被处理到的参数在argv中的下标值。
    （3）opterr：如果opterr = 0，在getopt、getopt_long、getopt_long_only遇到错误将不会输出错误信息到标准输出流。
         opterr在非0时，向屏幕输出错误。
    （4）optopt：表示没有被未标识的选项。
    返回值：
    （1）如果短选项找到，那么将返回短选项对应的字符。
    （2）如果长选项找到，如果flag为NULL，返回val。如果flag不为空，返回0
    （3）如果遇到一个选项没有在短字符、长字符里面。或者在长字符里面存在二义性的，返回“？”
    （4）如果解析完所有字符没有找到（一般是输入命令参数格式错误，eg： 连斜杠都没有加的选项），返回“-1”
    （5）如果选项需要参数，忘了添加参数。返回值取决于optstring，如果其第一个字符是“：”，则返回“：”，否则返回“？”。
    注意：
    （1）longopts的最后一个元素必须是全0填充，否则会报段错误
    （2）短选项中每个选项都是唯一的。而长选项如果简写，也需要保持唯一性。
    */
    while ((opt=getopt_long(argc, argv,"Vc:?h",long_options,&options_index)) != EOF) {
        switch (opt) {
            case  0 : break;
            case 'c':
                conf_file = optarg;
                break;
            case 'V':
                printf(PROGRAM_VERSION"\n");
                return 0;
            case ':':
            case 'h':
            case '?':
                usage();
                return 0;
        }
    }

    debug("conffile = %s", conf_file);

    if (optind < argc) {
        log_err("non-option ARGV-elements: ");
        while (optind < argc)
            log_err("%s ", argv[optind++]);
        return 0;
    }

    /*
    * read confile file
    */
    char conf_buf[BUFLEN];
    zv_conf_t cf;
    rc = read_conf(conf_file, &cf, conf_buf, BUFLEN);
    check(rc == ZV_CONF_OK, "read conf err");

    /*
    *   install signal handle for SIGPIPE
    *   when a fd is closed by remote, writing to this fd will cause system send
    *   SIGPIPE to this process, which exit the program
    */
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_err("install sigal handler for SIGPIPE failed");
        return 0;
    }
    /*SIGPIPE信号产生的原因：
    简单来说，就是客户端程序向服务器端程序发送了消息，然后关闭客户端，服务器端返回消息的时候就会收到内核给的SIGPIPE信号。
    TCP的全双工信道其实是两条单工信道，client端调用close的时候，虽然本意是关闭两条信道，
    但是其实只能关闭它发送的那一条单工信道，还是可以接受数据，server端还是可以发送数据，并不知道client端已经完全关闭了。
    以下为引用：
    对一个已经收到FIN包的socket调用read方法, 如果接收缓冲已空, 则返回0, 这就是常说的表示连接关闭. 
    但第一次对其调用write方法时, 如果发送缓冲没问题, 会返回正确写入(发送). 
    但发送的报文会导致对端发送RST报文, 因为对端的socket已经调用了close, 完全关闭, 既不发送, 也不接收数据. 
    所以, 第二次调用write方法(假设在收到RST之后), 会生成SIGPIPE信号, 导致进程退出.
    * initialize listening socket
    */
    int listenfd;
    struct sockaddr_in clientaddr;
    // initialize clientaddr and inlen to solve "accept Invalid argument" bug
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));  
    /*open_listenfd函数实现socket绑定以及listen*/
    listenfd = open_listenfd(cf.port);
    rc = make_socket_non_blocking(listenfd);
    check(rc == 0, "make_socket_non_blocking");

    /*
    * create epoll and add listenfd to ep
    */
    int epfd = zv_epoll_create(0);
    struct epoll_event event;
    /*http报头初始化*/
    zv_http_request_t *request = (zv_http_request_t *)malloc(sizeof(zv_http_request_t));
    zv_init_request_t(request, listenfd, epfd, &cf);
    /*其中events表示感兴趣的事件和被触发的事件，可能的取值为：
    EPOLLIN：表示对应的文件描述符可以读；
    EPOLLOUT：表示对应的文件描述符可以写；
    EPOLLPRI：表示对应的文件描述符有紧急的数可读；

    EPOLLERR：表示对应的文件描述符发生错误；
    EPOLLHUP：表示对应的文件描述符被挂断；
    EPOLLET： ET的epoll工作模式；*/
    event.data.ptr = (void *)request;
    
    event.events = EPOLLIN | EPOLLET;
    /* 注册listen事件 */
    zv_epoll_add(epfd, listenfd, &event);

    /*
    * create thread pool
    */
    /*
    zv_threadpool_t *tp = threadpool_init(cf.thread_num);
    check(tp != NULL, "threadpool_init error");
    */
    
    /*
     * initialize timer
     */
    zv_timer_init();

    log_info("zaver started.");
    int n;
    int i, fd;
    int time;

    /* epoll_wait loop */
    while (1) {
        time = zv_find_timer();//查找最早的连接，它最先回超时，time为它超时的剩余时间
        debug("wait time = %d", time);
        n = zv_epoll_wait(epfd, events, MAXEVENTS, time);//保证所有已连接的连接在最大时间内连接成功的话都不会被断开
        zv_handle_expire_timers();//处理所有超时连接
        
        for (i = 0; i < n; i++) {
            zv_http_request_t *r = (zv_http_request_t *)events[i].data.ptr;
            fd = r->fd;
            
            if (listenfd == fd) {
                //监听端口动了，说明有新连接到了
                /* we hava one or more incoming connections */

                int infd;
                while(1) {
                    infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                    //从listenfd中得到新连接的IP和端口
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* we have processed all incoming connections */
                            break;
                        } else {
                            log_err("accept");
                            break;
                        }
                    }

                    rc = make_socket_non_blocking(infd);
                    check(rc == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);
                    //为新连接建立结构
                    zv_http_request_t *request = (zv_http_request_t *)malloc(sizeof(zv_http_request_t));
                    if (request == NULL) {
                        log_err("malloc(sizeof(zv_http_request_t))");
                        break;
                    }

                    zv_init_request_t(request, infd, epfd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    zv_epoll_add(epfd, infd, &event);
                    zv_add_timer(request, TIMEOUT_DEFAULT, zv_http_close_conn);
                }   // end of while of accept

            } 
            else 
            {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                    log_err("epoll error fd: %d", r->fd);
                    close(fd);
                    continue;
                }

                log_info("new data from fd %d", fd);
                //rc = threadpool_add(tp, do_request, events[i].data.ptr);
                //check(rc == 0, "threadpool_add");

                do_request(events[i].data.ptr);
            }
        }   //end of for
    }   // end of while(1)
    

    /*
    if (threadpool_destroy(tp, 1) < 0) {
        log_err("destroy threadpool failed");
    }
    */

    return 0;
}


/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */

#include <strings.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "http.h"
#include "http_parse.h"
#include "http_request.h"
#include "epoll.h"
#include "error.h"
#include "timer.h"

static const char* get_file_type(const char *type);
static void parse_uri(char *uri, int length, char *filename, char *querystring);
static void do_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
static void serve_static(int fd, char *filename, size_t filesize, zv_http_out_t *out);
static char *ROOT = NULL;

mime_type_t zaver_mime[] = 
{
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {NULL ,"text/plain"}
};

void do_request(void *ptr) {
    //由于参数为void，所以需要强制转换为zv_http_request_t
    //ptr就是包头
    zv_http_request_t *r = (zv_http_request_t *)ptr;
    int fd = r->fd;
    int rc, n;
    char filename[SHORTLINE];
    struct stat sbuf;
    ROOT = r->root;
    char *plast = NULL;
    size_t remain_size;
    
    zv_del_timer(r);
    //删除原超时节点
    for(;;) {
        plast = &r->buf[r->last % MAX_BUF];
        
        remain_size = MIN(MAX_BUF - (r->last - r->pos) - 1, MAX_BUF - r->last % MAX_BUF);

        n = read(fd, plast, remain_size);
        /*
        ssize_t read (int fd, void *buf, size_t count);
        参数count是请求读取的字节数，读上来的数据保存在缓冲区buf中，同时文件的当前读写位置向后移。
        注意这个读写位置和使用C标准I/O库时的读写位置有可能不同，这个读写位置是记在内核中的，而使用C标准I/O库时的读写位置是用户空间I/O缓冲区中的位置。
        比如用fgetc读一个字节，fgetc有可能从内核中预读1024个字节到I/O缓冲区中，再返回第一个字节，这时该文件在内核中记录的读写位置是1024，而在FILE结构体中记录的读写位置是1。
        注意返回值类型是ssize_t，表示有符号的size_t，这样既可以返回正的字节数、0（表示到达文件末尾）也可以返回负值-1（表示出错）。
        */
        check(r->last - r->pos < MAX_BUF, "request buffer overflow!");

        if (n == 0) {   
            // EOF
            log_info("read return 0, ready to close fd %d, remain_size = %zu", fd, remain_size);
            goto err;
        }

        if (n < 0) {
            /*对非阻塞socket而言，EAGAIN不是一种错误*/
            if (errno != EAGAIN) {
                log_err("read err, and errno = %d", errno);
                goto err;
            }
            break;
        }

        r->last += n;
        check(r->last - r->pos < MAX_BUF, "request buffer overflow!");
        
        log_info("ready to parse request line"); 
        rc = zv_http_parse_request_line(r);
        if (rc == ZV_AGAIN) {
            continue;
        } else if (rc != ZV_OK) {
            log_err("rc != ZV_OK");
            goto err;
        }

        log_info("method == %.*s", (int)(r->method_end - r->request_start), (char *)r->request_start);
        log_info("uri == %.*s", (int)(r->uri_end - r->uri_start), (char *)r->uri_start);

        debug("ready to parse request body");
        rc = zv_http_parse_request_body(r);
        if (rc == ZV_AGAIN) {
            continue;
        } else if (rc != ZV_OK) {
            log_err("rc != ZV_OK");
            goto err;
        }
        
        /*
        *   handle http header
        */
        zv_http_out_t *out = (zv_http_out_t *)malloc(sizeof(zv_http_out_t));
        if (out == NULL) {
            log_err("no enough space for zv_http_out_t");
            exit(1);
        }

        rc = zv_init_out_t(out, fd);
        check(rc == ZV_OK, "zv_init_out_t");

        parse_uri(r->uri_start, r->uri_end - r->uri_start, filename, NULL);
        /*
        定义函数:    int stat(const char *file_name, struct stat *buf);
        函数说明:    通过文件名filename获取文件信息，并保存在buf所指的结构体stat中
        返回值:     执行成功则返回0，失败返回-1，错误代码存于errno
        • 	必需。
        • 规定文件的路径。
        • 该函数将返回一个包含下列元素的数组：
        • [0] 或 [dev] - 设备编号
        • [1] 或 [ino] - inode 编号
        • [2] 或 [mode] - inode 保护模式
        • [3] 或 [nlink] - 连接数目
        • [4] 或 [uid] - 所有者的用户 ID
        • [5] 或 [gid] - 所有者的组 ID
        • [6] 或 [rdev] - inode 设备类型
        • [7] 或 [size] - 文件大小的字节数
        • [8] 或 [atime] - 上次访问时间（Unix 时间戳）
        • [9] 或 [mtime] - 上次修改时间（Unix 时间戳）
        • [10] 或 [ctime] - 上次 inode 改变时间（Unix 时间戳）
        • [11] 或 [blksize] - 文件系统 IO 的块大小（如果支持）
        • [12] 或 [blocks] - 所占据块的数目
        struct stat {
            dev_t         st_dev;      // device 
            ino_t         st_ino;      // inode 
            mode_t        st_mode;     // protection 
            nlink_t       st_nlink;    // number of hard links 
            uid_t         st_uid;      // user ID of owner 
            gid_t         st_gid;      // group ID of owner 
            dev_t         st_rdev;     // device type (if inode device) 
            off_t         st_size;     // total size, in bytes 
            blksize_t     st_blksize;  // blocksize for filesystem I/O 
            blkcnt_t      st_blocks;   // number of blocks allocated 
            time_t        st_atime;    // time of last access 
            time_t        st_mtime;    // time of last modification 
            time_t        st_ctime;    // time of last status change 
        };
        错误代码:
            ENOENT         参数file_name指定的文件不存在
            ENOTDIR        路径中的目录存在但却非真正的目录
            ELOOP          欲打开的文件有过多符号连接问题，上限为16符号连接
            EFAULT         参数buf为无效指针，指向无法存在的内存空间
            EACCESS        存取文件时被拒绝
            ENOMEM         核心内存不足
            ENAMETOOLONG   参数file_name的路径名称太长
        注释：
        从这个函数返回的结果与服务器到服务器的结果是不相同的。这个数组包含了数字索引、名称索引或同时包含上述二者。
        注释：
        该函数的结果会被缓存。
        请使用 clearstatcache() 来清除缓存。
        */
        if(stat(filename, &sbuf) < 0) {
            do_error(fd, filename, "404", "Not Found", "zaver can't find the file");
            continue;
        }
        /*
        S_ISREG是否是一个常规文件.
        S_ISDIR是否是一个目录
        S_ISCHR是否是一个字符设备.
        S_ISBLK是否是一个块设备
        S_ISFIFO是否是一个FIFO文件.
        S_ISSOCK是否是一个SOCKET文件.
        S_IRUSR：用户读权限
        S_IWUSR：用户写权限
        S_IRGRP：用户组读权限
        S_IWGRP：用户组写权限
        S_IROTH：其他组都权限
        S_IWOTH：其他组写权限
        */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            do_error(fd, filename, "403", "Forbidden",
                    "zaver can't read the file");
            continue;
        }
        //上次修改时间
        out->mtime = sbuf.st_mtime;

        zv_http_handle_header(r, out);
        check(list_empty(&(r->list)) == 1, "header list should be empty");
        
        if (out->status == 0) {
            out->status = ZV_HTTP_OK;
        }

        serve_static(fd, filename, sbuf.st_size, out);

        if (!out->keep_alive) {
            log_info("no keep_alive! ready to close");
            free(out);
            goto close;
        }
        free(out);

    }
    
    struct epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

    zv_epoll_mod(r->epfd, r->fd, &event);
    zv_add_timer(r, TIMEOUT_DEFAULT, zv_http_close_conn);
    return;

err:
close:
    rc = zv_http_close_conn(r);
    check(rc == 0, "do_request: zv_http_close_conn");
}

static void parse_uri(char *uri, int uri_length, char *filename, char *querystring) {
    check(uri != NULL, "parse_uri: uri is NULL");
    uri[uri_length] = '\0';

    char *question_mark = strchr(uri, '?');
    /*在参数str所指向的字符串中搜索第一次出现字符c（一个无符号字符）的位置。*/
    int file_length;
    if (question_mark) {
        file_length = (int)(question_mark - uri);
        debug("file_length = (question_mark - uri) = %d", file_length);
    } else {
        file_length = uri_length;
        debug("file_length = uri_length = %d", file_length);
    }

    if (querystring) {
        //TODO
    }
    
    strcpy(filename, ROOT);

    // uri_length can not be too long
    if (uri_length > (SHORTLINE >> 1)) {
        log_err("uri too long: %.*s", uri_length, uri);
        return;
    }

    debug("before strncat, filename = %s, uri = %.*s, file_len = %d", filename, file_length, uri, file_length);
    /*
    char *strncat(char *dest, const char *src, size_t n)
    dest -- 指向目标数组，该数组包含了一个 C 字符串，且足够容纳追加后的字符串，包括额外的空字符。
    src -- 要追加的字符串。
    n -- 要追加的最大字符数。
    */
    strncat(filename, uri, file_length);

    char *last_comp = strrchr(filename, '/');
    char *last_dot = strrchr(last_comp, '.');
    /*
    char *strrchr(const char *str, int c)
    在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置。
    str -- C 字符串。
    c -- 要搜索的字符。以 int 形式传递，但是最终会转换回 char 形式。
    */
    if (last_dot == NULL && filename[strlen(filename)-1] != '/') {
        /*将两个char类型连接。
        例如：
        char d[20]="Golden";
        char s[20]="View";
        strcat(d,s);
        //打印d
        printf("%s",d);
        输出 d 为 GoldenView （中间无空格）
        d和s所指内存区域不可以重叠且d必须有足够的空间来容纳s的字符串。
        返回指向d的指针。*/
        strcat(filename, "/");
    }
    
    if(filename[strlen(filename)-1] == '/') {
        strcat(filename, "index.html");
    }

    log_info("filename = %s", filename);
    return;
}

static void do_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char header[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>Zaver Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
    sprintf(body, "%s%s: %s\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\n</p>", body, longmsg, cause);
    sprintf(body, "%s<hr><em>Zaver web server</em>\n</body></html>", body);

    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(header, "%sServer: Zaver\r\n", header);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));
    //log_info("header  = \n %s\n", header);
    rio_writen(fd, header, strlen(header));
    rio_writen(fd, body, strlen(body));
    //log_info("leave clienterror\n");
    return;
}

static void serve_static(int fd, char *filename, size_t filesize, zv_http_out_t *out) {
    char header[MAXLINE];
    char buf[SHORTLINE];
    size_t n;
    struct tm tm;
    
    const char *file_type;
    /*该函数返回 str 中最后一次出现字符 c 的位置。*/
    const char *dot_pos = strrchr(filename, '.');
    file_type = get_file_type(dot_pos);
    //HTTP/1.1 200 OK
    //Server: Zaver
    //ontent-type: text/html
    //Content-length: 566
    //Last-Modified: Tue, 07 Jul 2020 17:39:48 GMT
    sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));

    if (out->keep_alive) {
        sprintf(header, "%sConnection: keep-alive\r\n", header);
        sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT);
    }

    if (out->modified) {
        sprintf(header, "%sContent-type: %s\r\n", header, file_type);
        sprintf(header, "%sContent-length: %zu\r\n", header, filesize);
        localtime_r(&(out->mtime), &tm);
        strftime(buf, SHORTLINE,  "%a, %d %b %Y %H:%M:%S GMT", &tm);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buf);
    }

    sprintf(header, "%sServer: Zaver\r\n", header);
    sprintf(header, "%s\r\n", header);

    n = (size_t)rio_writen(fd, header, strlen(header));
    check(n == strlen(header), "rio_writen error, errno = %d", errno);
    if (n != strlen(header)) {
        log_err("n != strlen(header)");
        goto out; 
    }

    if (!out->modified) {
        goto out;
    }

    int srcfd = open(filename, O_RDONLY, 0);
    check(srcfd > 2, "open error");
    // can use sendfile
    /*void* mmap ( void * addr , size_t len , int prot , int flags , int fd , off_t offset ) 
    mmap的作用是映射文件描述符fd指定文件的 [off,off + len]区域至调用进程的[addr, addr + len]的内存区域
    参数fd为即将映射到进程空间的文件描述字，一般由open()返回，同时，fd可以指定为-1，此时须指定flags参数中的
    MAP_ANON，表明进行的是匿名映射（不涉及具体的文件名，避免了文件的创建及打开，很显然只能用于具有亲缘关系的进程间通信）。
    len是映射到调用进程地址空间的字节数，它从被映射文件开头offset个字节开始算起。
    prot 参数指定共享内存的访问权限。可取如下几个值的或：PROT_READ（可读） , PROT_WRITE （可写）, PROT_EXEC （可执行）, PROT_NONE（不可访问）。
    flags由以下几个常值指定：MAP_SHARED , MAP_PRIVATE , MAP_FIXED，其中，MAP_SHARED , MAP_PRIVATE必选其一，而MAP_FIXED则不推荐使用。
    offset参数一般设为0，表示从文件头开始映射。
    参数addr指定文件应被映射到进程空间的起始地址，一般被指定一个空指针，此时选择起始地址的任务留给内核来完成。
    函数的返回值为最后文件映射到进程空间的地址，进程可直接操作起始地址为该值的有效地址。
    */
    char *srcaddr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    check(srcaddr != (void *) -1, "mmap error");
    close(srcfd);

    n = rio_writen(fd, srcaddr, filesize);
    // check(n == filesize, "rio_writen error");

    munmap(srcaddr, filesize);

out:
    return;
}

static const char* get_file_type(const char *type)
{
    if (type == NULL) {
        return "text/plain";
    }

    int i;
    for (i = 0; zaver_mime[i].type != NULL; ++i) {
        if (strcmp(type, zaver_mime[i].type) == 0)
            return zaver_mime[i].value;
    }
    return zaver_mime[i].value;
}

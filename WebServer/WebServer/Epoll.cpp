// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Epoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include "Util.h"
#include "base/Logging.h"


#include <arpa/inet.h>
#include <iostream>
using namespace std;

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;
//epoll_create1 产生一个epoll 实例，返回的是实例的句柄。
//flag 可以设置为0 或者EPOLL_CLOEXEC，为0时函数表现与epoll_create一致，EPOLL_CLOEXEC标志与open 时的O_CLOEXEC 标志类似，即进程被替换时会关闭文件描述符。
Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
  assert(epollFd_ > 0);
}
Epoll::~Epoll() {}

// 注册新描述符
//event并不和Channel绑定
void Epoll::epoll_add(SP_Channel request, int timeout) {
  int fd = request->getFd();
  if (timeout > 0) {
    //先在定时器中注册新的节点
    add_timer(request, timeout);
    //每出现一个新的request，就注册epoll，然后在对应epoll中保存一个request->http
    fd2http_[fd] = request->getHolder();
  }
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->getEvents();

  request->EqualAndUpdateLastEvents();
  //再保存一个request->channel
  fd2chan_[fd] = request;
  if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_add error");
    fd2chan_[fd].reset();
  }
}

// 根据request修改描述符状态
void Epoll::epoll_mod(SP_Channel request, int timeout) {
  if (timeout > 0) add_timer(request, timeout);
  int fd = request->getFd();
  if (!request->EqualAndUpdateLastEvents()) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
      perror("epoll_mod error");
      fd2chan_[fd].reset();
    }
  }
}

// 从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request) {
  int fd = request->getFd();
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->getLastEvents();
  // event.events = 0;
  // request->EqualAndUpdateLastEvents()
  if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_del error");
  }
  fd2chan_[fd].reset();
  fd2http_[fd].reset();
}

// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll() {
  while (true) {
    int event_count =
        epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
        /*int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);
        等待事件的产生，类似于select()调用。
        参数events用来从内核得到事件的集合，maxevents告之内核这个events有多大，
        这个 maxevents的值不能大于创建epoll_create()时的size，参数timeout是超时时间
        （毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。
        该函数返回需要处理的事件数目，如返回0表示已超时。如果返回–1，则表示出现错误，需要检查 errno错误码判断错误类型。
        第1个参数 epfd是 epoll的描述符。
        第2个参数 events则是分配好的 epoll_event结构体数组，epoll将会把发生的事件复制到 events数组中
        （events不可以是空指针，内核只负责把数据复制到这个 events数组中，不会去帮助我们在用户态中分配内存。内核这种做法效率很高）。
        第3个参数 maxevents表示本次可以返回的最大事件数目，通常 maxevents参数与预分配的events数组的大小是相等的。
        第4个参数 timeout表示在没有检测到事件发生时最多等待的时间（单位为毫秒），
        如果 timeout为0，则表示 epoll_wait在 rdllist链表中为空，立刻返回，不会等待。
        */
    if (event_count < 0) perror("epoll wait error");
    //把events中的元素转为channel
    std::vector<SP_Channel> req_data = getEventsRequest(event_count);
    if (req_data.size() > 0) return req_data;
  }
}

void Epoll::handleExpired() { timerManager_.handleExpiredEvent(); }

// 分发处理函数
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num) {
  std::vector<SP_Channel> req_data;
  for (int i = 0; i < events_num; ++i) {
    // 获取有事件产生的描述符
    int fd = events_[i].data.fd;
    //每个事件有一个描述符，每个描述符对应一个Channel
    SP_Channel cur_req = fd2chan_[fd];

    if (cur_req) {
      cur_req->setRevents(events_[i].events);
      /*
      其中events表示感兴趣的事件和被触发的事件，可能的取值为：
      EPOLLIN：表示对应的文件描述符可以读；
      EPOLLOUT：表示对应的文件描述符可以写；
      EPOLLPRI：表示对应的文件描述符有紧急的数可读；
      EPOLLERR：表示对应的文件描述符发生错误；
      EPOLLHUP：表示对应的文件描述符被挂断；
      EPOLLET：    ET的epoll工作模式；
      */
      cur_req->setEvents(0);
      // 加入线程池之前将Timer和request分离
      // cur_req->seperateTimer();
      req_data.push_back(cur_req);
    } else {
      LOG << "SP cur_req is invalid";
    }
  }
  return req_data;
}

void Epoll::add_timer(SP_Channel request_data, int timeout) {
  shared_ptr<HttpData> t = request_data->getHolder();
  if (t)
    timerManager_.addTimer(t, timeout);
  else
    LOG << "timer add fail";
}
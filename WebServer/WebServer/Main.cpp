// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include <getopt.h>
#include <string>
#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"


int main(int argc, char *argv[]) {
  int threadNum = 4;
  int port = 80;
  std::string logPath = "./WebServer.log";

  // parse args
  int opt;
  const char *str = "t:l:p:";//表示-t、-l、-p这几个选项都需要带参数
  /*
  getopt()函数的出处就是unistd.h头文件
  int getopt(int argc,char * const argv[ ],const char * optstring);
  前两个参数大家不会陌生，没错，就是老大main函数的两个参数！老大传进来的参数自然要有人接着！
  第三个参数是个字符串，看名字，我们可以叫他选项字符串（后面会说明）
  返回值为int类型，我们都知道char类型是可以转换成int类型的，每个字符都有他所对应的整型值，其实这个返回值返回的就是一个字符，什么字符呢，叫选项字符（姑且这么叫吧，后面会进一步说明）
  小弟1、extern char* optarg;
  小弟2、extern int optind;
  小弟3、extern int opterr;
  小弟4、extern int optopt;
  队形排的不错。
  小弟1是用来保存选项的参数的（先混个脸熟，后面有例子）；
  小弟2用来记录下一个检索位置；
  小弟3表示的是是否将错误信息输出到stderr，为0时表示不输出，
  小弟4表示不在选项字符串optstring中的选项（有点乱哈，后面会有例子）
  optind和opterr的初始值都为1，那么optind的初值为什么是1呢
  这就要涉及到main函数的那两个参数了，argc表示参数的个数，argv[]表示每个参数字符串，
  ./WebServer -t 10 -p 88 -l /home
  对于上面的输出argc就为7，argv[]分别为：
  ./WebServer、-t、10、-p、88、-l、/home
  所以真正的参数从-t开始，所以optind为1
  当执行getopt()函数时，会依次扫描每一个命令行参数（从下标1开始）,
  第一个-t,是一个选项，而且这个选项在选项字符串optstring中有，我们看到t后面有冒号，也就是t后面必须带有参数，
  而10就是他的参数。所以这个命令行是符合要求的。参数保存在optarg中
  至于执行后optind是3，这是因为optind是下一次进行选项搜索的开始索引，也是说下一次getopt()函数要从argv[3]开始搜索。
  */
  //./WebServer -t 10 -p 88 -l /home
  /*
  总结：对于getopt，第一个参数为main函数的参数数量，第二个为参数数组，第三个为参数格式
  返回值为当前下标对应的参数，该参数的参数保存在optarg中
  */
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg;
        if (logPath.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          //abort调用会使函数异常终止
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  //Logger类的static函数，未实例化的时候也可以调用
  Logger::setLogFileName(logPath);
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !";
#endif
  EventLoop mainLoop;
  Server myHTTPServer(&mainLoop, threadNum, port);
  myHTTPServer.start();
  mainLoop.loop();
  return 0;
}

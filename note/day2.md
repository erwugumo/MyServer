我们平时用到的套接字其实只是一个引用(一个对象ID)，这个套接字对象实际上是放在操作系统内核中。
这个套接字对象内部有两个重要的缓冲结构，一个是读缓冲(read buffer)，一个是写缓冲(write buffer)，它们都是有限大小的数组结构。
当我们对客户端的socket写入字节数组时(序列化后的请求消息对象req)，
是将字节数组拷贝到内核区套接字对象的write buffer中，
内核网络模块会有单独的线程负责不停地将write buffer的数据拷贝到网卡硬件，
网卡硬件再将数据送到网线，经过一些列路由器交换机，最终送达服务器的网卡硬件中。
同样，服务器内核的网络模块也会有单独的线程不停地将收到的数据拷贝到套接字的read buffer中等待用户层来读取。
最终服务器的用户进程通过socket引用的read方法将read buffer中的数据拷贝到用户程序内存中进行反序列化成请求对象进行处理。
然后服务器将处理后的响应对象走一个相反的流程发送给客户端，这里就不再具体描述。

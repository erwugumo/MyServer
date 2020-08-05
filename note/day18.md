再重申一遍：
TCP连接，是服务器端先新建一个socket，本质上是一个文件，作为监听socket;每次有新的客户端连接，都会先写入这个socket，然后服务器会新建一个socket，用客户端的ip和port标识;下次这个客户端再与服务器通信，就直接往新的socket中写

四个线程运行四个eventloop，处理已有请求
主进程运行eventloop，处理新接进来的请求，给它一个socket，给他一个Channel，然后扔给一个线程。
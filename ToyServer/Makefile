objects= helper.o echoserv.o
serv:$(objects)
    cc -o  serv $(objects)

echoserv.o:helper.h

.PHONY:clean
clean:
    rm serv $(objects)                                            
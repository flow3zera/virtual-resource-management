#ifndef _EPOLL_SERVER_H_
#define _EPOLL_SERVER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include "ThreadPool.hpp"

using namespace std;
 
#define MAX_EVENT 1024   //epoll_events的最大个数
#define MAX_BUFFER 4096 //Buffer的最大字节

unordered_map<int, string> fdCache;
//unordered_map<char, string> clientList;
unordered_map<char, int> fdList;

class BaseTask{
public:
	virtual void doit() = 0;
};
 
class Task : public BaseTask{
private:
    string command;
	int socket_fd;
    int epoll_fd;
	
    //FILE *fp;

public:
	Task(string str, int fd, int epfd) : command(str), socket_fd(fd), epoll_fd(epfd){}
	
    void doit()  //任务的执行函数
	{
        int n;
        char buffer[MAX_BUFFER];

        cout << "command:" << command << endl;

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = socket_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &ev);

        string resp;
        if(fdList.find(command[0]) != fdList.end())
            resp = "valid command";
        else
            resp = "invalid command";

        bzero(buffer, sizeof(buffer));
        strcpy(buffer, resp.c_str());
        send(socket_fd, buffer, strlen(buffer), 0);
       
        string data = "";
        if(resp[0] == 'v'){     
            bzero(buffer, sizeof(buffer));
		    n = recv(socket_fd, buffer, MAX_BUFFER, 0);
            buffer[n] = '\0';
            data = buffer;

            cout << "data: " << data << endl;
        }

        if(resp[0] == 'v'){
             cout << "send to cal_client" << endl;

             char data_buffer[MAX_BUFFER];
             bzero(data_buffer, sizeof(data_buffer));
             strcpy(data_buffer, data.c_str());
             send(fdList[command[0]], data_buffer, strlen(data_buffer), 0);
        }
        else
            cout << "no data to cal_client" << endl;
        /*
             bzero(buffer, sizeof(buffer));
             strcpy(buffer, data.c_str());
             send(fdList[command[0]], buffer, strlen(buffer), 0);
        
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, resp.c_str());
        if(send(socket_fd, buffer, strlen(buffer), 0) == -1){
             cout << "send error" << endl;
             return;
        }

        string fileName = "cache/" + command + ".json";

        if((fp = fopen(fileName.c_str(), "ab")) == NULL)
             cout << "file error" << endl;

        char buffer[MAX_BUFFER];
        int n;
        while(1){
             bzero(buffer, sizeof(buffer));
             n = read(socket_fd, buffer, MAX_BUFFER);
             if(n == 0)
                 break;
             fwrite(buffer, 1, n, fp);
        }

        fclose(fp);

        string resp;
        if(clientList.find(command[0]) != clientList.end())
            resp = "valid command";
        else
            resp = "invalid command";
	    
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, resp.c_str());
        send(socket_fd, buffer, strlen(buffer), 0);

        //struct epoll_event ev2;
        //ev.data.fd = socket_fd;
*/
        ev.events = EPOLLIN|EPOLLET;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);
	}
};
 
class EpollServer
{
private:
	bool is_stop;   //是否停止epoll_wait的标志
	int threadnum;  //线程数目
	int listen_fd;  //监听的fd
	int port;       //端口
	int epoll_fd;   //epoll的fd
	threadpool<BaseTask> *pool;   //线程池的指针
	epoll_event events[MAX_EVENT];//epoll的events数组
	struct sockaddr_in bindAddr;  //绑定的sockaddr
 
public://构造函数
	EpollServer(){}

	EpollServer(int ports, int thread): is_stop(false), threadnum(thread), port(ports), pool(NULL){}

	~EpollServer(){
		delete pool;
	}
 
	void init();
 
	void epoll();
 
	static int setnonblocking(int fd){ //将fd设置为非阻塞
		int old_option = fcntl(fd, F_GETFL);
		int new_option = old_option | O_NONBLOCK;
		fcntl(fd, F_SETFL, new_option);
		return old_option;
	}
 
	static void addfd(int epoll_fd, int socket_fd, bool oneshot){ //向Epoll中添加fd
	    //oneshot表示是否设置同一时刻，只能有一个线程访问fd，数据的读取都在主线程中，所以调用都设置成false
		epoll_event event;
		event.data.fd = socket_fd;
		event.events = EPOLLIN | EPOLLET;
		if(oneshot)
		    event.events |= EPOLLONESHOT;
		
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event); //添加fd
		//EpollServer::setnonblocking(socket_fd);
	}
};
 
void EpollServer::init(){
	bzero(&bindAddr, sizeof(bindAddr)); //将绑定的地址全部置零
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bindAddr.sin_port = htons(port); //将输入的端口号绑定在这个地址上

    //创建Socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0){
		cout << "EpollServer socket init error" << endl;
		return;
	}

	int n = bind(listen_fd, (struct sockaddr *)&bindAddr, sizeof(bindAddr));
	if(n < 0){
		cout << "EpollServer bind init error" << endl;
		return;
	}

	n = listen(listen_fd, 10);
	if(n < 0){
		cout << "EpollServer listen init error" << endl;
		return;
	}
/*
    //通过本地文件初始化虚拟资源匹配表
    ifstream infile("./clientList.txt");
    while(!infile.eof()){
         string tmp;
         getline(infile, tmp);
         if(tmp.empty()) break; //文档中可能在末尾有空行
         clientList[tmp[0]] = tmp.substr(2);
    }
*/
    //创建Epoll
	epoll_fd = epoll_create(1024);
	if(epoll_fd < 0){
		cout << "EpollServer epoll_create init error" << endl;
		return;
	}

	pool = new threadpool<BaseTask>(threadnum);  //创建线程池
}
 
void EpollServer::epoll(){
	pool->start();   //线程池启动
	
	addfd(epoll_fd, listen_fd, false);

	while(!is_stop){
		int n = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
		if(n < 0){
			cout << "epoll_wait error" << endl;
			break;
		}

		for(int i = 0; i < n; ++i){ //遍历处理所有待处理的event
			int fd = events[i].data.fd;

			if(fd == listen_fd){ //新的连接
				struct sockaddr_in clientAddr;
				socklen_t len = sizeof(clientAddr);
				int confd = accept(listen_fd, (struct sockaddr *)&clientAddr, &len);
                cout << "New connection" << endl;

                char buffer[MAX_BUFFER];
                bzero(buffer, sizeof(buffer));
                recv(confd, buffer, MAX_BUFFER, 0);
                cout << buffer << endl;
                if(buffer[0] == '0')
                    addfd(epoll_fd, confd, false);
                else
                    fdList[buffer[0]] = confd;
			}
			else if(events[i].events & EPOLLIN){ //fd上有数据可读
				char recv_buff[MAX_BUFFER];
            	bzero(recv_buff, sizeof(recv_buff));
				int n = recv(fd, recv_buff, MAX_BUFFER, 0);
                string command = recv_buff;

				if(n == 0){ //某个fd关闭了连接，从Epoll中删除并关闭fd
					struct epoll_event ev;
					ev.events = EPOLLIN;
					ev.data.fd = fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
					shutdown(fd, SHUT_RDWR);
					cout << "fd:" << fd << " close" << endl;
					continue;
				}
				else if(n < 0){ //读取出错
					cout << "read error!" << endl;
				    continue;
				}
				else{ //成功读取，向线程池中添加任务
					BaseTask *task = new Task(command, fd, epoll_fd);
					pool->append_task(task);
				}
			}
			else if(events[i].events & EPOLLOUT){
                char send_buff[MAX_BUFFER];
                string str = fdCache[fd];
                strcpy(send_buff, str.c_str());

                send(fd, send_buff, strlen(send_buff), 0); //发送数据

		        struct epoll_event ev;
                ev.data.fd = fd;
                ev.events = EPOLLIN|EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev); //修改标识符，等待下一个循环时接收数据
            }
		}
	}
	close(listen_fd);//结束。
 
	pool->stop();
}
 
#endif

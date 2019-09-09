#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

using namespace std;

#define MAXLINE 4096

int main(int argc, char** argv){
	int sock_fd, n;
	char recv_buff[MAXLINE], send_buff[MAXLINE];
	struct sockaddr_in serv_addr;

	if(argc != 3){
		cout << "用法：./client <ipaddress> <command>" << endl;
		return -1;
	}

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		cout << "创建socket失败" << endl;
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6666);
	
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) == -1){
		cout << "IP地址绑定失败" << endl;
		return -1;
	}

	if(connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
		cout << "连接失败" << endl;
		return -1;
	}

    string reg = argv[2];
    bzero(send_buff, sizeof(send_buff));
    strcpy(send_buff, reg.c_str());
    send(sock_fd, send_buff, strlen(send_buff), 0);

	cout << "|等待数据|" << endl;

    while(1){
         bzero(recv_buff, sizeof(recv_buff));
         n = recv(sock_fd, recv_buff, MAXLINE, 0);
         recv_buff[n] = '\0';
         cout << recv_buff << endl; 
    }
    close(sock_fd);
}

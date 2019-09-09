#include <iostream>
#include "EpollServer.hpp"
#include <unordered_map>
int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		cout << "usage " << argv[0] << " port threadnum" << endl;
		return -1;
	}

	int port = atoi(argv[1]);
	if(port == 0)
	{
		cout << "port must be Integer" << endl;
		return -1;
	}
	int threadnum = atoi(argv[2]);
	if(threadnum == 0)
	{
		cout << "threadnum must be Integer" << endl;
		return -1;
	}
	EpollServer *epoll = new EpollServer(port, threadnum);
 
	epoll->init();
 
	epoll->epoll();
	return 0;
}

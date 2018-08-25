#include<stdio.h>
#include"httpd.h"
#include<pthread.h>

static void usage(const char* proc)
{
	printf("Usage : %s [local_ip] [local_port]\n",proc);
}

void* accept_request(void* arg)
{

	int*  data = reinterpret_cast<int*>(arg);
	int sock =*data;
	pthread_detach(pthread_self());
	return  reinterpret_cast<void*>(handler_request(sock));	
}

int main(int argc,char* argv[])
{
	if(argc !=3)
	{
		usage(argv[0]);
		return 1;
	}
	signal(SIGPIPE, SIG_IGN);
	int listen_sock = startup(argv[1],atoi(argv[2]));

	while(1)
	{
		struct sockaddr_in client;
		socklen_t len = sizeof(client);
		int new_sock = accept(listen_sock,(struct sockaddr*)&client,&len);
		if(new_sock < 0)
		{
			print_log("accept failed ",NOTICE);
			continue;
		}

		printf("get client [%s : %d]\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		//down
		pthread_t id;

		int ret = pthread_create(&id,NULL,accept_request,&new_sock);
		if(ret != 0)
		{
			print_log("pthread_creat failed",WARNING);
			close(new_sock);
			continue;
		}
		else
		{
			pthread_detach(id);
		}
	}
	close(listen_sock);
	return 0;
}

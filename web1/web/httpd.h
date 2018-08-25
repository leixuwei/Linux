#ifndef _HTTPD_
#define _HTTPD_

#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<sys/types.h>
#include<sys/wait.h>

#define SUCCESS 0
#define NOTICE 1
#define WARNING 2
#define ERROR 3
#define FATAL 4

#define SIZE 1024
#define MAIN_PAGE "index.html"
#define PAGE_404 "wwwroot/404.html"

void print_log(const char* msg,int level);
int startup(const char* ip,int port);
int handler_request(int arg);
static int get_line(int sock, char * buf, int len);


#endif

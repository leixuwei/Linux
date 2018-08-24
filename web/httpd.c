#pragma GCC diagnostic ignored"-Wwrite-strings"
#include"httpd.h"
#include<assert.h>


//调用失败之后打印日志
void print_log(const char* msg,int level)
{
#ifdef _STDOUT_
	char* level_msg[] = 
	{
		"SUCCESS",
		"NOTICE",
		"WARMING",
		"ERROR",
		"FATAL",
	};
	printf("[%s][%s]\n",msg,level_msg[level%5]);
#endif
}

//创建监听套接字
int startup(const char* ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		print_log("sock failed !",FATAL);
		exit(2);
	}
	//防止因为 time_wait 导致服务器不能重启
	int opt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		print_log(" bind failed !",FATAL);
		exit(3);
	}

	//	if(listen(sock,10) < 0)
	if(listen(sock,5) < 0)
	{
		print_log("Listen Failed !",FATAL);
		exit(4);
	}
	return sock;
}
// 按行读取内容
//ret > 1 line != '\0' ||||  ret = 1 & line = '\n'; |||| ret = 0 && line == '\0'.
//static int get_line(int sock,char line[],int size)
//{
//	//read 1 char , one by one
//	char c = '\0';
//	int len = 0;
//	while( c != '\n' && len < size-1)
//	{
//		int r = recv(sock,&c,1,0);//read
//		if( r > 0 )
//		{
//			if(c == '\r')
//			{
//				//下一个是否\n
//				int ret = recv(sock,&c,1,MSG_PEEK); // LOOK AND DONT TAKE
//				if(ret > 0)
//				{
//					if( c == '\n')
//					{
//						recv(sock,&c,1,0);
//					}else
//					{
//						c = '\n'; //不是\r\n结尾 设置成\n
//					}
//				}
//			}// /r -> /n /r/n -> /n   c == \n
//			line[len++] = c;
//		}else
//		{
//			c = '\n';
//		}
//		line[len] = '\0';  
//	}
//	return len;
//}


static int get_line(int sock, char * buf, int len)
{
	assert(buf);
	char ch='\0';
	int i = 0;
	while(i < len -1 && ch != '\n' )
	{
		if(recv(sock, &ch, 1, 0) > 0)
		{
			if(ch == '\r')
			{
				//考虑下一个字符是否为\n
				if( recv(sock, &ch, 1, MSG_PEEK) > 0 && ch == '\n' )
					recv(sock, &ch, 1, 0);
				else
					ch = '\n';//不是\r\n结尾 设置成\n
			}
			buf[i++] = ch;
		}
		//buf[i++] = ch;
	}
	buf[i] = '\0';
	return i;
}


void bad_request(const char* path, const char* head, int sock)
{
	struct stat st;			   
	if(stat(path, &st) < 0)
	{
		return;
	}			    
	int fd = open(path, O_RDONLY);					    

	//响应报头+空行
	const char* status_line = head;
	send(sock,status_line,strlen(status_line),0);
	const char* content_type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock,content_type,strlen(content_type),0);
	send(sock, "\r\n", 2, 0);

	sendfile(sock, fd, NULL,st.st_size);
	close(fd);
}

//给用户回显错误码的函数
static void echoErrno(int sock,int err_code)
{
	switch(err_code)
	{
		case 404:
			bad_request("wwwroot/404.html", "HTTP/1.0 404 Not Found\r\n", sock);
			break;
		case 503:
			bad_request("wwwroot/503.html","HTTP/1.0 503 Server Unavailable\r\n",sock);
			break;
		default:
			break;
	}
}

//echo资源
static int echo_www(int sock,char* path,int size)//处理非CGi的请求
{
	int fd = open(path,O_RDONLY);
	if(fd < 0)
	{
		echoErrno(sock,503);
		print_log("Open failed! ",FATAL);
		return 8;
	}

	//我们的一个相应信息，必须要包括响应行，消息报头，空行，有效载荷.
	const char* echo_line = "HTTP/1.0 200 OK\r\n";
	send(sock,echo_line,strlen(echo_line),0);   //发送状态行
	const char* null_line = "\r\n";              //发送空行
	send(sock,null_line,strlen(null_line),0);

	//sendfile 在内核区建立两个fd的数据拷贝,节约效率.
	if(sendfile(sock,fd,NULL,size) < 0)
	{
		echoErrno(sock,503);
		print_log("Sendfile Failed! ",FATAL);
		return 9;
	}

	close(fd);
	return 0;
}

//对于我们的服务器来说，当你读完请求行的时候 你得到了 方法 路径 参数,剩下的消息报头不需要再关心了
//所以 一个get_line 一个 读到空行为止.
static int drop_header(int sock)
{

	char line[SIZE];
	int ret = -1;
	do{
		ret = get_line(sock,line,sizeof(line));
	}while(ret!=1 && strcmp(line, "\n"));

//	}while(ret>0 && strcmp(line, "\n"));
	return ret;
}


static int exe_cgi(int sock,char* method,char* path,char* query_string)//处理CGI模式的请求
{
	char line[SIZE];
	int ret = 0;
	int content_len = -1;
	char method_env[SIZE];
	char query_string_env[SIZE];
	char content_len_env[SIZE/16];

	if(strcasecmp(method,"GET") == 0) //GET 方法的cgi
	{
		drop_header(sock);
	}else    //是POST方法 需要读取参数，正文长度表示content_len 防止粘包问题
	{
		do{
			//读取到POST方法当中的Content-Length的长度
			ret = get_line(sock,line,sizeof(line));
			//获得请求消息的大小
			if(ret > 0 && strncasecmp(line,"Content-Length: ",16) == 0)
			{
				//content_len = atoi(&line[16]);

				content_len = atoi(line+16);//求出正文长度
			}
//		}while(ret!=1 && strcmp(line,"\n"));
		}while(ret>0 && strcmp(line,"\n")!=0);

		if(content_len == -1)
		{
			print_log("have no arguments!",FATAL);
			echoErrno(sock,404);
			ret = 10;
		}
	}


	const char* echo_line = "HTTP/1.0 200 OK\r\n";
	send(sock,echo_line,strlen(echo_line),0);
	const char* null_line = "\r\n";
	send(sock,null_line,strlen(null_line),0);
	send(sock,"\r\n",2,0);
	//path -> exec
	//创建管道
	int input[2];
	int output[2];

	pipe(input);
	pipe(output);

	if(pipe(input) < 0 || pipe(output) < 0)
	{
		echoErrno(sock,503);
		print_log("pipe failed ",FATAL);
		return 11;
	}
	//fork出来一个子进程用来执行cgi可执行方法,然后客户端和父进程通信，父进程和子进程通信.
	//子进程将可执行文件运行的结果传给父进程，然后父进程传输给客户端.
	pid_t id = fork();
	if(id < 0)
	{
		
		print_log("fork failed",FATAL);
		echoErrno(sock,503);
		return 12;
	}else if(id == 0)
	{
		//child 
		//关闭文件描述符，文件输出重定向，
		//环境变量传递变量,exec程序替换
		close(input[1]);
		close(output[0]);

		sprintf(method_env,"METHOD=%s",method);
		putenv(method_env);

		if(strcasecmp(method, "GET") == 0)
		{
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			//putenv(query_string_env);
		}else // post
		{
			sprintf(content_len_env,"CONTENT_LENGTH=%d",content_len);
			//putenv(content_len_env);
		}
		//将文件描述符重定向到标准输入标准输出
		dup2(input[0],0);// 标准输入重定向到子进程
		dup2(output[1],1);//将子进程输出重定向到标准输出 到浏览器

		execl(path,path,NULL);//替换CGI程序
		exit(1);
	}
	else
	{
		//父进程
		//关闭适当文件描述符
		//其中的方法 决定读写顺序
		//将数据和方法交给子进程后等待子进程返回结果
		close(input[0]);
		close(output[1]);

		//POST
		int i = 0;
		char c = '\0';

		if(strcasecmp(method,"POST") == 0)
		{
			for(; i < content_len; ++i)
			{
				recv(sock,&c,1,0);// 先sock获取数据 一次读一个字符 总共读取content_len个字符
				write(input[1],&c,1);// 通过管道传给子进程
			}
		}

		c = '\0';
		
		//接收子进程的返回结果
		while(read(output[0],&c,1) > 0)//如果是CGI程序结束，写端关闭 则读端返回0
		{
			send(sock,&c,1,0);
		}

		waitpid(id,NULL,0);//回收子进程

		close(input[1]);
		close(output[0]);
	}
	return 0;
}


//浏览器处理函数
int handler_request(int sock)
{
	//	int*  data = reinterpret_cast<int*>(arg);
	//	int   sock    = *data;


#ifdef _DEBUG_
	char line[SIZE];
	do{
		int ret = get_line(sock,line,sizeof(line));
		if(ret > 0)
		{
			printf("%s",line);
		}else
		{
			printf("request... .... done\n");
			break;
		}
		//	}while(1)
		}while(ret!=1 && strcmp(line,"\n")!=0);
#else
	int i,j;
	int cgi=0;
	int  ret = 0;
	char buf[SIZE];
	char method[SIZE/16];
	char path[SIZE];
	char url[SIZE];
	char* query_string = NULL; //参数(GET方法)

	//bzero(buf,sizeof(buf));
	get_line(sock,buf,sizeof(buf));

	if(get_line(sock,buf,sizeof(buf)) <= 0)//如果为空行 至少有一个‘\n’ 因此应为<=0时候读取失败
	{
		print_log("get_line error! ",FATAL);
		echoErrno(sock,404);
		ret = 5;
		goto end;
	}

	printf("%s\n",buf);

	i=0; //method-> index
	j=0; //buf -> index.

	//GET / http/1.0
	//遇到空格时候  则读到的就为方法
	while( !isspace(buf[j]) && j < sizeof(buf)-1 && i < sizeof(method)-1)
	{
		method[i]=buf[j];
		i++,j++;
	}

	//method[i] = 0;
	method[i]='\0';

	//只处理GET 和 POST 方法
//	if(strcasecmp(method, "GET") && strcasecmp(method,"POST") )
	if(!strcasecmp(method, "GET") && !strcasecmp(method,"POST") )
	{
		print_log("method failed",FATAL);
		echoErrno(sock,503);
		ret = 6;
		goto end;
	}

	// POST 要支持cgi模式
	if(strcasecmp(method,"POST") == 0)
	{
		cgi = 1;
	}

	//过滤掉空格 是j指向资源路径的有效字符处
	while(isspace(buf[j]) && j < sizeof(buf))
	{
		++j;
	}

	i = 0;
	while(!isspace(buf[j]) && j < sizeof(buf) && i < sizeof(url)-1)
	{
		url[i] = buf[j];
		i++,j++;
	}
	//url[i] = 0;
	url[i]='\0';
	printf("method: %s,url : %s\n",method,url);

	if(strcasecmp("GET",method) == 0)
		{
			query_string = url;
			while(*query_string != '\0' && *query_string != '?')
					query_string++;
				//如果有'?'表明有参数，使query_string指向参数处
				if(*query_string == '?')
				{
					*query_string = '\0';
					query_string++;	
					cgi=1;
				}	
			}
//	if(strcasecmp("GET",method)==0)
//	{
//		query_string=url;
//	//	while(*query_string!='\0' && *query_string!='?')
//	//		query_string++;
//			while(*query_string != '\0')
//			{
//			// 如果‘？’表名有参数，使query_string 指向参数处
//			if(*query_string == '?')
//			{
//				*query_string = '\0';
//				++query_string;
//				cgi = 1;
//				break;
//			}
//			++query_string;
//		}
//	}

	// method,url,query_string,cgi.
	//如果路径只有/ 那就跳转至我们的主页，我们的主页就是index.html
	//转换路径 /XX/YY/ZZ ->->wwwroot/XX/YY/ZZ
	sprintf(path,"wwwroot%s",url);//wwwroot 输出到path
	
	if(path[strlen(path)-1] == '/') //如果是上一目录 就拼上默认主页
	{
		strcat(path,"index.html");
	}
	printf("method: %s,url: %s,query_string: %s\n",method,path,query_string);
	
	//GET 带参
	struct stat st;
	if(stat(path,&st)<0)//获取客户端请求的资源的相关属性
	{
		print_log("stat path faile ",FATAL);
		drop_header(sock);
		echoErrno(sock,404);
		ret = 7;
		goto end;
	}else{
		//判断path是不是一个目录
		if(S_ISDIR(st.st_mode))
		{
			strcat(path,"/index.html");
		}
		//判断是不是普通文件
		else if(S_ISREG(st.st_mode))
		{ 
				if((st.st_mode & S_IXUSR) ||\
					(st.st_mode & S_IXGRP) ||\
					(st.st_mode & S_IXOTH))
				cgi = 1;
		}else
		{}
		}

		// 处理cgi模式或非cgi模式
		//method (GET,POST),path(EXIST),cgi(0|1),query_string(GET)
		//if(cgi==1)
		if(cgi==1)
		{
			//执行cgi过程
			//	exe_cgi(sock,method,path,query_string);
			ret=exe_cgi(sock,method,path,query_string);
		}
		else
		{
			//drop_header(sock);
			ret=drop_header(sock);
			printf("method : %s, url : %s ,path : %s,cgi : %d,query_string: %s\n",\
					method,url,path,cgi,query_string);
			ret=echo_www(sock,path,st.st_size);////如果是GET方法，而且没有参数，请求的也不是可执行程序，则直接返回资源
			//echo_www(sock,path,st.st_size);
		}
end:
	printf("quit client...\n");
	close(sock);
	return ret;
#endif
}


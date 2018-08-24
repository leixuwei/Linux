#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#define SIZE 1024
//void mymath(char* arg)
//{
//	assert(arg);
//	char* argv[3];
//
//	int i = 0;
//
//	char* start = arg;
//	while(*start)
//	{
//		if(*start == '=')
//		{
//			start++;
//			argv[i++] = start;
//			continue;
//		}
//		if(*start == '&')
//		{
//			*start = '\0';
//		}
//		start++;
//	}
//	argv[i] = NULL;
//	int num1 = atoi(argv[0]);
//	int num2 = atoi(argv[1]);
//
//	perror("data1 and data2!");
//	printf("<html>\n");
//	printf("<body>\n");
//	printf("<h1>data1 + data2 = %d</h1>\n",num1 + num2);
//	printf("<h1>data1 - data2 = %d</h1>\n",num1 - num2);
//	printf("<h1>data1 * data2 = %d</h1>\n",num1 * num2);
//	printf("<h1>data1 / data2 = %d</h1>\n",num1 / num2);
//	printf("</body>\n");
//	printf("</html>\n");
//
//	//    printf("<html><body><h3>\n");
//	//    printf("%d + %d = %d\n",data1,data2,data1+data2);
//	//	  printf("<html><h1>data1 + data2 = %d\n</h1></html>",atoi(argv[0]) + atoi(argv[1]));
//
//}
//
//int main()
//{
//	char* method = NULL;
//	char* query_string = NULL;
//	char* string_arg = NULL;
//	int content_len = -1;
//	char buf[_SIZE_];
//
//	if((method=getenv("METHOD")))
//	{
//		if(strcasecmp(method,"GET") == 0)
//		{
//			if((query_string=getenv("QUERY_STRING")))
//			{
//				string_arg = query_string;
//			}
//		}else
//		{
//			if(getenv("CONTENT_LENGTH")){
//				content_len = atoi(getenv("CONTENT_LENGTH"));
//				int i = 0;
//				for( ; i < content_len ; ++i )
//				{
//					read(0,&buf[i],1);
//				}
//				buf[i] = '\0';
//
//				string_arg = buf;
//			}
//		}
//	}
//
//	mymath(string_arg);
//	return 0;
//}



int main()
{
	char method[SIZE];
	char content_len[SIZE];
	char content_data[SIZE];

	if(getenv("METHOD"))         //取出METHOND环境变量
	{
		strcpy(method,getenv("METHOD"));
	}
	else
	{
		printf("cgi: method not exist");
		return 1;
	}

	if(strcasecmp(method,"GET")==0)      //如果是GET方法
	{
		if(getenv("QUERY_STRING"))
		{
			strcpy(content_data,getenv("QUERY_STRING"));
		}
		else
		{
			printf("cgi: query_string not exist");
			return 1;
		}
	}
	else if(strcasecmp(method,"POST")==0)     //如果是post方法，需要先获取conten_len，然后再从管道中读数据
	{
		if(getenv("CONTENT_LEN"))
		{
			strcpy(content_len,getenv("CONTENT_LEN"));
		}
		else
		{
			//	printf("cgi: content_len not exist");
			return 1;
		}

		int i=0;
		char ch='\0';
		for(i=0;i<atoi(content_len);i++)
		{
			read(0,&ch,1);
			content_data[i]=ch;
		}
		content_data[i]='\0';
	}

	int data1=0;
	int data2=0;
	char *arr[3];
	int i=0;
	char* start=content_data;
	while(*start)
	{
		if(*start=='=')
		{
			arr[i++]=start+1;
		}
		else if(*start=='&')
		{
			*start='\0';
		}
		start++;
	}
	data1=atoi(arr[0]);
	data2=atoi(arr[1]);

	//将运算结果返回
	printf("<html>");
	printf("<h2>");
	printf("<tr>%d+%d=%d</tr><br/>",data1,data2,data1+data2);
	printf("<tr>%d-%d=%d</tr><br/>",data1,data2,data1-data2);
	printf("<tr>%d*%d=%d</tr><br/>",data1,data2,data1*data2);
	printf("<tr>%d/%d=%d</tr><br/>",data1,data2,data1/data2);
	printf("<tr>%d%%%d=%d</tr><br/>",data1,data2,data1%data2);
	printf("</h2>");
	printf("</html>");

	return 0;
}

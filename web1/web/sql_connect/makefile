ROOTPATH=$(shell pwd)
MYSQL_INCLUDE=$(ROOTPATH)/lib/include

.PHONY:all
all:insert_cgi select_cgi

insert_cgi:insert_cgi.o sql_api.o
	g++ -o $@ $^
select_cgi:select_cgi.o sql_api.o
	g++ -o $@ $^

%.o:%.c
	g++ -c $< -I$(MYSQL_INCLUDE)

.PHONY:clean
clean:
	rm -f insert_cgi select_cgi *.o


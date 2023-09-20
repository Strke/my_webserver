#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>
#include "locker.h"

class http_conn{

public:
    /*文件名最大长度*/
    static const int FILENAME_LEN = 200;
    /*读缓冲区大小*/
    static const int READ_BUFFER_SIZE = 2048;
    /*写缓冲区大小*/
    static const int WRITE_BUFFER_SIZE = 1024;
    /*HTTP请求方法*/
    enum METHOD{ GET = 0, POST, HEAD, PUT, DELETE,
                TRACE, OPTIONS, CONNECT, PATCH};
    /*解析客户端请求，主状态机有三种状态，
    当前正在分析请求行，当前正在分析头部字段，当前正在分析内容*/
    enum CHECK_STATE{CHECK_STATE_REQUESTLINE = 0,
                     CHECK_STATE_HEADER,
                     CHECK_STATE_CONTENT};
    /*服务器处理结果：NO_REQUEST表示请求不完整，需要继续读取客户数据；GET_REQUEST表示获得了一个完整的客户端请求；
BAD_REQUEST表示客户请求有语法错误；FORBIDDEN_REQUEST表示客户对资源没有足够的访问权限；INTERNAL_ERROR表示服务器内部错误；
 CLOSE_CONNECTION表示客户端已关闭连接*/
    enum HTTP_CODE{NO_REQUEST, GET_REQUEST, BAD_REQUEST,
                   NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST,
                   INTERNAL_ERROR, CLOSED_CONNECTION};
    /*从状态机三种状态，读取完整一行，行出错，行数据读取不完整*/
    enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_OPEN};

public:
    http_conn(){}
    ~http_conn(){}

public:
    /*初始化新建立的连接*/
    void init(int sockfd, const sockaddr_in& addr);
    /*关闭连接*/
    void close_conn(bool real_close = true);
    /*处理客户请求*/
    void process();
    /*非阻塞读操作*/
    bool read();
    /*非阻塞写操作*/
    bool write();

private:
    /*初始化连接*/
    void init();
    /*解析HTTP请求*/
    HTTP_CODE process_read();
    /*填充HTTP应答*/
    bool process_write(HTTP_CODE ret);

    /*以下函数供process_read调用来分析HTTP请求*/
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line(){return m_read_buf + m_start_line;}
    LINE_STATUS parse_line();

    /*以下供process_write调用来填充HTTP应答*/
    void unmap();
    /*往写缓冲中写入待发送的数据*/
    bool add_response(const char* format, ...);
    /*往写缓冲中写入HTTP请求回复的内容*/
    bool add_content(const char* content);
    /*往写缓冲总写入状态行*/
    bool add_status_line(int status, const char* title);
    /*往写缓冲中写入HTTP请求头*/
    bool add_headers(int content_length);
    /*往写缓冲中写入回复内容的长度*/
    bool add_content_length(int content_line);
    bool add_linger();
    bool add_blank_line();

public:
    /*设置为静态变量是因为所有socket上的事件*/
    static int m_epollfd;
    /*用户数量*/
    static int m_user_count;

private:
    /*该HTTP连接的socket和对方的socket地址*/
    int m_sockfd;
    sockaddr_in m_address;



    /*读缓冲区及其相关信息*/
    char m_read_buf[READ_BUFFER_SIZE];
    /*标识读缓冲区中已经读入客户数据的最后一个字节的下一个位置*/
    int m_read_idx;
    /*当前正在分析的字符在缓冲区中的位置*/
    int m_checked_idx;
    /*当前正在解析的行的起始地址*/
    int m_start_line;



    /*写缓冲区及其相关信息*/
    char m_write_buf[WRITE_BUFFER_SIZE];
    /*写缓冲区中待发送的字节数*/
    int m_write_idx;



    /*主状态机相关信息*/
    /*主状态机当前所处的状态*/
    CHECK_STATE m_check_state;
    /*请求方法*/
    METHOD m_method;



    /*客户请求的目标文件的完整路径*/
    char m_real_file[FILENAME_LEN];
    /*客户请求目标文件文件名*/
    char* m_url;
    /*HTTP协议版本号*/
    char* m_version;
    /*主机名*/
    char* m_host;
    /*HTTP请求的消息体的长度*/
    int m_content_length;
    /*HTTP请求是否保持连接*/
    bool m_linger;


    /*客户请求的目标文件被mmap到内存中的起始位置*/
    char* m_file_address;
    /*目标文件的状态*/
    struct stat m_file_stat;
    /*采用writev来执行写操作*/

    /*struct iovec{
        //iov_base是缓冲区开始的地址
        void* iov_base;
        //iov_len是缓冲区的长度信息
        size_t iov_len;
    }*/
    struct iovec m_iv[2];
    //m_iv_count是m_iv内含的缓冲区个数
    int m_iv_count;

};

#endif
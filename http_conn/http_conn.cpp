#include "http_conn.h"
#include <iostream>


/*定义HTTP响应的状态信息*/
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file  from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";

const char* doc_root = "/var/www/html";

/*设置文件描述符为非阻塞*/
int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/*将一个文件描述符加入epoll监听列表, 添加，删除，修改*/
void addfd(int epollfd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if(one_shot){
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
void removefd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}
void modfd(int epollfd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

/*初始化当前连接的用户数量以及事件注册表*/
int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

/*关闭服务器上搭载的连接之一*/
void http_conn::close_conn(bool real_close){
    if(real_close && (m_sockfd != -1)){
        removefd(m_epollfd, m_sockfd);
        /*设置己方sockfd为-1*/
        m_sockfd = -1; 
        /*用户数量减一*/
        m_user_count --;
    }
}


/*初始化服务器*/
void http_conn::init(int sockfd, const sockaddr_in& addr){
    m_sockfd = sockfd;
    m_address = addr;
    /*以下为了避免TIME_WAIT状态
    int reuse = 1;
    setsockpt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    */
    addfd(m_epollfd, sockfd, true);
    m_user_count ++;
    init();
}
/*初始化HTTP请求的相关参数*/
void http_conn::init(){
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;

    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    memset(m_read_buf, '\0', sizeof(m_read_buf));
    memset(m_write_buf, '\0', sizeof(m_write_buf));
    memset(m_real_file, '\0', sizeof(m_real_file));
}



/*从状态机，解析一行*/
http_conn::LINE_STATUS http_conn::parse_line(){
    char temp;
    for(;m_checked_idx < m_read_idx; m_checked_idx ++){
        temp = m_read_buf[m_checked_idx];
        if(temp == '\r'){
            if( (m_checked_idx + 1) == m_read_idx){
                return LINE_OPEN;
            }
            else if(m_read_buf[m_read_idx + 1] == '\n'){
                m_read_buf[m_read_idx ++] = '\0';
                m_read_buf[m_read_idx ++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(temp == '\n'){
            if( (m_checked_idx > 1) && (m_read_buf[m_checked_idx - 1] == '\r')){
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx ++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_BAD;
}
/*从状态机，解析请求行, 获得解决方法，目标URL，以及HTTP版本号*/
http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    /*寻找text第一处含有" \t"的位置并给m_url*/
    m_url = strpbrk(text, " \t");
    if( ! m_url ){
        return BAD_REQUEST;
    }
    *m_url++ = '\0';


    char* method = text;
    /*忽略大小写比较字符串，如果含有GET*/
    if(strcasecmp(method, "GET") == 0){
        m_method = GET;
    }
    else {
        return BAD_REQUEST;
    }
    /*strspn(a, b)返回a中元素第一个不属于b的位置*/
    m_url += strspn(m_url, " \t");


    m_version = strpbrk(m_url, " \t");
    if(! m_version) {
        return BAD_REQUEST;
    }
    *m_version ++ = '\0';
    m_version += strspn(m_version, " \t");


    if(strcasecmp(m_version, "HTTP/1.1" ) != 0) {
        return BAD_REQUEST;
    }
    if(strncasecmp(m_url, "http://", 7) == 0){
        m_url += 7;
        m_url = strchr(m_url, '/');
    }
    if( !m_url || m_url[0] != '/'){
        return BAD_REQUEST;
    }
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}
/*从状态机，解析HTTP请求的一个头部信息*/
http_conn::HTTP_CODE http_conn::parse_headers(char* text){
    /*遇到空格表示头部信息解析完毕*/
    if(text[0] == '\0'){
        /*如果长度还有，说明有content需要读取，状态转移到CHECK_STATE_CONTENT状态*/
        if(m_content_length != 0){
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        /*在这里的话就说明得到的是一个完整的HTTP请求*/
        return GET_REQUEST;
    }
    /*处理connection头部字段*/
    else if(strncasecmp(text, "Connection:", 11) == 0){
        text += 11;
        text += strspn(text, " \t");
        if(strcasecmp(text, "keep-alive") == 0){
            m_linger = true;
        }
    }
    /*处理content-length头部字段*/
    else if(strncasecmp(text, "Content-Length:", 15) == 0){
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    /*处理Host头部字段*/
    else if(strncasecmp(text, "Host:", 5) == 0){
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else{
        std::cout << "oop! unknow header " << text << std::endl;
    }
    return NO_REQUEST;
}
/*此处并不是真正的解析了HTTP请求的消息体，只是判断了该HTTP请求是否被完整的读入*/
http_conn::HTTP_CODE http_conn::parse_content(char* text){
    if(m_read_idx >= (m_content_length + m_checked_idx)){
        text[m_content_length] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

/*主状态机*/
http_conn::HTTP_CODE http_conn::process_read(){
    /*每一行读取的行状态*/
    LINE_STATUS line_status = LINE_OK;
    /*ret为返回的请求结果*/
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;
    /*如果当前主状态机正在读取内容，且从状态机完整读取一行
    或者从状态机完整读取一行*/
    while( ( (m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK) ) || ( ( line_status = parse_line() ) == LINE_OK)){
        text = get_line();
        m_start_line = m_checked_idx;
        std::cout << "got 1 http line: " << text << std::endl;
        /*根据主状态机的状态不同执行不同的操作*/
        switch(m_check_state)
        {
            /*分析请求行*/
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line(text);
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            }
            /*分析头部字段*/
            case CHECK_STATE_HEADER:
            {
                ret = parse_headers(text);
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
                if(ret == GET_REQUEST){
                    return do_request();
                }
                break;
            }
            /*分析内容字段*/
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content(text);
                if(ret == GET_REQUEST){
                    return do_request();
                }
                line_status = LINE_OPEN;
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUEST;
}


/*循环读取客户数据，直到无数据可读或者对方关闭连接*/
bool http_conn::read(){
    if(m_read_idx >= READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read = 0;
    while(true){
        bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        /*对应读取失败的情况*/
        if(bytes_read == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return false;
        }
        /*对应无数据可读的情况*/
        else if(bytes_read == 0){
            return false;
        }
        m_read_idx += bytes_read;
    }
    return true;
}


/*分析HTTP请求目标文件的属性，如果该文件存在、对所有用户可见且不是目录，则
使用mmap将该文件映射到m_file_address处*/
http_conn::HTTP_CODE http_conn::do_request(){
    strcpy(m_real_file, doc_root);
    int len = strlen(doc_root);
    strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
    /*捕获文件信息到m_file_stat，成功返回0， 失败返回-1*/
    if(stat(m_real_file, &m_file_stat) < 0){
        return NO_REQUEST;
    }
    /*该文件模式为其他组读权限时*/
    if(!(m_file_stat.st_mode & S_IROTH)){
        return FORBIDDEN_REQUEST;
    }
    /*该文件为目录时*/
    if(S_ISDIR(m_file_stat.st_mode)){
        return BAD_REQUEST;
    }
    /*将请求的该文件以只读方式映射进内存*/
    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char*)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}
/*释放共享内存*/
void http_conn::unmap(){
    if(m_file_address){
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}

bool http_conn::write(){
    int temp = 0;
    int bytes_have_send = 0;
    int bytes_to_send = m_write_idx;
    if(bytes_to_send == 0){
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        init();
        return true;
    }
    while(1){
        /*成功时temp为写的总字数，失败时temp为-1*/
        temp = writev(m_sockfd, m_iv, m_iv_count);
        /*如果写操作失败*/
        if(temp <= -1){
            /*此处EAGAIN表示缓冲区不可写
            如果写缓冲满，则等待下一轮的EPOLLOUT事件*/
            if(errno == EAGAIN){
                /*修改m_sockfd在epoll中的行为*/
                modfd(m_epollfd, m_sockfd, EPOLLOUT);
                return true;
            }
            unmap();
            return false;
        }

        bytes_to_send -= temp;
        bytes_have_send += temp;
        if(bytes_to_send <= bytes_have_send){
            unmap();
            /*更具connection字段的值来判断是否保持连接*/
            if(m_linger){
                init();
                modfd(m_epollfd, m_sockfd, EPOLLIN);
                return true;
            }
            else{
                modfd(m_epollfd, m_sockfd, EPOLLIN);
                return false;
            }
        }
    }
}
/*往写缓冲中写入待发送的数据*/
bool http_conn::add_response(const char* format, ...){
    if(m_write_idx >= WRITE_BUFFER_SIZE){
        return false;
    }

    /*va_list是用来解决变参问题的的宏*/
    va_list arg_list;
    /*va_start用来初始化va_list变量，
      第二个参数填最后一个具名的参数，这里是format*/
    va_start(arg_list, format);
    /*指明存储位置以及存储大小，
      在这里vsnprintf将format和arg_list组合成一个字符串输出*/
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx,
                         format, arg_list);
    if(len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx)){
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    return true;
}

bool http_conn::add_status_line(int status, const char* title){
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
/*将各类信息组成头文件*/
bool http_conn::add_headers(int content_len){
    add_content_length(content_len);
    add_linger();
    add_blank_line();
}
bool http_conn::add_content_length(int content_len){
    return add_response("Content-Length: %d\r\n", content_len);
}
/*添加连接信息*/
bool http_conn::add_linger(){
    return add_response("Connection: %s\r\n", (m_linger == true)? "keep-alive" : "close");
}
/*写入每行的回车换行符，HTTP头中以回车换行符(\r\n)结束*/
bool http_conn::add_blank_line(){
    return add_response("%s", "\r\n");
}
/*写入每行的内容*/
bool http_conn::add_content(const char* content){
    return add_response("%s", content);
}
/*根据请求，决定返回的内容*/
bool http_conn::process_write(HTTP_CODE ret){
    switch(ret)
    {
        case INTERNAL_ERROR:
        {
            add_status_line(500, error_500_title);
            add_headers(strlen(error_500_form));
            if(!add_content(error_500_form)){
                return false;
            }
            break;
        }
        case BAD_REQUEST:
        {
            add_status_line(400, error_400_title);
            add_headers(strlen(error_400_form));
            if(! add_content(error_400_form)){
                return false;
            }
            break;
        }
        case NO_RESOURCE:
        {
            add_status_line(404, error_404_title);
            add_headers(strlen(error_404_form));
            if(! add_content(error_404_form)){
                return false;
            }
            break;
        }
        case FORBIDDEN_REQUEST:
        {
            add_status_line(403, error_403_title);
            add_headers(strlen(error_403_form));
            if(! add_content(error_403_form)){
                return false;
            }
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line(200, ok_200_title);
            if(m_file_stat.st_size != 0){
                add_headers(m_file_stat.st_size);
                m_iv[0].iov_base = m_write_buf;
                m_iv[0].iov_len = m_write_idx;
                m_iv[1].iov_base = m_file_address;
                m_iv[1].iov_len = m_file_stat.st_size;
                m_iv_count = 2;
                return true;
            }
            else{
                const char* ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if(!add_content(ok_string)){
                    return false;
                }
            }
        }
        default:
        {
            return false;
        }
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    return true;
}


void http_conn::process(){
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_REQUEST){
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }
    bool write_ret = process_write(read_ret);
    if(! write_ret){
        close_conn();
    }
    modfd(m_epollfd, m_sockfd, EPOLLOUT);
}
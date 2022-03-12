#ifndef SERVER_HTTP_H
#define SERVER_HTTP_H

#include <netinet/in.h>
#include <sys/stat.h>

class HttpConnection
{
public:
	const static int FILENAME_LEN = 200;
	const static int READ_BUFFER_SIZE = 2048;
	const static int WRITE_BUFFER_SIZE = 1024;
    enum class Method
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum class Code
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum class LineStatus
    {
        OK = 0,
        BAD,
        OPEN
    };


public:
    void closeConn();
    Code processRead();
    bool processWrite(Code res);
    void process();
    bool read();
    bool write();

    sockaddr_in* address() {
        return &address_;
    }

    HttpConnection(int sockfd, const sockaddr_in& addr, char* root, bool et, int closeLog, std::string user, std::string passwd, std::string sqlname);
    ~HttpConnection() {}

private:
    Code parseRequestLine(char* text);
    Code parseHeader(char* text);
    Code parseContent(char* text);
    LineStatus parseLine();

    int sockfd_;
    sockaddr_in address_;
    char readBuf_[READ_BUFFER_SIZE];
    int readIdx_;
    char writeBuf_[WRITE_BUFFER_SIZE];
    int writeIdx_;
    int checkedIdx_;
    int checkState_;
    Method method_;
    char file_[FILENAME_LEN];
    char* url_;
    char* version_;
    char* host_;
    int userCount_;
    int epollfd_;
    int contentLength_;
    char* fileAddress;
    struct stat fileStat_;
    iovec iv_[2];
    int ivCount_;
    char* header_; //�洢����ͷ����
    int bytesToSend_;
    int bytesSent_;
    char* docRoot_;
    bool et_;
    int closeLog_;
    bool longConn_;
    int startLine_;
    int state_;

    char sqlUser_[100];
    char sqlPasswd_[100];
    char sqlName_[100];
};
#endif // !SERVER_HTTP_H

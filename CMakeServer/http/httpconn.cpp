#include <sys/epoll.h>
#include "httpconn.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file form this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the request file.\n";

void addfd(int epollfd, int fd, bool oneShot, bool et) {
	epoll_event event;
    event.data.fd = fd;

    if (et)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (oneShot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    int flag = fcntl(fd, F_GETFL);
    flag = flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

HttpConnection::HttpConnection(int sockfd, const sockaddr_in& addr, char* root, bool et, int closeLog, std::string user, std::string passwd, std::string sqlname) {
    sockfd_ = sockfd;
    address_ = addr;
    addfd(epollfd_, sockfd, true, et);
    userCount_ = 1;
    epollfd_ = -1;

    docRoot_ = root;
    et_ = et;
    closeLog_ = closeLog;

    strcpy(sqlUser_, user.c_str());
    strcpy(sqlPasswd_, passwd.c_str());
    strcpy(sqlName_, sqlname.c_str());

    bytesToSend_ = 0;
    bytesSent_ = 0;
    longConn_ = false;
    method_ = Method::GET;
    url_ = 0;
    version_ = 0;
    contentLength_ = 0;
    host_ = 0;
    startLine_ = 0;
    readIdx_ = 0;
    writeIdx_ = 0;
    state_ = 0;

    memset(readBuf_, '\0', READ_BUFFER_SIZE);
    memset(writeBuf_, '\0', WRITE_BUFFER_SIZE);
    memset(file_, '\0', FILENAME_LEN);
}

void HttpConnection::closeConn() {
    if (sockfd_ != -1) {
        printf("close %d\n", sockfd_);
        removefd(epollfd_, sockfd_);
        sockfd_ = -1;
        userCount_--;
    }
}

bool HttpConnection::read() {
    if (readIdx_ >= READ_BUFFER_SIZE) {
        return false;
    }

    if (et_) {
        while (true) {
            int readBytes = recv(sockfd_, readBuf_ + readIdx_, READ_BUFFER_SIZE - readIdx_, 0);
            if (readBytes == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                return false;
            }
            else if (readBytes == 0) {
                return false;
            }
            readIdx_ += readBytes;
        }
        return true;
    }
    else {
        int readBytes = recv(sockfd_, readBuf_ + readIdx_, READ_BUFFER_SIZE - readIdx_, 0);
        readIdx_ += readBytes;

        if (readBytes <= 0) return false;
        return true;
    }
}
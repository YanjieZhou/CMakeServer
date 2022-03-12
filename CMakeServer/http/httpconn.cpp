#include <sys/epoll.h>
#include "httpconn.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "../log/log.h"

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

void modfd(int epollfd, int fd, int ev, bool et) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    if (et) event.events |= EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
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
    checkedIdx_ = 0;
    state_ = 0;
    checkState_ = 1;

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

HttpConnection::Code HttpConnection::parseRequestLine(char* text) {
    url_ = strpbrk(text, " \t");
    if (!url_) {
        return Code::BAD_REQUEST;
    }

    *url_++ = '\0';
    char* method = text;
    if (strcasecmp(method, "GET") == 0) {
        method_ = Method::GET;
    }
    else if (strcasecmp(method, "POST") == 0) {
        method_ = Method::POST;
    }
    else return Code::BAD_REQUEST;

    url_ += strspn(url_, " \t");
    version_ = strpbrk(url_, " \t");
    if (!version_) {
        return Code::BAD_REQUEST;
    }
    *version_++ = '\0';
    version_ += strspn(version_, " \t");
    if (strncasecmp(url_, "http://", 7) == 0) {
        url_ += 7;
        url_ = strchr(url_, '/');
    }
    if (strncasecmp(url_, "https://", 8) == 0) {
        url_ += 8;
        url_ = strchr(url_, '/');
    }
    if (!url_ || url_[0] != '/') {
        return Code::BAD_REQUEST;
    }
    checkState_ = 2;
    return Code::NO_REQUEST;
}

HttpConnection::Code HttpConnection::parseHeader(char* text) {
    if (text[0] = '\0') {
        if (contentLength_ != 0) {
            checkState_ = 3;
            return Code::NO_REQUEST;
        }
        return Code::GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11)) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            longConn_ = true;
        }
        else if (strncasecmp(text, "Content-length:", 15) == 0)
        {
            text += 15;
            text += strspn(text, " \t");
            contentLength_ = atol(text);
        }
        else if (strncasecmp(text, "Host:", 5) == 0)
        {
            text += 5;
            text += strspn(text, " \t");
            host_ = text;
        }
        else
        {
            LOG_INFO("oop!unknow header: %s", text);
        }
        return Code::NO_REQUEST;
    }
}

HttpConnection::Code HttpConnection::parseContent(char* text) {
    if (readIdx_ > (contentLength_ + checkedIdx_)) {
        text[contentLength_] = '\0';
        header_ = text;
        return Code::GET_REQUEST;
    }
    return Code::NO_REQUEST;
}
HttpConnection::LineStatus HttpConnection::parseLine()
{
    char temp;
    for (; checkedIdx_ < readIdx_; ++checkedIdx_)
    {
        temp = readBuf_[checkedIdx_];
        if (temp == '\r')
        {
            if ((checkedIdx_ + 1) == readIdx_)
                return LineStatus::OPEN;
            else if (readBuf_[checkedIdx_ + 1] == '\n')
            {
                readBuf_[checkedIdx_++] = '\0';
                readBuf_[checkedIdx_++] = '\0';
                return LineStatus::OK;
            }
            return LineStatus::BAD;
        }
        else if (temp == '\n')
        {
            if (checkedIdx_ > 1 && readBuf_[checkedIdx_ - 1] == '\r')
            {
                readBuf_[checkedIdx_ - 1] = '\0';
                readBuf_[checkedIdx_++] = '\0';
                return LineStatus::OK;
            }
            return LineStatus::OK;
        }
    }
    return LineStatus::OPEN;
}

HttpConnection::Code HttpConnection::processRead() {
    Code res = Code::NO_REQUEST;
    LineStatus lineStatus = LineStatus::OK;
    char* text = 0;
    while (lineStatus == LineStatus::OK) {
        text = readBuf_ + startLine_;
        startLine_ = checkedIdx_;
        LOG_INFO("%s", text);
        switch (checkState_) {
        case 1:
        {
            res = parseRequestLine(text);
            if (res == Code::BAD_REQUEST) return res;
            break;
        }
        case 2:
        {
            res = parseHeader(text);
            if (res == Code::BAD_REQUEST) return res;
            else if (res == Code::GET_REQUEST) {
                return res;
            }
            break;
        }
        case 3:
        {
            res = parseContent(text);
            if (res == Code::GET_REQUEST) return res;
            lineStatus = LineStatus::OPEN;
            break;
        }
        default:
            return Code::INTERNAL_ERROR;
        }  
    }
    return Code::NO_REQUEST;
}

void HttpConnection::process() {
    Code readRes = processRead();
    if (readRes == Code::NO_REQUEST) {
        modfd(epollfd_, sockfd_, EPOLLIN, et_);
        return;
    }
    modfd(epollfd_, sockfd_, EPOLLOUT, et_);
}


#include <iostream>
#include <sys/socket.h>
#include <vector>
#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <climits>
#include <fstream>
#include <unistd.h>
#define MAGIC_NUMBER_LENGTH 6
#define IDLE 0
#define Open_conn 1
#define Auth 2
#define MAIN 3
#define OPEN_CONN_REQUEST (char(0xA1))
#define OPEN_CONN_REPLY (char(0xA2))
#define AUTH_REQUEST (char(0xA3))
#define AUTH_REPLY (char(0xA4))
#define LIST_REQUEST (char(0xA5))
#define LIST_REPLY (char(0xA6))
#define GET_REQUEST (char(0xA7))
#define GET_REPLY (char(0xA8))
#define FILE_DATA (char(0xFF))
#define PUT_REQUEST (char(0xA9))
#define PUT_REPLY (char(0xAA))
#define QUIT_REQUEST (char(0xAB))
#define QUIT_REPLY (char(0xAC))
#define INIT (char(0x00))
using namespace std;
const char myftp_str[MAGIC_NUMBER_LENGTH] = {'\xe3', 'm', 'y', 'f', 't', 'p'};
class myFTP_Header
{
public:
    char m_protocol[MAGIC_NUMBER_LENGTH]; /* protocol magic number (6 bytes) */
    char m_type;                          /* type (1 byte) */
    char m_status;                        /* status (1 byte) */
    uint32_t m_length;                    /* length (4 bytes) in Big endian*/
    myFTP_Header()
    {
        memcpy(m_protocol, myftp_str, 6);
    }
} __attribute__((packed));
struct __client__
{
    int conn_fd;
    int status;
};
class myFTP_Header_and_Data
{
public:
    myFTP_Header header;
    char payload[INT_MAX];
    myFTP_Header_and_Data(char type)
    {
        header.m_type = type;
    }
} __attribute__((packed));
ssize_t send_myftp(int sock_fd, void *buf, int len, int flags)
{
    ssize_t ret = 0;
    while (ret < len)
    {
        ssize_t b = send(sock_fd, buf + ret, len - ret, 0);
        if (b == 0)
        {
            printf("socket closed\n");
            return b;
        }
        if (b < 0)
        {
            printf("error\n");
            return b;
        }
        ret += b;
    }
    return ret;
}
ssize_t recv_myftp(int sock_fd, void *buf, int len, int flags)
{
    ssize_t ret = 0;
    bool recv_len = false;
    while (ret < 12)
    {
        ssize_t b = recv(sock_fd, buf + ret, len - ret, flags);
        if (b == 0)
        {
            printf("socket closed\n");
            return b;
        }
        if (b < 0)
        {
            printf("error\n");
            return b;
        }
        ret += b;
    }
    len = ntohl(reinterpret_cast<myFTP_Header_and_Data *>(buf)->header.m_length);
    while (ret < len)
    {
        ssize_t b = recv(sock_fd, buf + ret, len - ret, flags);
        if (b == 0)
        {
            printf("socket closed\n");
            return b;
        }
        if (b < 0)
        {
            printf("error\n");
            return b;
        }
        ret += b;
    }
    return ret;
}
myFTP_Header_and_Data *open_conn_reply(char status)
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(OPEN_CONN_REPLY);
    myftp->header.m_length = htonl(12);
    myftp->header.m_status = status;
    return myftp;
}
myFTP_Header_and_Data *auth_reply(char status)
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(AUTH_REPLY);
    myftp->header.m_length = htonl(12);
    myftp->header.m_status = status;
    return myftp;
}
myFTP_Header_and_Data *list_reply()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(LIST_REPLY);
    FILE *file = NULL;
    char buf[4096];
    file = popen("ls", "r");
    int len = fread(myftp->payload, 1, 4095, file);
    myftp->payload[len] = '\0';
    myftp->header.m_length = htonl(12 + len + 1);
    if (fclose(file) != 0)
        assert(false);
    return myftp;
}
myFTP_Header_and_Data *get_reply(char status)
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(GET_REPLY);
    myftp->header.m_length = htonl(12);
    myftp->header.m_status = status;
    return myftp;
}
myFTP_Header_and_Data *file_data(char *file_name)
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(FILE_DATA);
    FILE *file = fopen(file_name, "rb");
    int len = fread(myftp->payload, 1, INT_MAX, file);
    myftp->header.m_length = htonl(len + 12);
    return myftp;
}
myFTP_Header_and_Data *put_reply()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(PUT_REPLY);
    myftp->header.m_length = htonl(12);
    return myftp;
}
myFTP_Header_and_Data *quit_reply()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(QUIT_REPLY);
    myftp->header.m_length = htonl(12);
    return myftp;
}
bool check_open_conn_request(myFTP_Header_and_Data *recv_ptr)
{
    if (recv_ptr->header.m_length == htonl(12) && memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 && recv_ptr->header.m_type == char(0xA1))
        return true;
    else
        return false;
}
bool check_auth_request(myFTP_Header_and_Data *recv_ptr)
{
    if (memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) != 0 || recv_ptr->header.m_type != char(0xA3) || ntohl(recv_ptr->header.m_length) <= 13)
        return false;
    string usr = "";
    string psw = "";
    char temp = '\0';
    char *rd = recv_ptr->payload;
    int rd_len = 0;
    bool rd_str = false;
    printf("line 131:%d\n", ntohl(recv_ptr->header.m_length) - 13);
    printf("line 132:%s\n", rd);
    vector<string> vec_str;
    while (rd_len < ntohl(recv_ptr->header.m_length) - 13)
    {
        temp = rd[rd_len];
        if (temp == '\0')
            break;
        if (temp == ' ')
        {
            rd_str = false;
            ++rd_len;
            continue;
        }
        if (rd_str == false)
        {
            vec_str.push_back("");
            rd_str = true;
        }
        ++rd_len;
        vec_str.back() += temp;
    }
    if (vec_str.size() != 2)
    {
        printf("error: please enter two parameters for username and password\n");
        return false;
    }
    if (vec_str[0] == "user" && vec_str[1] == "123123")
        return true;
    else
    {
        printf("error: wrong username or password\n");
        return false;
    }
}
bool check_list_request(myFTP_Header_and_Data *recv_ptr)
{
    if (recv_ptr->header.m_length == htonl(12) && memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 && recv_ptr->header.m_type == char(0xA5))
        return true;
    else
        return false;
}
bool check_get_request(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) >= 13 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == char(0xA7))
        return true;
    else
        return false;
}
bool check_put_request(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) >= 13 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == char(0xA9))
        return true;
    else
        return false;
}
bool check_quit_request(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) == 12 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == char(0xAB))
        return true;
    else
        return false;
}
char *return_file_name(myFTP_Header_and_Data *recv_ptr)
{
    // int len = ntohl(recv_ptr->header.m_length) - 13;
    return recv_ptr->payload;
}
bool check_file_exist(char *file_name)
{
    char buf[4096];
    FILE *file = popen("ls", "r");
    int len = fread(buf, 1, 4095, file);
    buf[len] = '\0';
    int start = 0;
    int end = 0;
    int pos = 0;
    while (pos <= len)
    {
        while (buf[pos] != '\0' && buf[pos] != '\n')
            ++pos;
        end = pos;
        if (memcmp(file_name, buf + start, end - start) == 0)
            return true;
        ++pos;
        start = pos;
    }
    return false;
}
vector<__client__> vec_client;
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("%s\n", argv[0]);
        assert(false);
    }
    int listen_fd = 0, conn_fd = 0;
    int b = 0;
    socklen_t client_len = 0;
    struct sockaddr_in client_addr, server_addr;
    myFTP_Header_and_Data *send_ptr = NULL;
    myFTP_Header_and_Data *recv_ptr = new myFTP_Header_and_Data(INIT);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    printf("%s %d\n", argv[1], atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_fd, 128);
    client_len = sizeof(client_addr);
    while (conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len))
    {
        vec_client.push_back(__client__());
        vec_client.back().conn_fd = conn_fd;
        vec_client.back().status = Open_conn;

        // debug
        int cycle = 1;

        // debug

        while ((b = recv_myftp(conn_fd, (void *)recv_ptr, INT_MAX, 0)) > 0)
        {
            printf("cycle %d: status %d\n", cycle, vec_client.back().status);
            printf("%d\n", b);
            printf("%d\n", ntohl(reinterpret_cast<myFTP_Header_and_Data *>(recv_ptr)->header.m_length));
            if (vec_client.back().status == Open_conn)
            {
                // vec_client.back().status = Open_conn;
                if (check_open_conn_request(recv_ptr) == false)
                {
                    send_ptr = open_conn_reply(char(0));
                    printf("no\n");
                }
                else
                {
                    send_ptr = open_conn_reply(char(1));
                    vec_client.back().status = Auth;
                    printf("receive open_connection, need to auth\n");
                }
                send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                delete (send_ptr);
                send_ptr = NULL;
                ++cycle;
                continue;
            }
            if (vec_client.back().status == Auth)
            {
                if (check_auth_request(recv_ptr) == true)
                {
                    send_ptr = auth_reply(char(1));
                    printf("auth successfully\n");
                    vec_client.back().status = MAIN;
                    send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                    delete (send_ptr);
                    send_ptr = NULL;
                    ++cycle;
                    continue;
                }
                else
                {
                    printf("auth failed\n");
                    send_ptr = auth_reply(char(0));
                    vec_client.back().status = IDLE;
                    send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                    if (close(conn_fd) == -1)
                    {
                        printf("close socket failed\n");
                        assert(false);
                    }
                    delete (send_ptr);
                    send_ptr = NULL;
                    ++cycle;
                    continue;
                }
            }
            if (vec_client.back().status == MAIN)
            {
                if (check_list_request(recv_ptr) == true)
                {
                    printf("receive check_list_request\n");
                    send_ptr = list_reply();
                    send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                    delete (send_ptr);
                    send_ptr = NULL;
                    ++cycle;
                    continue;
                }
                if (check_get_request(recv_ptr) == true)
                {
                    printf("receive get_request\n");
                    char *file_name = return_file_name(recv_ptr);
                    printf("file name is %s\n", file_name);
                    if (check_file_exist(file_name))
                    {
                        send_ptr = get_reply(char(1));
                        send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                        delete (send_ptr);
                        send_ptr = file_data(file_name);
                        send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                        delete (send_ptr);
                        send_ptr = NULL;
                        ++cycle;
                        continue;
                    }
                    else
                    {
                        printf("file does not exists\n");
                        send_ptr = get_reply(char(0));
                        send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                        delete (send_ptr);
                        send_ptr = NULL;
                        ++cycle;
                        continue;
                    }
                }
                if (check_put_request(recv_ptr) == true)
                {
                    printf("receive put_request\n");
                    char *file_name = new char[strlen(return_file_name(recv_ptr))];
                    strcpy(file_name, return_file_name(recv_ptr));
                    send_ptr = put_reply();
                    send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                    delete (send_ptr);
                    send_ptr = NULL;
                    recv_myftp(conn_fd, (void *)recv_ptr, INT_MAX, 0);
                    FILE *file = fopen(file_name, "wb");
                    fwrite(recv_ptr->payload, sizeof(char), ntohl(recv_ptr->header.m_length) - 12, file);
                    if (fclose(file) != 0)
                        assert(false);
                    delete[] file_name;
                    ++cycle;
                    continue;
                }
                if (check_quit_request(recv_ptr) == true)
                {
                    printf("receive quit_request\n");
                    send_ptr = quit_reply();
                    send_myftp(conn_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                    if (close(conn_fd) == -1)
                        assert(false);
                    break;
                }
            }
        }
    }
    return 0;
}
#include <iostream>
#include <string.h>
#include <vector>
#include <climits>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define IDLE 0
#define Open_conn 1
#define Auth 2
#define MAIN 3
#define MAGIC_NUMBER_LENGTH 6
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
vector<string> cmd;
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
void read_cmd()
{
    char temp = '\0';
    cmd.clear();
    string str = "";
    while ((temp = getchar()) == ' ')
    {
    }
    if (temp == '\n')
        return;
    str += temp;
    while ((temp = getchar()) != '\n')
    {
        if (temp == ' ')
        {
            cmd.push_back(str);
            str = "";
            continue;
        }
        str += temp;
    }
    cmd.push_back(str);
    return;
}
myFTP_Header_and_Data *open_conn_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(OPEN_CONN_REQUEST);
    myftp->header.m_length = htonl(12);
    return myftp;
}
myFTP_Header_and_Data *auth_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(AUTH_REQUEST);
    strcpy(myftp->payload, (cmd[1] + ' ' + cmd[2]).c_str());
    myftp->header.m_length = htonl(12 + strlen(myftp->payload) + 1);
    return myftp;
}
myFTP_Header_and_Data *list_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(LIST_REQUEST);
    myftp->header.m_length = htonl(12);
    return myftp;
}
myFTP_Header_and_Data *get_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(GET_REQUEST);
    myftp->header.m_length = htonl(13 + cmd[1].length());
    printf("%s\n", cmd[1].c_str());
    if (strcpy(myftp->payload, cmd[1].c_str()) == NULL)
    {
        printf("error: strcpy failed\n");
        assert(false);
    }
    return myftp;
}
myFTP_Header_and_Data *put_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(PUT_REQUEST);
    myftp->header.m_length = htonl(13 + cmd[1].length());
    if (strcpy(myftp->payload, cmd[1].c_str()) == NULL)
    {
        printf("error: strcpy failed\n");
        assert(false);
    }
    return myftp;
}
myFTP_Header_and_Data *file_data(const char *file_name)
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(FILE_DATA);
    FILE *file = fopen(file_name, "rb");
    int len = fread(myftp->payload, 1, INT_MAX, file);
    myftp->header.m_length = htonl(len + 12);
    return myftp;
}
myFTP_Header_and_Data *quit_request()
{
    myFTP_Header_and_Data *myftp = new myFTP_Header_and_Data(QUIT_REQUEST);
    myftp->header.m_length = htonl(12);
    return myftp;
}
bool check_open_conn_reply(myFTP_Header_and_Data *recv_ptr)
{
    if (recv_ptr->header.m_length == htonl(12) &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == OPEN_CONN_REPLY && recv_ptr->header.m_status == 1)
        return true;
    else
        return false;
}
bool check_auth_reply(myFTP_Header_and_Data *recv_ptr) // do not check status
{
    if (recv_ptr->header.m_length == htonl(12) &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == AUTH_REPLY)
        return true;
    else
        return false;
}
bool check_get_reply(myFTP_Header_and_Data *recv_ptr)
{
    if (recv_ptr->header.m_length == htonl(12) &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == GET_REPLY)
    {
        if (recv_ptr->header.m_status == char(0) || recv_ptr->header.m_status == char(1))
            return true;
        else
            return false;
    }
    else
        return false;
}
bool check_put_reply(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) == 12 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == PUT_REPLY)
        return true;
    else
        return false;
}
bool check_file_data(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) >= 12 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == FILE_DATA)
        return true;
    else
        return false;
}
bool check_quit_reply(myFTP_Header_and_Data *recv_ptr)
{
    if (ntohl(recv_ptr->header.m_length) == 12 &&
        memcmp(recv_ptr->header.m_protocol, myftp_str, MAGIC_NUMBER_LENGTH) == 0 &&
        recv_ptr->header.m_type == QUIT_REPLY)
        return true;
    else
        return false;
}
__client__ client;
int main(int argc, char **argv)
{
    int socket_fd;
    struct sockaddr_in server_addr;
    myFTP_Header_and_Data *send_ptr = NULL;
    myFTP_Header_and_Data *recv_ptr = new myFTP_Header_and_Data(INIT);
    client.status = IDLE;
    while (true)
    {
        if (client.status == IDLE)
        {
            client.status = Open_conn;
            continue;
        }
        printf("Client> ");
        if (client.status == Open_conn)
        {
            read_cmd();
            if (cmd[0] != "open")
            {
                printf("error: need to open connection first\n");
                continue;
            }
            if (cmd.size() != 3)
            {
                printf("error: please enter 2 parameters for open connection\n");
                continue;
            }
            if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
                printf("error: create socket failed\n");
                continue;
            }
            bzero(&server_addr, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(atoi(cmd[2].c_str()));
            inet_pton(AF_INET, cmd[1].c_str(), &server_addr.sin_addr);
            if ((connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1)
            {
                printf("error: connect failed;please check the parameters you entered\n");
                continue;
            }
            client.conn_fd = socket_fd;
            send_ptr = open_conn_request();
            if ((send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0)) == -1)
            {
                printf("error: send packet failed\n");
                continue;
            }
            delete (send_ptr);
            send_ptr = NULL;
            if ((recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0)) == -1)
            {
                printf("error: receive packet failed\n");
                continue;
            }
            if (check_open_conn_reply(recv_ptr))
            {
                client.status = Auth;
                printf("open connection successfully\n");
                continue;
            }
            else
            {
                printf("open connection failed\n");
                continue;
            }
        }
        if (client.status == Auth)
        {
            read_cmd();
            send_ptr = auth_request();
            if ((send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0)) == -1)
            {
                printf("error: send packet failed\n");
                continue;
            }
            delete (send_ptr);
            send_ptr = NULL;
            if ((recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0)) == -1)
            {
                printf("error: receive packet failed\n");
                continue;
            }
            if (check_auth_reply(recv_ptr) == true)
            {
                if (recv_ptr->header.m_status == char(1))
                {
                    printf("auth successfully\n");
                    client.status = MAIN;
                    continue;
                }
                else if (recv_ptr->header.m_status == char(0))
                {
                    printf("auth failed; close connection\n");
                    if (close(socket_fd) == -1)
                    {
                        printf("close socket failed\n");
                        assert(false);
                    }
                    client.status = IDLE;
                    continue;
                }
                else
                {
                    printf("error: undefined status for auth reply\n");
                    assert(false);
                }
            }
            else
                assert(false);
        }
        if (client.status == MAIN)
        {
            read_cmd();
            if (cmd.size() == 0)
            {
                continue;
            }
            if (cmd[0] == "ls")
            {
                if (cmd.size() > 1)
                {
                    printf("error:ls order should have no parameters\n");
                    continue;
                }
                send_ptr = list_request();
                if ((send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0)) == -1)
                {
                    printf("error: send packet failed\n");
                    continue;
                }
                delete (send_ptr);
                send_ptr = NULL;
                if ((recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0)) == -1)
                {
                    printf("error: receive packet failed\n");
                    continue;
                }
                printf("###################### file list start ######################\n%s###################### file list end ######################\n", reinterpret_cast<char *>(recv_ptr->payload));
                continue;
            }
            if (cmd[0] == "get")
            {
                if (cmd.size() > 2)
                {
                    printf("error: get command should have only one parameter\n");
                    continue;
                }
                send_ptr = get_request();
                if ((send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0)) == -1)
                {
                    printf("error: send packet failed\n");
                    continue;
                }
                delete (send_ptr);
                send_ptr = NULL;
                recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0);
                if (check_get_reply(recv_ptr) == false)
                {
                    printf("error: receive wrong packet\n");
                    continue;
                }
                if (recv_ptr->header.m_status == char(0))
                {
                    printf("error: file does not exists\n");
                    continue;
                }
                else if (recv_ptr->header.m_status == char(1))
                {
                    recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0);
                    if (check_file_data(recv_ptr) == false)
                    {
                        printf("error: receive wrong packet\n");
                        continue;
                    }
                    printf("%s\n", cmd[1].c_str());
                    FILE *file = fopen(cmd[1].c_str(), "wb");
                    fwrite(recv_ptr->payload, sizeof(char), ntohl(recv_ptr->header.m_length) - 12, file);
                    if (fclose(file) != 0)
                        assert(false);
                    continue;
                }
                else
                    assert(false);
            }
            if (cmd[0] == "put")
            {
                if (cmd.size() > 2)
                {
                    printf("error: put command should have only one parameter\n");
                    continue;
                }
                send_ptr = put_request();
                printf("%s\n", cmd[1].c_str());
                send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                delete send_ptr;
                send_ptr = NULL;
                recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0);
                if (check_put_reply(recv_ptr) == false)
                {
                    printf("error: receive wrong packet\n");
                    continue;
                }
                send_ptr = file_data(cmd[1].c_str());
                send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                continue;
            }
            if (cmd[0] == "quit")
            {
                if (cmd.size() > 1)
                {
                    printf("error:ls order should have no parameters\n");
                    continue;
                }
                send_ptr = quit_request();
                send_myftp(socket_fd, (void *)send_ptr, ntohl(send_ptr->header.m_length), 0);
                delete send_ptr;
                send_ptr = NULL;
                recv_myftp(socket_fd, (void *)recv_ptr, INT_MAX, 0);
                if (check_quit_reply(recv_ptr) == false)
                {
                    printf("error: receive wrong packet\n");
                    continue;
                }
                if (close(socket_fd) == -1)
                {
                    printf("close socket failed\n");
                    assert(false);
                }
                client.status = IDLE;
                printf("client quit\n");
                break;
            }
        }
    }
    return 0;
}
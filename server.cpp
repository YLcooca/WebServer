#include <arpa/inet.h>  // sockaddr_in
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>  // socket(),
#include <cstring>  // memset()

int main() {
  // 使用TCP协议
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // 将IP和PORT绑定到server_addr中
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8888);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // 将sockfd和server_addr进行绑定
  int ret = bind(sockfd, reinterpret_cast<sockaddr*>(&server_addr),
                 sizeof(server_addr));

  // 开始监听，并且设置最大监听队列长度为SOMAXCONN(4096)
  listen(sockfd, SOMAXCONN);

  // 调用accept从监听队列中取出客户端信息
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  memset(&client_addr, 0, sizeof(client_addr));
  int client_fd = accept(sockfd, reinterpret_cast<sockaddr*>(&client_addr),
                         &client_addr_len);
  printf("new client fd %d! Ip: %s, Port: %d\n", client_fd,
         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

  return 0;
}
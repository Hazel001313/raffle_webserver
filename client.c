#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define SERV_IP "9.135.13.170"
#define SERV_PORT 7777

int main(void)
{
    int sfd, len;
    struct sockaddr_in serv_addr;
    char buf[BUFSIZ]; 
    int sfds[10]={0};

    for(int i=0;i<10;i++){
        sfds[i] = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&serv_addr, sizeof(serv_addr));                       
        serv_addr.sin_family = AF_INET;                             
        inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);    
        serv_addr.sin_port = htons(SERV_PORT);                      

        connect(sfds[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        printf("%d\n",sfds[i]);
    }
    // sfd = socket(AF_INET, SOCK_STREAM, 0);

    // bzero(&serv_addr, sizeof(serv_addr));                       
    // serv_addr.sin_family = AF_INET;                             
    // inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);    
    // serv_addr.sin_port = htons(SERV_PORT);                      

    // connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    strcpy(buf,"PING");
    for(int i=0;i<10;i++){
        int r = write(sfds[i], buf, strlen(buf));       
        printf("Write r ======== %d\n", r);
    }
    for(int i=0;i<10;i++){
        len = read(sfd, buf, sizeof(buf));
        printf("Read len ========= %d\n", len);
        write(STDOUT_FILENO, buf, len);
    }
    // while (1) {
    //     fgets(buf, sizeof(buf), stdin);
    //     int r = write(sfd, buf, strlen(buf));       
    //     printf("Write r ======== %d\n", r);
    //     len = read(sfd, buf, sizeof(buf));
    //     printf("Read len ========= %d\n", len);
    //     write(STDOUT_FILENO, buf, len);
    // }

    close(sfd);

    return 0;
}


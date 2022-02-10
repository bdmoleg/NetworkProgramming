#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <WinSock2.h>

char * concat(const char * a, int sizeContent, const char * b, int sizechunk){

    // lucru cu octetii si strlen nu chiar se leaga, https://stackoverflow.com/questions/5117182/number-of-bytes-in-a-binary-character-pointer

    int lena = sizeContent;
    int lenb = sizechunk;


    char * con = (char *) malloc (lena+lenb); // +1 nu avem nevoie

    // copy & concat (including string termination)

    memcpy(con, a, lena);
    memcpy(con+lena, b, lenb);

    printf("\n%d\n", strlen(con)); // pina la primul '\0', https://stackoverflow.com/questions/24596189/does-strlen-return-same-value-for-a-binary-and-ascii-data

    return con;
}


// aici trebuie sa te mai gindesti

char * split(char * string, char * delimiter) {
    string = strstr(string, delimiter);
    return string + strlen(delimiter);
}

void socketConnect() {
    WSADATA WinSockData;
    int WsaStartup;
    int WsaCleanup;

    SOCKET ClientSocket;
    int CloseSocket;

    struct sockaddr_in ServerAddr;

    int Connect;

    int Recv;


    // char RecvBuffer[512];    
    // int RecvBufferLen = strlen(RecvBuffer)

    // strlen(RecvBuffer) nu iti intoarce 512(incearca si o sa intelegi de ce), cel mai bine utilizeaza varianta de mai jos sau:
    // int RecvBufferLen = sizeof(RecvBuffer)

    int BufferLength = 512;  // citi octeti dorim sa citim cel mult din socket la fiecare recv, aici poate fi si 1024, 2048
    // vezi aici este descris bine despre chunks, python si recv -> https://www.binarytides.com/receive-full-data-with-the-recv-socket-function-in-python/

    char RecvBuffer[BufferLength];



    int Send;
    char* SendBuffer = "GET /slide/large/2.jpg HTTP/1.1\r\n"
                       "Host: me.utm.md\r\n"
                       "\r\n";

    int SendBufferLen = strlen(SendBuffer) + 1;

    FILE* file;
    struct timeval timestamp;

    WsaStartup = WSAStartup(MAKEWORD(2,2), &WinSockData);
    if(WsaStartup != 0) {
        printf("WSAStartup failed\n");
    }
    printf("WSAStartup success\n");

    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(ClientSocket == INVALID_SOCKET) {
        printf("TCP client socket creation failed\n");
    }
    printf("TCP client socket creation succeeded\n");

    struct hostent *host=gethostbyname("me.utm.md");
    char *ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

    printf("\n%s\n", ip);

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(ip);
    ServerAddr.sin_port = htons(80);

    Connect = connect(ClientSocket, (SOCKADDR*) &ServerAddr, sizeof(ServerAddr));
    if(Connect == SOCKET_ERROR) {
        printf("Socket connection failed\n");
    }
    printf("Socket connected successfully\n");

    Send = send(ClientSocket, SendBuffer, SendBufferLen, 0);
    if(Send == SOCKET_ERROR) {
        printf("Data send unsuccessfully\n");
    }
    printf("Bytes Sent: %ld\n", Send);
    printf("Data send successfully\n");

    int count = 0;
    char * content = "";
    int bytesRecvCounter = 0;
/*    while((Recv = recv(ClientSocket, RecvBuffer, RecvBufferLen, 0)) > 0) {
    	printf("%s", RecvBuffer);
//        if(count == 0)
//            memcpy(content,RecvBuffer,RecvBufferLen - 1);
//        else
//            memcpy(content+(RecvBufferLen - 1),RecvBuffer,strlen(content) +1);
        content = concat(content, RecvBuffer);
        if(Recv == SOCKET_ERROR) {
            printf("Socket data receiving failed\n");
        }
//        printf(content);
//        printf("\n");s
//        printf(RecvBuffer);
//        printf("\n");
        count++;
    }*/

    do {
        Recv = recv(ClientSocket, RecvBuffer, BufferLength, 0);
        if ( Recv > 0 ) {
            
            printf("Bytes received: %d\n", Recv);

            content = concat(content, bytesRecvCounter, RecvBuffer, Recv); // aici transmitem marimea numaidecit
            
            //prin strlen daca se intilneste NULL byte(ce tine de binary data probabilitatea e destul de mare) atunci va fi gresita marimea
           
            bytesRecvCounter += Recv;

        }
        else if ( Recv == 0 ) {
            printf("Connection closed\n");
        }else {
            printf("recv failed: %d\n", WSAGetLastError());
        }
    } while( Recv > 0 );



    content = strstr(content, "\r\n\r\n"); // trebuie sa te mai gindesti
    char *result = content + 4;

    printf("%s",result);
    printf("\n");
    printf("Data received successfully\n");

    gettimeofday(&timestamp, NULL);
    char time[255];
    sprintf(time, "%lu", timestamp.tv_sec * 1000000 + timestamp.tv_usec);

    file = fopen("D:\\Cora\\univer\\SemVII\\PR\\image1.jpg", "wb");

    if (file == NULL) {
        perror("\n\n\n\n\nFailed to open file\n\n\n\n\n");
        exit(0);
    }

    fwrite(content, 1, bytesRecvCounter, file);

    fflush(file);

    fclose(file);



    CloseSocket = closesocket(ClientSocket);
    if(ClientSocket == SOCKET_ERROR) {
        printf("Couldn't close socket\n");
    }
    printf("Socket closed successfully\n");

    WsaCleanup = WSACleanup();

    if(WsaCleanup == SOCKET_ERROR) {
        printf("Cleanup failed\n");
    }
    printf("Successful cleanup\n");

    printf("\n\nTotal bytes received from socket: %d\n\n", bytesRecvCounter);
    free(content);
}


int main() {

	socketConnect();

	return 0;

}


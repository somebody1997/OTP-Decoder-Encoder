#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1000000

void encrypt(char* message, char* key){
    int length = strlen(message);
    for(int i=0; i < length; i++){
        int messageValue, keyValue, encryptedVal;

        if (message[i] == '\n'){
          continue;
        }

        if (message[i] == ' ') {  messageValue = 26;}
        else { messageValue = message[i] - 'A'; }
        //printf("\n %d",messageValue);
        if (key[i] == ' ') {  keyValue = 26;}
        else { keyValue = key[i] - 'A'; }
       // printf("\n %d",keyValue);
        encryptedVal = (messageValue + keyValue) % 27; // negtive
        //printf("\n %d", encryptedVal);
        if(encryptedVal == 26) message[i] = ' ';
        else message[i] = encryptedVal + 'A';
    }
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);   
  // Alloe client to connect to any address
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]){
    int connectionfd, s, charsRead, charsWriten, size;
    struct addrinfo hints, *result; //, *rp;
    struct sockaddr_storage their_addr;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addr_size; 

    if(argc < 2) {
        fprintf(stderr, "USAGE %s \n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket TCP socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0) {
        perror("ERROR open socket");
    }

    setupAddressStruct(&serverAddress, atoi(argv[1]));

    if (bind(listenSocket, (struct sockaddr *)&serverAddress,  sizeof(serverAddress)) < 0){
        perror("ERROR on binding");
    }
     
    // Listen for incoming connections. set listen queue
    listen(listenSocket, 5);

    while(1){
        
        addr_size = sizeof their_addr;
        connectionfd = accept(listenSocket, (struct sockaddr *)&clientAddress, &addr_size);

        if(connectionfd < 0){
            perror("Accept error");
        }

        // fork a child process
        pid_t child_pid = fork();
        if(child_pid < 0) {
            perror("fork error");
            exit(1);
            break;
        } else if( child_pid == 0) {
            char buffer[BUF_SIZE], message[BUF_SIZE], key[BUF_SIZE];

            //check connection fron enc_client
            charsRead = recv(connectionfd, buffer, sizeof(buffer) - 1, 0); 
            if (charsRead < 0){
                perror("CLIENT: ERROR message reading from socket");
            }
            
            memset(buffer, '\0', sizeof(buffer));

            size = recv(connectionfd, buffer, sizeof(buffer)-1, 0);
            charsRead = 0;
            while(charsRead < size){
                //recive mesage
                charsRead = recv(connectionfd, buffer, sizeof(buffer) - 1, 0);
            }
            strcat(message, buffer);
            
            charsRead = 0;
            memset(buffer, '\0', sizeof(buffer));
            
            while(charsRead < size){
                //recive mesage
                charsRead = recv(connectionfd, buffer, sizeof(buffer) - 1, 0); 
            }
            strcat(key, buffer);
            encrypt(message, key);
            
            charsWriten = 0;
            
            while(charsWriten < size){
                charsWriten = send(connectionfd, message, strlen(message) , 0);
            }
            memset(message, '\0', sizeof(message));
            close(connectionfd); 
            exit(0);
        } else if(child_pid > 0){
            close(connectionfd); 
        } else{
            perror("fork error");
        }
    }

    close(listenSocket);
    return 0;
}

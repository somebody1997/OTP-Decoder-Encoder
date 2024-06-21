#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 100000 

long int getFileSize(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
    }
    fseek(file, 0, SEEK_END); // Move to the end of the file
    long int size = ftell(file); // Get the position at the end (the size of the file)
    fclose(file); // Close the file
    return size; // Return the size of the file
}

int errorcheck(const char* filename){
    FILE* file = fopen(filename, "r");
    int chars = fgetc(file);
    char invalid_chars[] = "!#$*%?@";

    while(chars != EOF) {
        if (strchr(invalid_chars, chars) != NULL) {
            fclose(file);
            return 1; // Invalid character found
        }

        if(!isspace(chars) && !isupper(chars)){
            fclose(file);
            return 1;
        }
        chars = fgetc(file);
    }
    fclose(file);
    return 0; // no invalid character
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char* argv[]){
    int sockfd, charsRead, charsWriten, bytesread;
    struct addrinfo hints;
    struct sockaddr_in serverAddress;
    char buffer[BUF_SIZE], message[BUF_SIZE], key[BUF_SIZE], message_output[BUF_SIZE];

    // Check usage & args
    if (argc < 3) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
        exit(0);
    } 

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    //create  socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
            perror("Client: error opening socket");
    }

    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    perror("CLIENT: ERROR connecting");
  }    

    memset(buffer, '\0', sizeof(buffer));
    // send message to server
    char* check_message = "enc_client";
    send(sockfd, check_message, strlen(check_message), 0);

    memset(buffer, '\0', sizeof(buffer));
  
    // get file length
   long messagesize = getFileSize(argv[1]);
   long keysize = getFileSize(argv[2]);

   if(keysize < messagesize){
       fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
       exit(1);
   }
   
    memset(buffer, '\0', sizeof(buffer));
    if(errorcheck(argv[1])){
        fprintf(stderr,"error message contains bad characters");
        exit(1);
    }

    if(errorcheck(argv[2])){
        perror("error key contains bad characters");
        exit(1);
    }

    FILE *messageFile = fopen(argv[1], "r");
    FILE *keyFile = fopen(argv[2], "r");

    //message = malloc(BUF_SIZE);
    fgets(key, sizeof(key), keyFile);
    fgets(message, sizeof(message), messageFile);
    fclose(keyFile);
    fclose(messageFile);
    
    int size = strlen(message);
    sprintf(buffer, "%d", size);
    send(sockfd, buffer,strlen(message), 0);

    charsWriten = 0;
    while( charsWriten < size){
        charsWriten = send(sockfd, message, strlen(message), 0); //message
    }

    memset(buffer, '\0', sizeof(buffer));
    charsWriten = 0;
    while(charsWriten < size){
        charsWriten = send(sockfd, key, strlen(message),0); //key
    }
    
    // clear 
    // return final message
    memset(buffer, '\0', sizeof(buffer));
    memset(message_output, '\0', sizeof(message_output));
    charsRead = 0;
    while(charsRead < strlen(message)){
        charsRead = recv(sockfd, buffer, sizeof(message) -1, 0);
        if(charsRead == 0){
			break;
		}
      strcat(message_output, buffer);
    }
    
    printf("%s", message_output);
    memset(buffer, '\0', sizeof(buffer));

    close(sockfd);
    return 0;
}

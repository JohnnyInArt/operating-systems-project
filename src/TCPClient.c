#include "TCPClient.h"

void sendMessage(int clientFd, const char *message) {
    if (send(clientFd, message, strlen(message), 0) < 0) {
        perror("Send failed");
    }
}

// Function to safely read user input
int getInput(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) == NULL) {
        perror("Input error");
        return -1;  // Error code
    }
    return 0;  // Success code
}

// Function to read and display the menu from the server
void readMenu(int clientFd) {
    ssize_t valread;              // Variable to hold the number of bytes read from the server
    char buffer[BUFFER_SIZE];    // Buffer to store the data received from the server

    memset(buffer, 0, sizeof(buffer)); // Clear the buffer to ensure it's empty before use

    // Read the menu sent by the server
    while ((valread = read(clientFd, buffer, sizeof(buffer) - 1)) > 0) {
        // Check if the server is signaling to exit
        if (strcmp(buffer, "exit") == 0) {
            // Print exit message and terminate the program if server sends "exit"
            printf("\n------------------------------------------ \n"
                   "              >>> Exiting. <<< "
                   "\n------------------------------------------ \n");
            exit(0);
        }

        buffer[valread] = '\0'; // Null-terminate the buffer to make it a proper string
        printf("%s", buffer); // Display the content of the buffer (the menu) to the user

        // Check if the buffer contains "Hit ENTER" or "Enter" indicating the end of the menu
        if (strstr(buffer, "Hit ENTER") != NULL
            || strstr(buffer, "Enter") != NULL
            || (buffer[valread - 1] == '\n' && strlen(buffer) == 1)) {
            // Break the loop if the end of the menu or prompt is found
            break;
        }

        // Clear the buffer before the next read operation
        memset(buffer, 0, sizeof(buffer));
    }

}


int main(int argc, char *argv[]) {
    int clientFd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char message[BUFFER_SIZE];


    // Create socket
    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Resolve server address
    server = gethostbyname(SERVERADDRESS);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }


    // Set server address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(SERVERPORT);

    // Connect to server
    if (connect(clientFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    // Keep reading and sending data until exit option is chosen
    while (1) {
        // Read and display the menu from the server
        readMenu(clientFd);

        // Get user choice
        if(getInput(message, sizeof(message)) != 0) {
            break; // Exit if there's an error reading input
        }
        sendMessage(clientFd, message);

    }

    // Close the socket
    close(clientFd);
    return 0;
}

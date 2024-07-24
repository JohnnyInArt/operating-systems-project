#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
// Define constants
#define BUFFER_SIZE 1024
#define MAX_LENGTH 100
#define PHONE_LENGTH 20
#define PASSWORD_LENGTH 256
#define SLEEPTIMER 1
#define SERVERPORT 50010



// Structure for a single menu option
typedef struct {
    char description[BUFFER_SIZE];
    void (*action)(int clientFd);
} MenuOption;

// Structure for the menu, containing multiple options
typedef struct {
    MenuOption *options;
    int optionCount;
} Menu;

// Structure for a contact
typedef struct {
    char firstName[MAX_LENGTH];
    char lastName[MAX_LENGTH];
    char phoneNumber[PHONE_LENGTH];
    char email[MAX_LENGTH];
} Contact;

// Function prototypes
void sendMessage(int clientFd, const char *message);
void receiveMessage(int clientFd, char *buffer, size_t sizeBuffer);
void addContact(Contact *contact);
void isValidName(int clientFd, const char *name, bool *runningValid);
void isValidPhoneNumber(int clientFd, const char *phoneNumber, bool *runningValid);
void isValidEmail(int clientFd, const char *email, bool *runningValid);
void phoneNumberExists(int clientFd, const char *phoneNumber, bool *existsFlag);
void handleClientInput(int clientFd, const char *requestMessage, char *detailBuffer, size_t bufferSize, void (*validate)(int, const char *, bool *));
void newContactAction(int clientFd);
void updateContactAction(int clientFd);
void displayContactAction(int clientFd);
void searchBy(int clientFd, const char *serchString);
void searchByNameAction(int clientFd);
void searchByNumberAction(int clientFd);
void searchByEmailAction(int clientFd);
void deleteContactAction(int clientFd);
void deleteAllContactAction(int clientFd);
void exitAction(int clientFd);
void sendMenuOptions(int clientFd, Menu *menu);
void handleClientChoice(int clientFd, Menu *menu, bool *running);
void phonebookManager(int clientFd);
void loginAction(int clientFd);
void guestAction(int clientFd);
void runMenu(int clientFd, Menu *menu);
struct sockaddr_in* init_sockaddr_in(uint16_t port_number);
void handle_sigchld(int sig);

#endif //TCPCLIENT_H

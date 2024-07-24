#include "TCPServer.h"

void sendMessage(int clientFd, const char *message) {
    if (send(clientFd, message, strlen(message), 0) < 0) {
        perror("Send failed");
    }
}

void receiveMessage(int clientFd, char *buffer, size_t sizeBuffer){
    char tempBuffer[BUFFER_SIZE];

    ssize_t valread = read(clientFd, tempBuffer, BUFFER_SIZE - 1);

    if(valread > 0) {
        tempBuffer[strcspn(tempBuffer, "\n")] = '\0';
        // Truncate data if it exceeds server buffer size
        if (valread > sizeBuffer - 1) {
            memcpy(buffer, tempBuffer, sizeBuffer - 1);
            buffer[strcspn(buffer, "\n")] = '\0';  // Ensure null-termination
        } else {
            memcpy(buffer, tempBuffer, valread);
        }
    } else {
        perror("Read failed");
        buffer[0] = '\0';
    }
}


// Function to add a contact to the file
void addContact(Contact *contact) {
    FILE *file = fopen("ContactList.txt", "a");
    if (file == NULL) {
        perror("Unable to open file");
        return;
    }

    // Move the file position indicator to the last character of the file.
    fseek(file, -1, SEEK_END);
    int lastChar = fgetc(file); // Read the last character

    // If the last character is not a newline, it means the file does not end with a newline.
    if (lastChar != '\n' && lastChar != EOF) {
        fprintf(file, "\n");
    }

    fprintf(file, "%s %s | %s | %s \n",
                contact->firstName, contact->lastName, contact->phoneNumber, contact->email);

    fclose(file);
}

void isValidName(int clientFd, const char *name, bool *runningValid) {
    *runningValid = false;
    // Check if name length is valid
    if (strlen(name) == 0 || strlen(name) >= MAX_LENGTH - 1) {
        const char *longError = "\n------------------------------------------\n"
                                "       Error: Name incorrect size. \n"
                                "------------------------------------------ \n";
        sendMessage(clientFd, longError);
        *runningValid = true;
        return;
    }
    // Check if name contains only contain alphabetic characters and spaces
    for (size_t i = 0; i < strlen(name); i++) {
        if (!isalpha(name[i]) && name[i] != ' ') {
            const char *alphaError = "\n---------------------------------------------------------------\n"
                                    " Error: Name can only contain alphabetic characters and spaces. \n"
                                    "---------------------------------------------------------------\n";
            sendMessage(clientFd, alphaError);
            *runningValid = true;
            return;
        }
    }
}

void isValidPhoneNumber(int clientFd, const char *phoneNumber, bool *runningValid) {
	// Define minimum and maximum phone number lengths
    const size_t MIN_PHONE_LENGTH = 7;
    const size_t MAX_PHONE_LENGTH = 15;
	size_t lengthNumber = strlen(phoneNumber);
	*runningValid = false;

    // Check if phone number length is valid
    if (lengthNumber < MIN_PHONE_LENGTH || lengthNumber > MAX_PHONE_LENGTH  ) {
        char longError[BUFFER_SIZE];
		snprintf(longError, BUFFER_SIZE,
                 "\n-------------------------------------------------------------------------------\n"
                 "   Error: Phone number incorrect size (got %zu, expected between %zu and %zu). \n"
                 "-------------------------------------------------------------------------------\n",
                 lengthNumber, MIN_PHONE_LENGTH, MAX_PHONE_LENGTH);
        sendMessage(clientFd, longError);

        *runningValid = true;
		return;
    }
    // Check if phone number contains only digits
    for (size_t i = 0; i < strlen(phoneNumber); i++) {
        if (!isdigit(phoneNumber[i])) {
            const char *digitError = "\n------------------------------------------\n"
                                    "  Error: Must contain only numeric digits. \n"
                                    "------------------------------------------ \n";
            sendMessage(clientFd, digitError);
            *runningValid = true;
			return;
        }
    }
}

void isValidEmail(int clientFd, const char *email, bool *runningValid) {
    *runningValid = false;
    // Check if email length is valid
    if (strlen(email) > MAX_LENGTH - 1) {
        const char *longError = "\n------------------------------------------\n"
                                "       Error: Email too long. \n"
                                "------------------------------------------ \n";
        sendMessage(clientFd, longError);
        *runningValid = true;
		return;
    }
    // Check if phone number contains contain '@' and '.'
    const char *at = strchr(email, '@');
    const char *dot = strrchr(email, '.');
    if (!at || !dot) {
        const char *formatError = "\n------------------------------------------\n"
                                "       Error: Invalid email format. \n"
                                "------------------------------------------ \n";
            sendMessage(clientFd, formatError);
        *runningValid = true;
		return;
    }
}

// Function to check if a phone number already exists
void phoneNumberExists(int clientFd, const char *phoneNumber, bool *existsFlag) {
    FILE *file = fopen("ContactList.txt", "r");
    if (file == NULL) {
        perror("Unable to open file");
        *existsFlag = false; // Assume the number doesn't exist if the file can't be opened
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, phoneNumber)) {
			const char *duplicateError = "\n-----------------------------------------------------------\n"
                                 		 "   Error: Contact with this phone number already exists.\n"
                                 		 "-----------------------------------------------------------\n";
       		sendMessage(clientFd, duplicateError);

            fclose(file);
            *existsFlag =  true;
			return;
        }
    }

    fclose(file);
    *existsFlag =  false;
}

void handleClientInput(int clientFd,const char *requestMessage, char *detailBuffer, size_t bufferSize, void (*validate)(int, const char *, bool *)) {
    bool runningValid = false;

   do {
        sendMessage(clientFd, requestMessage);
        receiveMessage(clientFd, detailBuffer, bufferSize);

        runningValid = false;
        validate(clientFd, detailBuffer, &runningValid);
   }
   while(runningValid);
}

// Function implementations for actions (placeholders for the actual logic)
void newContactAction(int clientFd) {
    Contact contact;
 	bool existsFlag = false;


    const char *addPage = "\n------------------------------------------\n"
                          "        >>> Add New Contact <<< \n"
                          "------------------------------------------ ";
    sendMessage(clientFd, addPage);


    // Request contact first name
    handleClientInput(clientFd, "\nEnter first name: ", contact.firstName, MAX_LENGTH, isValidName);
	// Request contact last name
    handleClientInput(clientFd, "\nEnter last name: ", contact.lastName, MAX_LENGTH, isValidName);
    // Check if the phone number already exists
    do {
		// Request contact phone number
     	handleClientInput(clientFd, "\nEnter phone number: ", contact.phoneNumber, PHONE_LENGTH, isValidPhoneNumber);
		phoneNumberExists(clientFd, contact.phoneNumber, &existsFlag);
	} while(existsFlag);

	// Request contact email
    handleClientInput(clientFd, "\nEnter email: ", contact.email, MAX_LENGTH, isValidEmail);

    addContact(&contact);

    // Send confirmation to client
    char *confirmation = "\n------------------------------------------ \n"
                         "   >>> Success: Contact detailes added. <<< "
                         "\n------------------------------------------ \n";
    sendMessage(clientFd, confirmation);

    sleep(SLEEPTIMER);

}

void updateContactAction(int clientFd) {
    Contact contactUpdate;
    bool contactFound = false;   // Flag to track if the contact was found and updated
	bool existsFlag = false;	// Flag to track if the new contact phone number already exists
    char line[BUFFER_SIZE];     // Buffer to read each line from the original file
    char tempBuffer[BUFFER_SIZE];
    char numberSearched[PHONE_LENGTH];
    char confirmation[BUFFER_SIZE];


    
    // Prompt for the phone number of the contact to update
    const char *prompt = "\n------------------------------------------ \n"
                         "        >>> Update Contact <<< \n"
                         "------------------------------------------ \n"
                         "Enter phone number of the contact to update: ";
    sendMessage(clientFd, prompt);
    
    receiveMessage(clientFd, contactUpdate.phoneNumber, PHONE_LENGTH);
    
    // Open the original file and a temporary file
    FILE *originalContactList = fopen("ContactList.txt", "r");
    FILE *tempContactList = fopen("tempContactList.txt", "w");
    
    if (originalContactList == NULL || tempContactList == NULL) {
        perror("Unable to open file");
        if (originalContactList) {
            fclose(originalContactList);
        }
        if (tempContactList) {
            fclose(tempContactList);
        }
        return;
    }
    
    // Read through the original file
    while (fgets(line, sizeof(line), originalContactList)) {
        // Check if the current line contains the phone number
        if (strstr(line, contactUpdate.phoneNumber)) {
            strncpy(numberSearched, contactUpdate.phoneNumber, PHONE_LENGTH);
            contactFound = true;
            
            // Display current contact details
            const char *contactFound = "\n------------------------------------------ \n"
                                       "       Current contact details:\n"
                                       "------------------------------------------ \n";
            sendMessage(clientFd, contactFound);
            sendMessage(clientFd, line);
            
            // Request contact first name
            handleClientInput(clientFd, "\nEnter first name: ", contactUpdate.firstName, MAX_LENGTH, isValidName);
            // Request contact last name
            handleClientInput(clientFd, "\nEnter last name: ", contactUpdate.lastName, MAX_LENGTH, isValidName);
             // Check if the phone number already exists
    		do {
       			// Request contact phone number
     			handleClientInput(clientFd, "\nEnter phone number: ", contactUpdate.phoneNumber, PHONE_LENGTH, isValidPhoneNumber);

                //if the number is the same, do not check if it already exists in the contact list
                if(strstr(numberSearched, contactUpdate.phoneNumber)){
                    existsFlag = false;
                } else {
                    phoneNumberExists(clientFd, contactUpdate.phoneNumber, &existsFlag);
                }

			} while(existsFlag);
			// Request contact email
            handleClientInput(clientFd, "\nEnter new email: ", contactUpdate.email, MAX_LENGTH, isValidEmail);
            
            // Display updated details for confirmation
            snprintf(tempBuffer, sizeof(tempBuffer), "\nUpdated details: Name: %s %s, Phone: %s, Email: %s\n",
                     contactUpdate.firstName, contactUpdate.lastName, contactUpdate.phoneNumber, contactUpdate.email);
            sendMessage(clientFd, tempBuffer);

             // Ask for confirmation
            const char *confirmMsg = "\n------------------------------------------ \n"
                                     "      >>> Confirm changes (Y/N) <<< \n"
                                     "      Enter your choice & Hit ENTER: \n"
                                     "------------------------------------------ \n";
            sendMessage(clientFd, confirmMsg);

            receiveMessage(clientFd, confirmation, sizeof(confirmation));

            // Proceed based on user's confirmation
            if (toupper(confirmation[0]) == 'Y') {
                // Write updated contact details to the temporary file
                fprintf(tempContactList, "%s %s | %s | %s \n",
                        contactUpdate.firstName, contactUpdate.lastName, contactUpdate.phoneNumber, contactUpdate.email);

                const char *successMsg = "\n----------------------------------------------- \n"
                                         " >>> Update SUCCESSFUL. Contact updated. <<< \n"
                                         "----------------------------------------------- \n";
                sendMessage(clientFd, successMsg);
            } else {
                // Write the unchanged line to the temporary file
                fputs(line, tempContactList);
                // Notify the client that the update was canceled
                const char *cancelMsg = "\n----------------------------------------------- \n"
                                        " >>> Update CANCELED. Contact not updated. <<< \n"
                                        "----------------------------------------------- \n";
                sendMessage(clientFd, cancelMsg);
            }
        } else {
            // Write the line unchanged to the temporary file
            fputs(line, tempContactList);
        }
    }


    if (!contactFound) {
        // If no contact was found, send an error message to the client
        const char *errorMsg = "\n----------------------------------------------- \n"
                               " ERROR: Contact with this phone number not found. \n"
                               "----------------------------------------------- \n";
        sendMessage(clientFd, errorMsg);
    }
    
    // Close the files
    fclose(originalContactList);
    fclose(tempContactList);
    
    // Replace the original file with the temporary file
    if (remove("ContactList.txt") != 0) {
        perror("Unable to delete original file");
        return;
    }
    
    if (rename("tempContactList.txt", "ContactList.txt") != 0) {
        perror("Unable to rename temporary file");
    }

    sleep(SLEEPTIMER);

}


void displayContactAction(int clientFd) {
    char line[BUFFER_SIZE];

    const char *displayPage = "\n------------------------------------------ \n"
                              "        >>> Display Contact <<< \n"
                              "------------------------------------------ \n";
    sendMessage(clientFd, displayPage);

    FILE *file = fopen("ContactList.txt", "r");
    if (file == NULL) {
        perror("Unable to open file");
        sendMessage(clientFd, "Unable to open contact list file.\n");
        return;
    }


    while (fgets(line, sizeof(line), file)) {
        sendMessage(clientFd, line);
    }

    fclose(file);

    sleep(SLEEPTIMER);
}

// Function to search for contacts
void searchBy(int clientFd, const char *serchString) {
    char line[BUFFER_SIZE];
    bool found = false;

    // Open the contact list file
    FILE *file = fopen("ContactList.txt", "r");
    if (file == NULL) {
        perror("Unable to open file");
        sendMessage(clientFd, "Unable to open contact list file.\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, serchString)) {
            // Message indicating contacts were found
            const char *stringFound = "\n------------------------------------------ \n"
                                      "           Contacts found. \n"
                                      "------------------------------------------ \n";
            sendMessage(clientFd, stringFound);

            sendMessage(clientFd, line);
            found = true;
        }
    }

    // Message indicating no contacts were found
    if (!found) {
        const char *stringNotFound = "\n------------------------------------------ \n"
                                  "       No contacts found with that name. \n"
                                  "------------------------------------------ \n";
        sendMessage(clientFd, stringNotFound);
    }

    // Close the file
    fclose(file);

    // Sleep for a while before returning
    sleep(SLEEPTIMER);
}

void searchByNameAction(int clientFd) {
    char name[MAX_LENGTH];

    const char *searchPage = "\n------------------------------------------ \n"
                             "        >>> Search Contact by Name <<< \n"
                             "------------------------------------------ \n";
    sendMessage(clientFd, searchPage);

    // Handle client input
    handleClientInput(clientFd, "\nEnter the name to search: ", name, MAX_LENGTH, isValidName);

    // Perform the search
    searchBy(clientFd, name);

}

void searchByNumberAction(int clientFd) {
    char phoneNumber[PHONE_LENGTH];

    const char *searchPage = "\n------------------------------------------ \n"
                             "        >>> Search Contact by Number <<< \n"
                             "------------------------------------------ \n";
    sendMessage(clientFd, searchPage);

    // Handle client input
    handleClientInput(clientFd, "\nEnter the phone number to search: ", phoneNumber, PHONE_LENGTH, isValidPhoneNumber);

    // Perform the search
    searchBy(clientFd, phoneNumber);

}

void searchByEmailAction(int clientFd) {
    char email[MAX_LENGTH];

    const char *searchPage = "\n------------------------------------------ \n"
                             "        >>> Search Contact by Email <<< \n"
                             "------------------------------------------ \n";
    sendMessage(clientFd, searchPage);

    // Handle client input
    handleClientInput(clientFd, "\nEnter the email to search: ", email, MAX_LENGTH, isValidEmail);

    // Perform the search
    searchBy(clientFd, email);

}


void deleteContactAction(int clientFd) {
    char phoneNumber[PHONE_LENGTH];
    char line[BUFFER_SIZE];

    const char *deletePage = "\n------------------------------------------ \n"
                             "        >>> Delete Contact <<< \n"
                             "------------------------------------------ \n";
    sendMessage(clientFd, deletePage);

    handleClientInput(clientFd, "\nEnter the phone number of the contact to delete: ", phoneNumber, PHONE_LENGTH, isValidPhoneNumber);

    FILE *file = fopen("ContactList.txt", "r");
    FILE *tempFile = fopen("temp.txt", "w");

    if (file == NULL || tempFile == NULL) {
        perror("Unable to open file");
        sendMessage(clientFd, "Unable to open contact list file.\n");
        if (file != NULL) {
            fclose(file);
        }
        if (tempFile != NULL) {
            fclose(tempFile);
        }
        return;
    }


    bool found = false;
    while (fgets(line, sizeof(line), file)) {
        if (!strstr(line, phoneNumber)) {
            fputs(line, tempFile);
        } else {
            found = true;
        }
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove("ContactList.txt");
        rename("temp.txt", "ContactList.txt");
        const char *deleteSuccess = "\n------------------------------------------ \n"
                                    "    Contact deleted successfully.\n"
                                    "------------------------------------------ \n";
        sendMessage(clientFd, deleteSuccess);
    } else {
        remove("temp.txt");
        const char *contactNotFound = "\n------------------------------------------ \n"
                                      "    No contact found with that phone number.\n"
                                      "------------------------------------------ \n";
        sendMessage(clientFd, contactNotFound);
    }

    sleep(SLEEPTIMER);

}

void deleteAllContactAction(int clientFd) {
    const char *deleteAllPage = "\n------------------------------------------ \n"
                                   "        >>> Delete All Contacts <<< \n"
                                   "------------------------------------------ \n";
    sendMessage(clientFd, deleteAllPage);

    // Attempt to truncate the file to delete all contents
    FILE *file = fopen("ContactList.txt", "w"); // Open the file in write mode, which truncates it
    if (file == NULL) {
        perror("Unable to open file");
        const char *errorMsg = "\n------------------------------------------ \n"
                               "   Error: Unable to delete contacts. \n"
                               "------------------------------------------ \n";
        sendMessage(clientFd, errorMsg);
        return;
    }

    fclose(file);

    // Notify the client that all contacts have been deleted
    const char *successMsg = "\n------------------------------------------ \n"
                             "   >>> Success: All contacts deleted. <<< \n"
                             "------------------------------------------ \n";
    sendMessage(clientFd, successMsg);

    sleep(SLEEPTIMER);

}


void exitAction(int clientFd) {
    char *exitMessage = "exit";
    sendMessage(clientFd, exitMessage);
    exit(0);
}

// Function to send menu options to the client
void sendMenuOptions(int clientFd, Menu *menu) {
    char listAction[4096] = {0};

    char *intro="\n         >>> Choose an option: <<< "
                "\n------------------------------------------ \n";
        sendMessage(clientFd, intro);

    for (int i = 0; i < menu->optionCount; i++) {
        snprintf(listAction, sizeof(listAction), "%d) %s\n", i + 1, menu->options[i].description);
        sendMessage(clientFd, listAction);
    }
}

//Fucntion to handle the client's choice
void handleClientChoice(int clientFd, Menu *menu, bool *running) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread;
    int choice;

    char *choise="\n------------------------------------------ \n"
                 "  >>> Enter your choice & Hit ENTER: <<< "
                 "\n------------------------------------------ \n";
    sendMessage(clientFd, choise);

    valread = read(clientFd, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0'; // Null-terminate the input
        choice = atoi(buffer) - 1; // Convert choice to zero-based index
        if (choice >= 0 && choice < menu->optionCount) {
            menu->options[choice].action(clientFd);
            if (choice == menu->optionCount - 1) {
                *running = false;
            }
        } else {
            char *invalid = "\n------------------------------------------ \n"
                            "      >>> Invalid choice. <<< "
                            "\n------------------------------------------ \n";
            sendMessage(clientFd, invalid);
            sleep(SLEEPTIMER);
        }
    } else {
        perror("Failed to read from client");
        *running = false;
    }
}


void phonebookManager(int clientFd) {
    bool running = true;

    MenuOption panelOptions[] = {
        {"Add New Contact.", newContactAction},
        {"Update Contact.", updateContactAction},
        {"Display All Conctact.", displayContactAction},
        {"Search By Name.", searchByNameAction},
        {"Search By Number.", searchByNumberAction },
        {"Search By Email.", searchByEmailAction},
        {"Delete Contact.", deleteContactAction },
        {"Delete All Contact.", deleteAllContactAction },
        {"EXIT.", exitAction}
    };

    int panelCount = sizeof(panelOptions) / sizeof(panelOptions[0]);
    Menu panelMenu = {panelOptions, panelCount};

    while(running) {

        char *intro="\n------------------------------------------ \n"
                    "              Control Pannel        \n";
        sendMessage(clientFd, intro);

        // Send menu options to client
        sendMenuOptions(clientFd, &panelMenu);

        // Handle the client's choice
        handleClientChoice(clientFd, &panelMenu, &running);
    }
}


void loginAction(int clientFd) {
    const char *loginPassword = "SO2324SO"; // Define the admin password
    char inputPassword[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0};

    const char *prompt = "\n------------------------------------------ \n"
                         "        >>> Login Page   <<< \n"
                         "------------------------------------------- \n"
                         "Enter your password & Hit ENTER: ";
    sendMessage(clientFd, prompt);

    receiveMessage(clientFd, buffer, sizeof(buffer));
    // Remove potential newline character
    strncpy(inputPassword, buffer, BUFFER_SIZE);

    //Verify the password
    if(strcmp(loginPassword, inputPassword) == 0) {
        const char *successMsg = "\n------------------------------------------ \n"
                                 "              ACCESS GARANTED \n"
                                 "     Welcome to Phonebook Manager Panel.  \n"
                                 "------------------------------------------ \n";
        sendMessage(clientFd, successMsg);
        phonebookManager(clientFd);
    } else {
        const char *errorMsg = "\n------------------------------------------ \n"
                               " ERROR: Invalid password, please try again.  \n"
                               "------------------------------------------ \n";
        sendMessage(clientFd, errorMsg);
    }
    sleep(SLEEPTIMER);
}


void guestAction(int clientFd) {
    bool running = true;

    MenuOption panelOptions[] = {
        {"Display All Conctact.", displayContactAction},
        {"Search By Name.", searchByNameAction},
        {"Search By Number.", searchByNumberAction },
        {"Search By Email.", searchByEmailAction},
        {"EXIT.", exitAction}
    };

    int panelCount = sizeof(panelOptions) / sizeof(panelOptions[0]);
    Menu panelMenu = {panelOptions, panelCount};

    while(running) {

        char *intro="\n------------------------------------------ \n"
                    "              >>> Guest Page <<<       \n"
                    "     Welcome to Phonebook Manager Panel.  \n";
        sendMessage(clientFd, intro);

        // Send menu options to client
        sendMenuOptions(clientFd, &panelMenu);

        // Handle the client's choice
        handleClientChoice(clientFd, &panelMenu, &running);
    }
}



// Function to execute the menu
void runMenu(int clientFd, Menu *menu) {
    bool running = true;

    while(running){

        char *intro="\n------------------------------------------ \n"
                    "       Phonebook Management System  \n";
        sendMessage(clientFd, intro);

        // Send menu options to client
        sendMenuOptions(clientFd, menu);

        // Handle the client's choice
        handleClientChoice(clientFd, menu, &running);

        sleep(SLEEPTIMER);
    }
}


// Function to initialize socket address structure
struct sockaddr_in* init_sockaddr_in(uint16_t port_number) {
    struct sockaddr_in *socket_address = malloc(sizeof(struct sockaddr_in));
    if(socket_address == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    memset(socket_address, 0, sizeof(*socket_address));
    socket_address->sin_family = AF_INET;
    socket_address->sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address->sin_port = htons(port_number);
    return socket_address;
}

void handleSigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[]) {
    const uint16_t port_number = SERVERPORT;

    signal(SIGCHLD, handleSigchld);

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *server_sockaddr = init_sockaddr_in(port_number);
    struct sockaddr_in client_sockaddr;
    socklen_t client_socklen = sizeof(client_sockaddr);

    MenuOption menuOptions[] = {
        {"Login.", loginAction},
        {"Guest.", guestAction},
        {"EXIT.", exitAction}
    };

    int optionCount = sizeof(menuOptions) / sizeof(menuOptions[0]);
    Menu menu = {menuOptions, optionCount};


    if (bind(serverFd, (const struct sockaddr *) server_sockaddr, sizeof(*server_sockaddr)) < 0) {
        perror("Bind failed");
        free(server_sockaddr);
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 3) < 0) {
        perror("Listen failed");
        free(server_sockaddr);
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port_number);

    while (1) {
        int clientFd = accept(serverFd, (struct sockaddr *) &client_sockaddr, &client_socklen);
        if (clientFd == -1) {
            perror("Accept failed");
            continue;
        }

        __pid_t pid = fork();
        if (pid == 0) {
            close(serverFd);

            printf("Connection with `%d` established. Delegated to process %d.\n", clientFd, getpid());

            runMenu(clientFd, &menu);
            close(clientFd);

            exit(0);
        } else if (pid > 0) {
            close(clientFd);
        } else {
            perror("Fork failed");
        }
    }


    // Clean up
    free(server_sockaddr);
    close(serverFd);

    return 0;
}

Phonebook Management System

To run the program, you need to open a terminal and navigate to the project folder using the cd command. Then, if the executable files TCPClient and TCPServer are not present in the folder, you can create them using the make command. This way, you can run the client-server program using the IP address '127.0.0.1' (the loopback address), creating communication between client and server on the same machine.

If you want to run the program on multiple different machines, you need to modify the SERVERADDRESS variable with the IP address of the machine (ensure that the firewall rules allow traffic on the specified port) where the TCPSERVER process will run, located in the header file TCPClient.h.

Once the mode is decided, run the TCPClient process in two different terminals with the command ./TCPClient and the TCPServer process in another terminal with the command ./TCPServer.

(The password to access the login screen in the program is "SO2324SO")

Christopher Boumalhab, 902138925

Server Side:

The server listens on a specified port and accepts incoming connections from clients.
Upon connection, it receives file contents from the client and writes them to a file named according to the connection ID.
Error handling is implemented for various scenarios, such as timeouts or failed file operations.

Client Side:

The client connects to the server at the specified hostname and port.
It sends a file specified as a command-line argument to the server.
Error handling is included for connection failures and file opening errors.
Communication Protocol:

The communication between the client and server is based on TCP/IP sockets.
File transfer occurs over the established connection.

Additional Considerations:

Signal handling is implemented to gracefully exit the server.
Socket options are set to manage timeouts and ensure reliable communication.
The code includes basic error checking and reporting.
This implementation provides a foundation for a simple FTP-like file transfer system in C. Further enhancements could include support for additional FTP commands, security features like encryption, and improved error handling.


Problems I ran into:

Failing tests because file was too big. A portion of the file would be sent, but not the whole thing.
Not formatting error printing as required which failed some tests.
Dealing with the 10 second timeout problem was difficult.

Additional Libraries:
- netdb.h

Oustide Help:
Youtube
GeeksForGeeks
Systems Programming Class
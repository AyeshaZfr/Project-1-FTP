#ifndef DEFINE
#define DEFINE

// Define ports
#define SERVER_IP_ADDRESS "127.0.0.1"
#define CONTROL_CONNECTION_PORT 21                           
#define DATA_TRANSFER_PORT 20 

// Define Responses
// PICKED FROM https://www.rfc-editor.org/rfc/rfc959

#define FILE_STATUS_OK "150 File status okay; about to open data connection"
#define TRANSFER_COMPLETED "226 Transfer completed."
#define TRANSFER_FAILED "226 Transfer completed."
#define INVALID_COMMAND "202 Command not implemented."

#define LOGIN_FAILED "530 Not logged in."
#define LOGIN_NEED_PASS "331 Username OK, need password."
#define LOGIN_SUCCESS "230 User logged in, proceed."

#define VALID_DIRECTORY "200 directory changed to "
#define INVALID_DIRECTORY "550 No such file or directory."

#define INVALID_SEQUENCE "503 Bad sequence of commands."

#define PORT_SUCCESS  "200 PORT command successful"
#define SERVER_OPEN "220 Service ready for new user."
#define SERVER_CLOSE "221 Service closing control connection."

#define PWD "257 %s."


#endif
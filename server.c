#include <stdio.h>
#include <windows.h>
#include <string.h>

#define PORT 9909
#define MAXCLIENTS 256

WSADATA ws;
SOCKET nServer, clientSockets[MAXCLIENTS];
struct sockaddr_in srv;

typedef struct clients{
char username[32];
int id,state;
}clients;
clients clientArr[1024];
int liveClients = 0;
int partner1ID;

void disconnectClient(int index);
int uValidation1(char name[32], int id);
void showWaiting(int id);
void sendingList1(int id);

int main(void)
{
if(WSAStartup(MAKEWORD(2,2),&ws) != 0)
{
printf("WSA intialization failed.\n");
WSACleanup();
exit(0);
}

// now creating the main server socket
nServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(nServer == INVALID_SOCKET)
{
printf("Error in creating the main socket.\n");
WSACleanup();
exit(0);
}

// setting up the address structure
srv.sin_family = AF_INET;
srv.sin_port = htons(PORT);
srv.sin_addr.s_addr = INADDR_ANY;
memset(&(srv.sin_zero),0,8);

// Bind the server socket
if(bind(nServer,(struct sockaddr*)&srv, sizeof(srv))<0)
{
printf("Error in binding the main socket.\n");
closesocket(nServer);
WSACleanup();
exit(0);
}

// Now we start listening for incoming connections
if(listen(nServer,MAXCLIENTS)<0)
{
printf("Failed to start listening.\n");
closesocket(nServer);
WSACleanup();
exit(0);
}
printf("Server listening on PORT %d \n", PORT);

// Initialize client socket array
for(int i = 0; i<MAXCLIENTS; i++)
{
clientSockets[i] = 0;
}

fd_set readfds;

while(1)
{
// Clearing the socket set
FD_ZERO(&readfds);

// Adding the server socket to set
FD_SET(nServer, &readfds); //first agr is the FD to add, second is the FdSet
SOCKET max_sd = nServer;

//Adding clients to set
for(int i = 0 ; i <MAXCLIENTS; i++)
{
SOCKET sd = clientSockets[i];
if(sd>0) // valid socket check, 0 represents unused slots.
{
FD_SET(sd,&readfds);
// adding client to the set of sockets
}
if(sd>max_sd)
{
max_sd = sd;
// updating the max value of SD as it will be used by Select() to monitor the range of FDs.
}
}

// Wait for activity on one of the sockets
int activity = select(max_sd +1, &readfds, NULL, NULL,NULL);

if(activity<0 && WSAGetLastError() != WSAEINTR)
{
printf("Select Error.\n");
continue;
}

// if activity on the server socket, it's an incoming connection
if (FD_ISSET(nServer, &readfds))
{
        struct sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET newSocket = accept(nServer, (struct sockaddr*)&clientAddr, &addrLen);

        if (newSocket == INVALID_SOCKET)
        {
            printf("Failed to accept connection.\n");
            continue;
        }
            printf("New Client Connected: %d\n", newSocket);

        char name_buffer[32];
        int isValid = 0;  // Flag to track validation status
        int attempts = 0; // Track number of attempts

        while (!isValid)
        {
            int bytesReceived = recv(newSocket, name_buffer, sizeof(name_buffer) - 1, 0);
            if (bytesReceived <= 0)
            {
                printf("Error receiving username or client disconnected.\n");
                closesocket(newSocket);
                break;
            }

        name_buffer[bytesReceived] = '\0';  // Null-terminate the string
        printf("Received username attempt: %s\n", name_buffer);

        // Validate the username
        if (uValidation1(name_buffer, newSocket) == 1)
        {
            // Username is valid
            isValid = 1;
            int flag = htonl(1);  // Success flag
            send(newSocket, (char*)&flag, sizeof(flag), 0);

            // Add the client to the array
            for (int i = 0; i < MAXCLIENTS; i++)
            {
                if (clientSockets[i] == 0)
                {
                    clientSockets[i] = newSocket;
                    strncpy(clientArr[liveClients].username, name_buffer, sizeof(clientArr[liveClients].username) - 1);
                    clientArr[liveClients].id = newSocket;
                    clientArr[liveClients].state = 0;
                    break;
                }
            }
        }
        else
        {
            // Invalid username
            int flag = htonl(0);  // Failure flag
            send(newSocket, (char*)&flag, sizeof(flag), 0);
            attempts++;

            // Optional: Limit attempts
            if (attempts >= 3)
            {
                printf("Too many invalid username attempts. Disconnecting client.\n");
                closesocket(newSocket);
                break;
            }
        }
    }
    int temp_choice;
    int r = recv(newSocket, (char*)&temp_choice, sizeof(temp_choice), 0);
if (r <= 0)
{
    printf("Error %d in receiving the choice or client disconnected.\n", WSAGetLastError());
   
}
temp_choice = ntohl(temp_choice);  // Convert from network byte order
printf("[%s] Chose option: %d\n", name_buffer, temp_choice);
clientArr[liveClients].state = temp_choice;

if(temp_choice == 2) // client wants to join somebody who is waiting(list of people who chose option1)
{
sendingList1(newSocket);

char relayBuff[256];
r = recv(newSocket, (char*)&partner1ID, sizeof(partner1ID),0);
if(r<0)
{
printf("Error %d in receiving the client id to connect with.\n", WSAGetLastError());
}
else if(r==0)
{
printf("Client disconnected\n");
}
partner1ID = ntohl(partner1ID);
printf("recieved client id : %d \n", partner1ID);

int r1, r2;
do {
r1 = recv(newSocket, relayBuff, sizeof(relayBuff),0);
if(r1>0)
send(partner1ID,relayBuff,sizeof(relayBuff),0);
r2 =recv(partner1ID,relayBuff,sizeof(relayBuff),0);
if(r2>0)
send(newSocket,relayBuff,sizeof(relayBuff),0);
if(r1 <= 0 || r2 <= 0)
{
if(r1<=0)
{
send(partner1ID,"Conversation is Closed\n", strlen("Conversation is Closed\n"),0);
}
if(r2<=0)
{
send(newSocket,"Conversation is Closed\n", strlen("Conversation is Closed\n"),0);
}
printf("Connection closed\n");
disconnectClient(liveClients-1);
disconnectClient(liveClients);
//closesocket(newSocket);
//closesocket(partner1ID);
liveClients-=2;
break;
}
}while(1);

}

liveClients++;  // Increment liveClients
//continue;
}


}
closesocket(nServer);
WSACleanup();
return 0;
}

void disconnectClient(int index)
{
SOCKET sd = clientSockets[index];
closesocket(sd);
clientSockets[index] = 0;
printf("Removed client %d from the active list.\n", sd);
strcpy(clientArr[index].username,"");
clientArr[index].state=404;
clientArr[index].id = 0;
showWaiting(sd);
}

int uValidation1(char name[32], int id)
{
printf("Name received : %s \n", name);
for (int i = 0; i < liveClients; i++) {
        if (strcmp(clientArr[i].username, name) == 0) {
            // Username already exists
            printf("Username [%s] is already in use.\n", name);
            return 0;
        }
    }

    int length = strlen(name);

    if (length < 1 || length > 32) {
        printf("Username must be between 1 and 32 characters.\n");
        return 0;
    }

    for (int i = 0; i < length; ++i) {
        if (name[i] == ' ') {
            printf("Username cannot contain spaces.\n");
            return 0;
        }
    }

    return 1; // Username is unique
}

void showWaiting(int id)
{
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
printf("Currently active clients.\n");
for(int i = 0; i<liveClients; i++)
{
if(clientArr[i].username)
{
printf("<Code : %d \t Name : %s \t >\n",clientArr[i].id, clientArr[i].username);
}
}
printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void sendingList1(int id)
{
for(int i = 0; i<liveClients; i++)
{
if(clientArr[i].state == 1)
{
char buffer[100];
sprintf(buffer,"<<ID: %d \t Name: %s", clientArr[i].id, clientArr[i].username);
//printf("%s\n",buffer);
if(send(id, buffer, sizeof(buffer), 0)<0)
{
printf("SomeError in sending the list of clients who chose 1\n");
}
else
{
//sent successfully.
}
}
}
send(id,"END", strlen("END"),0);
}

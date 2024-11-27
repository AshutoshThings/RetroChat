#include <stdio.h>
#include <windows.h>
#include <winsock.h>

#define PORT 9909
#define SERVER_IP "127.0.0.1"
#define MAX_USERNAME_LENGTH 32

struct sockaddr_in crv;
WSADATA ws;
SOCKET nClient;
char username[MAX_USERNAME_LENGTH + 1];

void loading(int n);
void instructions();


int validateUsername(const char* username)
{
    int length = strlen(username);

    if (length < 1 || length > MAX_USERNAME_LENGTH)
    {
        printf("Username must be between 1 and %d characters.\n", MAX_USERNAME_LENGTH);
        return 0;
    }

    for (int i = 0; i < length; ++i)
    {
        if (username[i] == ' ') // Spaces are not allowed
        {
            printf("Username cannot contain spaces.\n");
            return 0;
        }
    }

    return 1; // Valid username
}

int validateChoice(int n)
{
     if( n == 1 || n == 2 || n == 3 || n == 4)
     {
        return 1; // valid choice
     }
     else
     {
        return 0;
     }
}

int main(void)
{
    instructions();
    if(WSAStartup(MAKEWORD(2,2),&ws)!=0)
    {
        printf("Failed to initialize WSA environment\n");
        WSACleanup();
        exit(0);
    }

    nClient = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(nClient == INVALID_SOCKET)
    {
        printf("Failed to create your Client Socket\n");
        WSACleanup();
        exit(0);
    }

    crv.sin_family = AF_INET;
    crv.sin_port = htons(PORT);
    crv.sin_addr.s_addr = inet_addr(SERVER_IP);

    // in client side we don't need to do binding as it is automatically handled by the OS.
    //Now we will attempt to connect to the server
    if(connect(nClient,(struct sockaddr*)&crv, sizeof(crv))<0)
    {
        printf("Failed to connect to the Central Server\n");
        closesocket(nClient);
        WSACleanup();
        exit(0);
    }

    int result;

    do
    {
        printf("Enter your username (1-%d characters, no spaces): ", MAX_USERNAME_LENGTH);
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = '\0'; // Remove trailing newline
        fflush(stdin); 
        //printf("Sending username for validation\n");
        if(send(nClient, username, strlen(username)+1, 0)<0)
        {
            printf("Error in sending the username for validation\n");
            closesocket(nClient);
            WSACleanup();
            exit(0);
        } 
        //username sent for validation.
        printf("sent username for validation\n");
        printf("receiving flag for username validation\n");
        if(recv(nClient,(char*)&result, sizeof(result),0)<0 )
        {
            printf("Error in receiving validation result.\n");
            closesocket(nClient);
            WSACleanup();
            exit(0);
        }
        result = ntohl(result);
        loading(3);
        if(result == 1)
        {
            printf("Username validated\n");
            break;
        }
    }while (result!=1);

    printf("---------------------------------------------\n");
    //fflush(stdin);
    int choice;
    do{
        printf(" Enter 1 : Start a New Chat \n Enter 2 : Join Someone Waiting\n Enter 3 : More about Developers\n Enter 4 : Quit \n>>> ");
        scanf("%d",&choice);
        if(validateChoice(choice)==1)
        {
            //printf("Sending choice: %d (converted to network byte order: %d)\n", choice, htonl(choice));

            choice = htonl(choice);
            if(send(nClient,(char*)&choice, sizeof(choice), 0)>0)
            {
                //printf("Sent choice: %d (converted to network byte order: %d)\n", choice, htonl(choice));
                break;
            }
            else
            {
                printf("Error in sending the choice.\n ");
            }
        }

    }while(validateChoice(choice)!=1);

    if(ntohl(choice)==3)
    {
        printf("<<<<<This Project was made by Team: NoobCoderzz>>>>>\n");
    }
    
    if(ntohl(choice)==2)
        {
            char buffer[100];
            int r;
            printf("List of Waiting Clients who chose option 1\n");
            printf("----------------------------------------------------------------\n");
            do
            {
                r = recv(nClient, buffer, sizeof(buffer),0); 
                if(r>0)
                {
                    buffer[r] = '\0';
                    if (strcmp(buffer, "END") == 0) 
                    {
                        printf("End of client list received.\n");
                        break;
                    }
                    printf("%s\n",buffer);
                }

            }while(r!=-1);
            printf("----------------------------------------------------------------\n");

            printf("Enter the id to connect with : ");
            int parterIDinput;
            scanf("%d",&parterIDinput);
            parterIDinput = htonl(parterIDinput);
            if(send(nClient,(char*)&parterIDinput, sizeof(parterIDinput),0)<0)
            {
                printf("Error in sending partner client id to the server.\n");
            }
            printf("Sent the parterIDinput to the server\n");
            fflush(stdin); 
            do{
                char buffer2[256];
                printf("type your text : ");
                fgets(buffer2, sizeof(buffer2),stdin);
                if(send(nClient,buffer2,sizeof(buffer2),0)<0)
                {
                    printf("Error in sending the message from the server");
                }
                r = recv(nClient, buffer2, sizeof(buffer2),0);
                if(r==0)
                {
                    printf("Conversation is closed\n");
                    break;
                }
                printf("     Recived text : " );
                int z=0;
                while(buffer2[z])
                {
                    if(buffer2[z] == ' ')
                    {
                        Sleep(60);
                    }
                    printf("%c",buffer2[z]);
                    Sleep(30);
                    z++;
                }
                printf("\n");
            }while(FD_ISSET);
        }

        if(ntohl(choice)==1)
        {
            printf("Please wait for the other client to join you\n");
            loading(10);
            // assuming the partner has connected to this client in this time interval.
            int r;
            do{
                fflush(stdin);
                char buffer[256];
                int k = recv(nClient, buffer, sizeof(buffer),0);
                if(k>0)
                {
                    printf("     Recived text : ");
                    int z=0;
                    while(buffer[z])
                    {
                        if(buffer[z] == ' ')
                        {
                            Sleep(60);
                        }
                        printf("%c",buffer[z]);
                        Sleep(30);
                        z++;
                    }
                    printf("\n");

                }
                if(k==0)
                {
                    printf("Conversation is closed :(\n");
                    break;
                }
                printf("type your text : ");
                fgets(buffer, sizeof(buffer),stdin);
                if(send(nClient,buffer,sizeof(buffer),0)<0)
                {
                    printf("Error in sending the message from the server");
                }
                
            }while(FD_ISSET);
        }
    

    closesocket(nClient);
    WSACleanup();
    return 0;

}
void loading(int n)
{
    int i = 0;
    while(i<n)
    {
        int k = 0; 
        while(k<3)
        {
            printf(".");
            k++;
            Sleep(200);
        }
        printf("\b\b\b");
        i++;
        Sleep(300);
    }
}
void instructions()
{
 printf(" _____      _                _____ _           _   \n");
 printf("|  __ \\    | |              / ____| |         | |  \n");
 printf("| |__) |___| |_ _ __ ___   | |    | |__   __ _| |_ \n");
 printf("|  _  // _ \\ __| '__/ _ \\  | |    | '_ \\ / _` | __|\n");
 printf("| | \\ \\  __/ |_| | | (_) | | |____| | | | (_| | |_ \n");
 printf("|_|  \\_\\___|\\__|_|  \\___/   \\_____|_| |_|\\__,_|\\__|\n");

 printf("\t<The chat goes like, one sends the message then the other will be able to send>\n");
 printf("\t                        <Something like a Wakie-Talkie>\n");
 printf("\t            <if the program gets stuck at some point, then result the program>\n");
 printf("\t                <To report error, please email at : ashutoshv6393@gmail.com>\n");
 printf("------------------------------------------------------------------------------------------------------\n");
                                                    
                                                    

}
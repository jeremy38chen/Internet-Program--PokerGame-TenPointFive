#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


char buf[100];
char ip[100]={0};
int  determine_connect = 0;
int  numbytes,a;
int  port;
struct sockaddr_in server;
char ch;
int  menu_card_point[100];// for menu display
int  menu_card_suit[100]; // for menu display

void SendData(int socketfd,char *choice);// send data to dealer
void RecvData(int socketfd,int *Turn,int *PlayerNum,int Round[],int Size_Round,int PlayerSuit[][5],int Size_PlayerSuit,int PlayerPoint[][5],int Size_PlayerPoint,int Chance[],int Size_Chance,int playershowinitialcard[],int Size_playershowinitialcard,int win[],int Size_win,int lose[],int Size_lose);// recevie data from dealer
void ShowCard(int turn,int PlayerNum,int Round[],int PlayerSuit[][5],int PlayerPoint[][5],int playershowinitialcard[],int win[],int lose[]);
void Display_menu_card();             // for card display in menu
void Display_intruction(int player);  // for intruction display
void Display_menu(char *ip,int *port);// for menu display
void Display_load(int count);         // for loading bar display
void Delay(int a);
void Hit_Card(int socketfd,int Turn,int player);// send choice to dealer


int main()
{
    int socketfd;
    int PlayerNum,player=0,startgame=0,PlayerSuit[10][5],PlayerPoint[10][5]={0},Round[10]={0},Turn,Chance[10]={0};
    /*
     Checkcard: whether the card is drawn or not (0 represents the card has not been drawn)
     Round:     How many cards does a player draw?
     Turn:      Which player is going to play the game?
     Chance:    Does the player have the right to draw a card? (0 represents "yes"，1 represents "no")
     Bust:      Is the player busted? (0 represents "yes"，1 represents "no")
     Dragon:    Does the player get "Five Dragon"? (0 represents "no"，1 represents "yes")
     BlackJack: Does the player get "Ten Point Half"? (0 represents "yes"，1 represents "no"
     */
    int playershowinitialcard[100]={0};
    int win[100]={0};
    int lose[100]={0};
    int count=0;
   
    
    system("clear");
    
    // Display menu
    Display_menu(ip,&port);
    
    
    // Creat Socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Setting IP and Port
    server.sin_family = AF_INET;
    server.sin_port = htons(port);// port, which htons converts host number to network order
    inet_pton(AF_INET, ip, &server.sin_addr);// convert readable IP to their binary representation
    
    // Request a connection with dealer
    connect(socketfd, (struct sockaddr *)&server, sizeof(server));

    
    // Receive player's label and startgame trigger from dealer
    // Player?
    if ((numbytes = recv(socketfd,&player, sizeof(int), 0)) == -1)
        perror("recv");
    // Start game?
    if ((numbytes = recv(socketfd,&startgame, sizeof(int), 0)) == -1)
        perror("recv");
    
    // Game start
    while (startgame==1 && determine_connect !=-1) {
        
        // Display instruction
        if (determine_connect == 0) {
            
            Delay(1);
            while (1) {
                
                system("clear");
                Display_intruction(player);
                
                printf("\t\t\t\t   ");
                Display_load(count);
                Delay(1);
                if (count == 20){determine_connect=1; break;}
                count++;
            }
            
        }
        
        // Receive card status from dealer
        RecvData(socketfd,&Turn,&PlayerNum,Round,sizeof(Round),PlayerSuit,sizeof(PlayerSuit),PlayerPoint,sizeof(PlayerPoint),Chance,sizeof(Chance),playershowinitialcard,sizeof(playershowinitialcard),win,sizeof(win),lose,sizeof(lose));
        system("clear");
        
        // Display screen for gaming
        ShowCard(player,PlayerNum,Round,PlayerSuit,PlayerPoint,playershowinitialcard,win,lose);
        Hit_Card(socketfd,Turn,player);
        // the dealer ends to draw card
        if (Chance[0]) {
            break;
        }
        
    }
    
    // Receive  game result from dealer
    RecvData(socketfd,&Turn,&PlayerNum,Round,sizeof(Round),PlayerSuit,sizeof(PlayerSuit),PlayerPoint,sizeof(PlayerPoint),Chance,sizeof(Chance),playershowinitialcard,sizeof(playershowinitialcard),win,sizeof(win),lose,sizeof(lose));
    
    system("clear");
    
    // Display result
    if (determine_connect != -1) {
        
        ShowCard(player,PlayerNum,Round,PlayerSuit,PlayerPoint,playershowinitialcard,win,lose);
        printf("\n-----Game End-----\n");
        
        if (win[player]==1) {
            printf("You win!!!\n");
        }
        else if(win[0]==1){
            printf("Dealer win!!!\n");
        }
        
        close(socketfd);
    }
    else{
        printf("Lost connection!!!\n");
    }
    
    return 0;
}

// Send choice
void SendData(int socketfd,char *choice){
    if (send(socketfd,choice,sizeof(char), 0) == -1){perror("send");}
}

// Receive parameter from server
void RecvData(int socketfd,int *Turn,int *PlayerNum,int Round[],int Size_Round,int PlayerSuit[][5],int Size_PlayerSuit,int PlayerPoint[][5],int Size_PlayerPoint,int Chance[],int Size_Chance,int playershowinitialcard[],int Size_playershowinitialcard,int win[],int Size_win,int lose[],int Size_lose){
    
    if ((numbytes = recv(socketfd, Turn, sizeof(int), 0)) == -1)
    {perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, PlayerNum, sizeof(int), 0)) == -1)
    {perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, Round, Size_Round, 0)) == -1)
    { perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, PlayerSuit, Size_PlayerSuit, 0)) == -1)
    { perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, PlayerPoint, Size_PlayerPoint, 0)) == -1)
    {perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, Chance, Size_Chance, 0)) == -1)
    {perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, playershowinitialcard, Size_playershowinitialcard, 0)) == -1)
    { perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, win, Size_win, 0)) == -1)
    {perror("recv");determine_connect=-1;}
    if ((numbytes = recv(socketfd, lose, Size_lose, 0)) == -1)
    {perror("recv");determine_connect=-1;}
}

void ShowCard(int turn,int PlayerNum,int Round[],int PlayerSuit[][5],int PlayerPoint[][5],int playershowinitialcard[],int win[],int lose[]){
    
    int i,j,a;
    
    printf("\n");
    printf("\n");
    
    
    for (a=0; a<=PlayerNum; a++) {
        
        if (turn != a || a == PlayerNum) {
            if (a == PlayerNum) {
                a = turn;
            }
            printf("\n");
            if (a==0) {
                printf(" Dealer:\n");
                if (lose[0]==1) {
                    printf(" LOSE");
                }
                if (win[0]==1) {
                    printf(" WIN ");
                }
                else{printf("     ");}
                
            }else {
                printf(" Player %d:\n",a);
                if (lose[a]==1) {
                    printf(" LOSE");
                }
                if (win[a]==1) {
                    printf(" WIN ");
                }
                else{printf("    ");}
                
            }
            
            printf("    \t");
            for (j=0;j<Round[a];j++) {
                printf("-----  ");
            }
            printf("\n");
            printf("\t\t");
            for (j=0;j<Round[a];j++) {
                if (j==0 && playershowinitialcard[a] != 1 && a !=turn) {printf("|▼▼▼|  ");continue;}
                else{
                    if (PlayerPoint[a][j]==10) {printf("|%d |  ",10);continue;}
                    if (PlayerPoint[a][j]==11) {printf("|J  |  ");continue;}
                    if (PlayerPoint[a][j]==12) {printf("|Q  |  ");continue;}
                    if (PlayerPoint[a][j]==13) {printf("|K  |  ");continue;}
                    printf("|%d  |  ",PlayerPoint[a][j]);
                }
            }
            printf("\n");
            printf("\t\t");
            for (j=0;j<Round[a];j++) {
                if (j==0 && playershowinitialcard[a] != 1 && a !=turn) {printf("|◆◆◆|  ");continue;}
                else{
                    if(PlayerSuit[a][j]==1){printf("| ♠ |  ");};
                    if(PlayerSuit[a][j]==2){printf("| ♡ |  ");};
                    if(PlayerSuit[a][j]==3){printf("| ♢ |  ");};
                    if(PlayerSuit[a][j]==4){printf("| ♣ |  ");};
                }
            }
            printf("\n");
            printf("\t\t");
            for (j=0;j<Round[a];j++) {
                if (j==0 && playershowinitialcard[a] != 1 && a !=turn) {printf("|▲▲▲|  ");continue;}
                else
                    printf("|   |  ");
                
            }
            printf("\n");
            printf("\t\t");
            for (j=0;j<Round[a];j++) {
                printf("-----  ");
            }
            printf("\n");
            if (a==turn) {
                break;
            }
            
        }
        
        
    }
    
}

void Display_menu_card(){
    
    int i;
    for ( i = 0; i<13; i++) {
        menu_card_point[i] = rand()%13 +1;
        menu_card_suit[i] = rand()%4 +1;
    }
    
    printf("    ");
    
    
    for (i=0;i<13;i++) {
        printf("-----  ");
    }
    printf("\n");
    printf("    ");
    
    for (i=0;i<13;i++) {
        if (menu_card_point[i]==10) {printf("|%d |  ",menu_card_point[i]);continue;}
        if (menu_card_point[i]==11) {printf("|J  |  ");continue;}
        if (menu_card_point[i]==12) {printf("|Q  |  ");continue;}
        if (menu_card_point[i]==13) {printf("|K  |  ");continue;}
        printf("|%d  |  ",menu_card_point[i]);
    }
    printf("\n");
    printf("    ");
    
    for (i=0;i<13;i++) {
        if(menu_card_suit[i]==1){printf("| ♠ |  ");};
        if(menu_card_suit[i]==2){printf("| ♡ |  ");};
        if(menu_card_suit[i]==3){printf("| ♢ |  ");};
        if(menu_card_suit[i]==4){printf("| ♣ |  ");};
    }
    printf("\n");
    printf("    ");
    
    for (i=0;i<13;i++) {
        printf("|   |  ");
        
    }
    printf("\n");
    printf("    ");
    
    for (i=0;i<13;i++) {
        printf("-----  ");
    }
    printf("\n");
    
}

void Display_menu(char *ip,int *port){
    
    int c,d,count=0;
    printf("\n");
    printf("\t\t\t\t\t");
    printf("    CARD GAME ");
    printf("\n\n\n");
    Display_menu_card();
    printf("\n\n\n\n");
    printf("\t\t\t\t      ");
    printf(" insert Dealer's IP :\n");
    printf("\t\t\t\t      ");
    scanf("%s",ip);
    getchar();
    printf("\t\t\t\t      ");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\t\t\t\t      ");
    printf(" insert the Port number :\n");
    printf("\t\t\t\t      ");
    scanf("%d",port);
    while(getchar() != '\n');
    printf("\t\t\t\t      ");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣\n\n\n\n");
    printf("\t\t\t\t\t\t\t\t\t");
    printf("Waiting for response\n");
    
}

void Display_intruction(int player){
    
    printf("\n\n\n");
    printf("\t\t\t\t\t");
    printf("INSTRUCTION:\n");
    printf("\t\t");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\t");
    printf("10.5 card game is a kind of poker game which is going to have one Dealer and several players in a game.Firstly, each player and the Dealer are going to recieve a card which can't be seen by others. Then, Depending on each player 's perference or consideration the player can decide the option whether they are going to take the another card repeatedly or not. If the total of your card is larger than 10.5 or smaller than the card of the Dealer, you lose the game.\n\n\n");
    printf("\t\t\t\t");
    printf("REPRESENTATION OF THE CARD:\n");
    printf("\t\t");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\t");
    printf("The number of the card presents equally to the point except J,Q,K representing 0.5 point.\n\n\n");
    printf("\t\t\t\t\t");
    printf("CONTROL:\n");
    printf("\t\t");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\t\t\t\t");
    printf("Press y to get a new card\n\n");
    printf("\t\t\t\t");
    printf("Press n to finish this round\n\n\n");
    printf("\t\t\t\t\t\t\t\t\t");
    printf("Game postision: Player %d\n",player);
    printf("\t\t\t\t\t");
    printf("Connecting\n");
    
}

void Display_load(int count){
    
    switch (count) {
        case 1:printf("◼︎\n");break;
        case 2:printf("◼︎◼︎\n");break;
        case 3:printf("◼︎◼︎◼︎\n");break;
        case 4:printf("◼︎◼︎◼︎◼︎\n");break;
        case 5:printf("◼︎◼︎◼︎◼︎◼︎\n");break;
        case 6:printf("◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 7:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 8:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 9:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 10:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 11:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 12:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 13:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 14:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 15:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 16:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 17:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 18:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 19:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
        case 20:printf("◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎◼︎\n");break;
    }
    
}

void Delay(int a){
    int c,d;
    for ( c = 1 ; c <= 12767*a ; c++ )
        for ( d = 1 ; d <= 16767*a ; d++ )
        {};
    
}

void Hit_Card(int socketfd,int Turn,int player){
    
    while (1) {
        if (Turn ==0) {printf("Dealer : Hit or not(y/n)?\n");}
        else{printf("Player %d : Hit or not(y/n)?\n",Turn);}
        if(Turn == player)
        {
            ch = getchar();
            getchar();
        }
        else if(Turn ==0){printf("Waiting for the response from Dealer....\n");break;}
        else{printf("Waiting for the response from other player....\n"); break;}
        
        // the player wants to draw a card
        if(ch=='y'||ch=='Y'){
            SendData(socketfd,&ch);
            break;
        }
        // the player don't want to draw a card
        else if(ch=='n'||ch=='N'){
            SendData(socketfd,&ch);
            break;
        }
        else {printf("Please enter Y or N!\n");continue;}
    }
}



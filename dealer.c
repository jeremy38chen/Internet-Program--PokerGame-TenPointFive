#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define SPADES   1
#define HEARTS   2
#define DIAMONDS 3
#define CLUBS    4


fd_set master;    // master file descriptor list
fd_set read_fds;  // temp file descriptor list for select()
int fdmax;        // maximum file descriptor number
int listener;     // listening socket descriptor
int newfd;        // newly accept()ed socket descriptor
struct sockaddr_in addr;
struct sockaddr_in client;
socklen_t addrlen;
char buf[256];    // buffer for client data
char choice;      // store the player's cloice Y or N
char ip[100]={0}; // store IP address
int fd_active;    // use in the function select() to determine action from client
int fdmax;        // maximum file descriptor number
int listener;     // listening socket descriptor
int newfd;        // new socket descriptor
int nbytes;
int port;
int socketdescriptor[100]={0};// store client's socketdescriptor
int determine_connect =0;    // variable to check the connection
int player =0;
float Sum[10]={0};// store the total sum of the points
int playershowinitialcard[100]={0};// show the first card
int win[100]={0};        // Who lost the game?
int lose[100]={0};       // Who won the game?
int totalwinlose=0;      // Does the dealer need to draw a card?
int menu_card_point[100];// for menu display
int menu_card_suit[100]; // for menu display
int count=0;
int PlayerNum,PlayerSuit[10][5],PlayerPoint[10][5]={0},Suit,Point; //The number of players, player's suit, player's points, temporary storage for suit,temporary storage for point
int Checkcard[5][14]={0},Round[10]={0},Turn,Chance[10]={0},Bust[10]={0},Dragon[10]={0},BlackJack[10]={0};
/*
 Checkcard: whether the card is drawn or not (0 represents the card has not been drawn)
 Round:     How many cards does a player draw
 Turn:      Which player is going to play the game?
 Chance:    Does the player have the right to draw a card? (0 represents "yes"，1 represents "no")
 Bust:      Is the player busted? (0 represents "yes"，1 represents "no")
 Dragon:    Does the player get "Five Dragon"? (0 represents "no"，1 represents "yes")
 BlackJack: Does the player get "Ten Point Half"? (0 represents "yes"，1 represents "no"
 */

void RecvData(int socket_fd);// recevie data from players
void SendData(int playertosend,int *Turn,int *PlayerNum,int Round[],int Size_Round,int PlayerSuit[][5],int Size_PlayerSuit,int PlayerPoint[][5],int Size_PlayerPoint,int Chance[],int Size_Chance,int playershowinitialcard[],int Size_playershowinitialcard,int win[],int Size_win,int lose[],int Size_lose);// send data to players
void RandomDraw();
void Deal(int turn);// deal card to every player
void CountSum();    // count the sum of the point
void ShowCard(int turn,int PlayerNum,int Round[],int PlayerSuit[][5],int PlayerPoint[][5],int playershowinitialcard[],int win[],int lose[]);   // for card display
void Display_load(int count);// for loading bar display
void Display_menu_card();    // for card display in menu
void Display_menu(char *ip,int *port); // for menu display
void Display_intruction();   // for intruction display
void Delay(float a);
void Check_Bust();           // Does the player get busted? or"Five Dragon"? or"Ten Point Half"?
void Enter_PlayerNumber();   // for information display
void Hit_Card();             // The function for drawing a card
void Check_Turn();           // Which player is going to play the game?
void Check_Winner();         // Who won the game?
void SetShowinitialcard();


int main(void)
{
    int a,startgame=0,Menu=0;
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    
    srand(time(NULL));
    
    //Display menu
    system("clear");
    Display_menu(ip,&port);
    
    // Create Socket
    listener =  socket(AF_INET, SOCK_STREAM, 0);
    
    // Setting IP and Port
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);// port, which htons converts host number to network order
    addr.sin_addr.s_addr = INADDR_ANY;// bind to the IP of the host it's running on
    
    // Associate the socket with Port
    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    
    // Waiting for request
    listen(listener, 5);
    
    FD_SET(listener, &master);// add the listener to the master set
    
    fdmax = listener;// keep track of the biggest file descriptor
    
    // Dealing cards to every player
    for(a=0;a<PlayerNum;a++){
        Deal(a);
    }
    
    Turn = 1;// player1 plays the game first.
    
    // main loop
    while(1) {
        
        // if the dealer established connection with every player, the screen prints the instruction.
        if (startgame == 1 && Menu==0) {
            
            Delay(1);
            while (1) {
                system("clear");
                Display_intruction();
                
                printf("\t\t\t\t   ");
                Display_load(count);
                Delay(1);
                if (count == 20) {Menu=1;break;}
                count++;
            }
            
        }
        // after the instruction is printed, the game starts!
        else if(startgame == 1 && Menu==1) {
            
            system("clear");
            ShowCard(0,PlayerNum,Round,PlayerSuit,PlayerPoint,playershowinitialcard,win,lose);
            if (Turn !=0) {
                printf("Waiting for player %d to hit card..\n",Turn);
            }
            
        }
        
            read_fds = master; // copy it
        
            // turns for the dealer
            if (Turn==0) {
                if(Chance[0]){
                    Check_Winner();
                    // sending card status to each player and game end
                    for (a=1; a<=player; a++) {
                    SendData(a,&Turn,&PlayerNum,Round,sizeof(Round),PlayerSuit,sizeof(PlayerSuit),PlayerPoint,sizeof(PlayerPoint),Chance,sizeof(Chance),playershowinitialcard,sizeof(playershowinitialcard),win,sizeof(win),lose,sizeof(lose));
                    };
                    break;
                }
                
                else{
                
                    Hit_Card(socketdescriptor[Turn]);
                    Check_Bust();
                    Check_Turn();
                    
                    for (a=1; a<=player; a++) {
                        SendData(a,&Turn,&PlayerNum,Round,sizeof(Round),PlayerSuit,sizeof(PlayerSuit),PlayerPoint,sizeof(PlayerPoint),Chance,sizeof(Chance),playershowinitialcard,sizeof(playershowinitialcard),win,sizeof(win),lose,sizeof(lose));
                    };
                    continue;};
            }
        
            select(fdmax+1, &read_fds, NULL, NULL, NULL);
        
            for(fd_active = 0; fd_active <= fdmax; fd_active++) {
                
                if (FD_ISSET(fd_active, &read_fds)) {
                    
                    // New connection
                    if (fd_active == listener) {
                        // handle new connections
                        
                        addrlen = sizeof client;
                        
                        // send acceptation to new player
                        newfd = accept(listener,(struct sockaddr *)&client,&addrlen);
                        player++;
                        socketdescriptor[player]=newfd;
                        
                        FD_SET(newfd, &master); // add to master set
                        
                        // keep track of the max
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        
                        // send player's label to new player
                        if (send(socketdescriptor[player],&player,sizeof(int),0)== -1)
                        {perror("send");}
                        
                        // when the dealer established connection with every player, send "startgame = 1" to every player to start the game.
                        if (player == PlayerNum-1) {
                            startgame=1;
                            for (a=1; a<=PlayerNum; a++) {
                                if (send(socketdescriptor[a],&startgame,sizeof(int),0)== -1)
                                { perror("send");}
                            }
                        }
                    
                    }
                    // Active connection
                    else {
                            
                        Hit_Card(socketdescriptor[Turn]);
                        Check_Bust();
                        Check_Turn();
                        
       
                    }
                }
            }

            if (startgame==1) {
                
                // Sending card status to each player
                for (a=1; a<=player; a++) {
                    SendData(a,&Turn,&PlayerNum,Round,sizeof(Round),PlayerSuit,sizeof(PlayerSuit),PlayerPoint,sizeof(PlayerPoint),Chance,sizeof(Chance),playershowinitialcard,sizeof(playershowinitialcard),win,sizeof(win),lose,sizeof(lose));
                }
                
            }
        
    }
    system("clear");
    // Display result
    ShowCard(0,PlayerNum,Round,PlayerSuit,PlayerPoint,playershowinitialcard,win,lose);
    Check_Winner();
    
    return 0;
}

void RecvData(int socket_fd){
    int player;
    
    if ((nbytes = recv(socket_fd,&choice, sizeof(char), 0)) <= 0) {
        
        close(socket_fd);
        FD_CLR(socket_fd, &master); // remove from master set
        for (player=0; player<100; player++) {
            if (socketdescriptor[player] == socket_fd) {
                socketdescriptor[player] = 0;//socket close
            }
        }
    }
    
}

void SendData(int playertosend,int *Turn,int *PlayerNum,int Round[],int Size_Round,int PlayerSuit[][5],int Size_PlayerSuit,int PlayerPoint[][5],int Size_PlayerPoint,int Chance[],int Size_Chance,int playershowinitialcard[],int Size_playershowinitialcard,int win[],int Size_win,int lose[],int Size_lose){
    
    if(socketdescriptor[playertosend]!=0){
        if (send(socketdescriptor[playertosend],Turn,sizeof(int), 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],PlayerNum,sizeof(int), 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],Round,Size_Round, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],PlayerSuit,Size_PlayerSuit, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],PlayerPoint,Size_PlayerPoint, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],Chance,Size_Chance, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],playershowinitialcard,Size_playershowinitialcard, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],win,Size_win, 0) == -1){perror("send");}
        
        if (send(socketdescriptor[playertosend],lose,Size_lose, 0) == -1){perror("send");}
        
        
    }
    
}

void RandomDraw(){
    Suit = rand()%4 + 1;
    Point = rand()%13 + 1;
}

void Deal(int turn){
    if(Round[turn]<5){
        while (1) {
            RandomDraw();
            PlayerSuit[turn][Round[turn]] = Suit;
            PlayerPoint[turn][Round[turn]] = Point;
            if(Checkcard[Suit][Point]==0) break;   // if the card hasn't been used, stop the loop.
        }
        Checkcard[Suit][Point] = 1;    // the card has been used
        Round[turn]++;
    }
    else printf("More than 5 card!\n");
    
}

void CountSum(){
    int i,j;
    for(i=0;i<PlayerNum;i++){Sum[i]=0;}
    
    for(i=0;i<PlayerNum;i++){
        for(j=0;j<Round[i];j++){
            if(PlayerPoint[i][j]==11||PlayerPoint[i][j]==12||PlayerPoint[i][j]==13){Sum[i]+=0.5;}
            else{Sum[i]+=PlayerPoint[i][j];}
        }
    }
    
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

void Display_menu(char *ip, int *port){
    int c,d,count=0;
    printf("\n\n");
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
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\t\t\t\t      ");
    Enter_PlayerNumber();
    printf("\t\t\t\t      ");
    printf("￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
    printf("\n");
    printf("\t\t\t\t\t\t\t\t\t");
    printf("Waiting for response\n");
    
}

void Display_intruction(){
    
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
    printf("Game postision: Dealer\n");
    printf("\t\t\t\t\t");
    printf("Connecting\n");
    
}



void Delay(float a){
    int c,d;
    for ( c = 1 ; c <= 12767*a ; c++ )
        for ( d = 1 ; d <= 16767*a ; d++ )
        {};
    
}

void Check_Bust(){
    int i,Check_AllBust=0,Check_AllBlackJack=0,Check_AllDragon=0;
    
    CountSum();
    if(Sum[Turn]>10.5){printf("Bust!!\n");Bust[Turn]=1;lose[Turn]=1;Chance[Turn]=1;}                        // bust
    else if(Sum[Turn]== 10.5){printf("\n-----10.5!-----\n");BlackJack[Turn]=1;win[Turn]=1;Chance[Turn]=1;}// Ten Point Half
    else if(Round[Turn]==5){printf("\n-----FIVE DRAGON!-----\n");Dragon[Turn]=1;win[Turn]=1;Chance[Turn]=1;}; // Five Dragon
    
    SetShowinitialcard();
    
    for (i=1; i<PlayerNum; i++) {
        Check_AllBust = Check_AllBust + Bust[i];
        Check_AllBlackJack = Check_AllBlackJack + BlackJack[i];
        Check_AllDragon = Check_AllDragon + Dragon[i];
    }
    // every player is busted || every player gets a "Ten  Point Half" || every player gets a "Five Dragon"
    if (Check_AllBust == PlayerNum-1 || Check_AllBlackJack == PlayerNum-1 || Check_AllDragon == PlayerNum-1) {
        Chance[0]=1;
    }
    if(Check_AllBlackJack == PlayerNum-1 || Check_AllDragon == PlayerNum-1)lose[0]=1;// the dealer won
    if (Check_AllBust == PlayerNum-1)win[0]=1;// the dealer won
    
    for (i=1;i<PlayerNum;i++) {
        totalwinlose = win[i]+lose[i];
    }
    if(totalwinlose==PlayerNum-1)Chance[0]=1;// check whether each player finished the game
    
    
}

void Enter_PlayerNumber(){
    printf("Enter number of player:\n");
    
    while(1){
        printf("\t\t\t\t      ");
        while(scanf("%d",&PlayerNum) != 1){
            printf("\t\t\t\t      ");
            printf("Please enter NUMBER!\n");
            while(getchar() != '\n');
            printf("\t\t\t\t      ");
        }
        
        if (PlayerNum>=2 && PlayerNum <=10) {
            break;
        }
        else {printf("\t\t\t\t      ");printf("Please enter 2~10!\n");}
    }
    getchar();
}


void Hit_Card(int socketfd){
    
    
    while (1) {
        
        if (Turn ==0) {
            printf("Dealer : Hit or not(y/n)?\n");
            choice = getchar();
            getchar();
        }
        else{
            RecvData(socketfd);// receive choice('y' or 'n') from player
        }
        
        if(choice=='y'||choice=='Y'){
            Deal(Turn);break;
        }
        else if(choice=='n'||choice=='N'){
            Chance[Turn] = 1;
            break;
        }
        else {printf("Please enter Y or N!\n");continue;}
    }
}

void Check_Turn(){
    
    if(Chance[Turn]==1||Bust[Turn]==1){
        if(Turn<PlayerNum-1 && Turn != 0){Turn++;}// turns for each player
        else if(Turn== PlayerNum-1){Turn = 0;}// turns for the dealer
        else if(Turn==0){;}
        else printf("there is problem for 'Turn'\n");
    }
    
}


void Check_Winner(){
    int i,a;
    CountSum();
    printf("\n-----Game End-----\n");
    for (i=1;i<PlayerNum;i++) {
        if(Bust[0]==1){printf("You lose to everyone : You is busted QQ\n");
            for (a=1;a<PlayerNum;a++) {
                if(Bust[a]!=1)win[a]=1;
            }
            break;}// the dealer lost to every player
        
        // the player got a "Five Dragon", the dealer loses.
        else if(Dragon[i]==1){printf("You lose to Player %d : Player got Five Dragon\n",i);}
        // the player got a "Ten Point Half", the dealer loses.
        else if(BlackJack[i]==1){printf("You lose to Player %d : Player got 10.5\n",i);}
        // the dealer got a "Five Dragon" or "Ten Point Half", the dealer wins
        else if(Dragon[0]==1){printf("You win to Player %d : You got 5 Dragon\n",i+1);lose[i]=1;win[0]=1;}
        // the player is busted, the dealer wins
        else if(Bust[i]==1){printf("Player %d is busted : You Win\n",i);}
        // the dealer's total number of points was larger than the player's, the dealer wins.
        else if(Sum[i]<=Sum[0]){printf("You win Player %d : Your point is bigger\n",i);playershowinitialcard[0]=1;playershowinitialcard[i]=1;
            lose[i]=1;win[0]=1;}
        // the dealer's total number of points was less than the player's, the dealer loses.
        else if(Sum[i]>Sum[0]){printf("You lose to Player %d : Your point is smaller\n",i);playershowinitialcard[0]=1;playershowinitialcard[i]=1;lose[0]=1;win[i]=1;}
        
    }
    
}
void SetShowinitialcard(){
    int i;
    CountSum();
    for (i=1;i<PlayerNum;i++) {
        if(Bust[0]==1){playershowinitialcard[0]=1; break;}
        else if(Dragon[i]==1){playershowinitialcard[i]=1;}
        else if(BlackJack[i]==1){playershowinitialcard[i]=1;}
        else if(Dragon[0]==1){playershowinitialcard[0]=1;}
        else if(Bust[i]==1){playershowinitialcard[i]=1;}
        
        
    }
}



#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>

#define BUF_SIZE 1000
#define NORMAL_SIZE 20

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handler(char *msg);

void menu();
void changeName();
void menuOptions(int sock);
void dutchPay(int sock);
void miniGame(int sock);
void fileTrasfer(int sock);
void fileDownload(int sock);

char filename[NORMAL_SIZE];
char name[NORMAL_SIZE]="[DEFALT]";
char msg_form[NORMAL_SIZE];
char s_time[NORMAL_SIZE];
char msg[BUF_SIZE];
char s_port[NORMAL_SIZE];
char c_ip[NORMAL_SIZE];

volatile int flag=0;
volatile int flagDetail=0;
volatile int gameflag=0;

pthread_mutex_t mutx;

int main(int argc,char *argv[])
{
    int sock;
    struct sockaddr_in s_adr;
    pthread_t send_thread,recv_thread;
    void* thread_return;

    if(argv!=4)
    {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
        exit(1);
    }

    /** local time **/
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
    sprintf(s_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,t->tm_min);

    sprintf(name, "  [%s] -", argv[3]);
    sprintf(c_ip, "%s", argv[1]);
    sprintf(s_port, "%s", argv[2]);

    sock=socket(PF_INET, SOCK_STREAM,0);
    memset(&s_adr,0,sizeof(s_adr));
    s_adr.sin_family = AF_INET;
    s_adr.sin_addr.s_addr=inet_addr(argv[1]);
    s_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr_in*)&s_adr,sizeof(s_adr))==-1) error_handler("Connect Error");

    menu();

    pthread_create(&send_thread,NULL,send_msg,(void *)&sock);
    pthread_create(&recv_thread,NULL,recv_msg,(void *)&sock);
    pthread_join(send_thread,&thread_return);
    pthread_join(recv_thread,&thread_return);

    close(sock);

    return 0;

}

void *send_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who;
    char temp[BUF_SIZE];


    int k=0;

    printf(">> Join the Chat\n");
    sprintf(myInfo,"%s's join. IP : %s",name,c_ip);
    write(sock,myInfo,strlen(myInfo));

    while(1)
    {
        fgets(msg,BUF_SIZE,stdin);
        if(!strcmp(msg,"!menu\n"))
        {
            menuOptions(sock);
        }
        else if(!strcmp(msg,"q\n") || !strcmp(msg,"Q\n"))
        {
            close(sock);
            exit(0);
        }

        sprintf(name_msg,"%s %s",name,msg);
        write(sock,name_msg,strlen(name_msg));
    }

    return NULL;

}
void *recv_msg(void *arg)
{
    int sock = *((int *)arg);
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    int str_len;
    char recvFlag[NORMAL_SIZE];
    char temp[NORMAL_SIZE];

    int Flag =0;

    while(1)
    {
        if(flag==2)
        {
            read(sock,recvFlag,1);
            Flag=atoi(recvFlag);
            if(Flag==1) printf("DOWN\n");
            else if(Flag==2)    printf("UP\n");
            else if(Flag==3)    printf("Error,Retry\n");
            else if(!strncmp(recvFlag,"miniGame",strlen("miniGame")))
            {
                printf("Congratulations! You are WINNER!\n");
                memset(name_msg,0,sizeof(name_msg));
                flag=0;
            }
        }
        else if(flag==4 || flagDetail==9)
        {
            FILE *fp;
            char filebuf[100];
            memset(filebuf,0x00,100);
            int read_cnt;
            char fileSize[BUF_SIZE];
            int ifSize=0;
            strcpy(fileSize,name_msg);
            fp=fileopen(filename,"wb");
            ifSize = atoi(fileSize);
            memset(name_msg,0,sizeof(name_msg));
            usleep(4000000);
            read_cnt = read(sock,filebuf,ifSize);
            fwrite((void *)filebuf,1,read_cnt,fp);
            printf("%s is stored!!",filename);
            fclose(fp);
            flag=0;
            flagDetail=0;
        }
        str_len=read(sock,name_msg,NORMAL_SIZE+BUF_SIZE-1);
        if(str_len==-1) return (void*)-1;
        name_msg[str_len]=0;
        fputs(name_msg,stdout);
    }
    return NULL;
}
void error_handler(char *msg)
{
    fputs(msg,stderr);
    fputs("\n",stderr);
    exit(1);
}

void menu()
{
    system("clear");
    printf(" **** Chat Client ****\n");
    printf(" server port : %s \n", s_port);
    printf(" client IP   : %s \n", c_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", s_time);
    printf(" ************* menu ***************\n");
    printf(" if you want to select menu -> !menu\n");
    printf(" 1. change name\n");
    printf(" 2. clear/update\n");
    printf(" **********************************\n");
    printf(" Exit -> q & Q\n\n");
}
void changeName()
{
    char nameTemp[100];
    printf("Input new name : ");
    scanf("%s",nameTemp);
    strcpy(msg,name);
    strcat(msg,"--> Change Name -->");
    sprintf(name,"[%s]",nameTemp);
    printf("\nComplete\n");
}
void menuOptions(int sock)
{
    int select;
    printf("\n***** menu mode *****\n");
    printf("1. change name\n");
    printf("2. clear / update\n");
    printf("3. dutchpay\n");
    printf("4. minigame\n");
    printf("5. file transfer\n");
    printf("6. file download\n");
    printf("the other key is cancel\n");
    printf("********************\n");
    printf(">> ");

    scanf("%d",&select);
    getchar();
    switch(select)
    {
        case 1 :
            changeName();
            flag=0;
            break;
        case 2 :
            menu();
            flag=0;
            break;
        case 3 :
            printf("dutchpay function start\n");
            flag=1;
            dutchPay(sock);
            break;
        case 4:
            printf("minigame function start\n");
            flag=2;
            miniGame(sock);
            break;
        case 5 :
            printf("filetransfer function start\n");
            flag=3;
            fileTrasfer(sock);
            break;
        case 6 : 
            printf("filedownload function start\n");
            flag=4;
            fileDownload(sock);
            flagDetail=9;
            flag=0;
            usleep(1000000);
            break;
        default :
            printf("cancel.\n");
            flag=0;
            break;
    }
}
void dutchPay(int sock)
{
    int price;
    char totalPrice[100];
    int howMany;
    char howm[100];

    strcpy(msg,"dutch"); // function flag
    write(sock,msg,BUF_SIZE);
    
    printf("Input How many? ");
    scanf("%d",&howMany);
    sprintf(howm,"%d",howMany);
    write(sock,howm,2); //People

    printf("Input total price : ");
    scanf("%d",&price);
    sprintf(totalPrice,"%d",price);
    write(sock,totalPrice,10); //Total Price

    memset(msg,0,sizeof(msg));
}
void miniGame(int sock)
{
    int gameNum;
    int gamec[100];
    char under[]="Under\n";
    char up[]="Up\n";
    char plz[]="Please Input Number\n";

    while(1)
    {
        strcpy(msg,"miniGame");
        write(sock,msg,strlen(msg));
        memset(msg,0,sizeof(msg));

        printf("Guess Number!! Choose between 1~999(second line is fake) : \n");
        scanf("%d",&gameNum);

        sprintf(gamec,"%d",gameNum);
        write(sock,gamec,4);
    }
    memset(msg,0,sizeof(msg));
}
void fileTrasfer(int sock)
{
    int i=0;
    FILE *fp;
    char fileBuf[BUF_SIZE];
    int read_cnt;
    char name_cnt[NORMAL_SIZE];
    int ifSize=0;
    char fSize[5];

    char name_msg[NORMAL_SIZE+BUF_SIZE];
    memset(name_msg,0,sizeof(name_msg));
    strcpy(msg,"transfer");
    write(sock,msg,strlen("transfer"));
    
    printf("Input filename : ");
    fgets(filename,NORMAL_SIZE,stdin);

    sprintf(name_cnt,"%d",strlen(filename));
    write(sock,name_cnt,2);

    write(sock,filename,strlen(filename));

    fp=fopen(filename,"rb");
    fseek(fp,0,SEEK_END);
    ifSize = ftell(fp);
    fseek(fp,0,SEEK_SET);
    
    sprintf(fSize,"%d",ifSize);
    write(sock,fSize,5);
    
    if(fp!=NULL)
    {
        while(1)
        {
            read_cnt = fread((void *)fileBuf,1,ifSize,fp);
            if(read_cnt<ifSize)
            {
                write(sock,fileBuf,read_cnt);
                break;
            }
            write(sock,fileBuf,read_cnt);
        }
        fclose(fp);
    }
}
void fileDownload(int sock)
{
    char name_msg[NORMAL_SIZE+BUF_SIZE];
    memset(name_msg,0,sizeof(name_msg));
    
    FILE *fp;
    char filebuf[BUF_SIZE];
    int read_cnt;
    char name_cnt[2];
    char filesize[5];
    int ifSize=0;
    char fSize[5];

    strcpy(msg,"download");
    write(sock,msg,strlen(msg));

    printf("Input Filename : ");
    fgets(filename,NORMAL_SIZE,stdin);
    
    sprintf(name_cnt,"%d",strlen(filename));
    write(sock,name_cnt,2);

    write(sock,filename,strlen(filename));

}

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

pthread_mutex_t mutx;

int main(int argc,char *argv[])
{
    int sock;
    struct sockaddr_in s_adr;
    pthread_t send_thread,recv_thread;
    void* thread_return;

    if(argc!=4)
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

    printf(">> Join the Chat\n");
    sprintf(myInfo,"%s's join. IP : %s\n",name,c_ip);
    write(sock,myInfo,strlen(myInfo));

    while(1)
    {
        scanf("\n");
        fgets(msg,BUF_SIZE,stdin);

        if(!strcmp(msg,"!menu\n"))
        {
            menuOptions(sock);
        }
        if(!strcmp(msg,"q\n") || !strcmp(msg,"Q\n"))
        {
            close(sock);
            exit(0);
        }
        else if(flag==1)//dutchpay
        {
            dutchPay(sock);
            flag=0;
            memset(msg,0,sizeof(msg));
            continue;
        }
        else if(flag==2) //fileTransfer
        {
            memset(name_msg,0,sizeof(name_msg));
            fileTrasfer(sock);
            flag=0;
            continue;
        }
        else if(flag == 3) //fileDownload
        {
            memset(name_msg,0,sizeof(name_msg));
            fileDownload(sock);
            flagDetail=9;
            flag=0;
            continue;
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
         if(flag==3 || flagDetail==9)
        {
            FILE *fp;
            char filebuf[100];
            memset(filebuf,0x00,100);
            int read_cnt;
            char fileSize[BUF_SIZE];
            int ifSize=0;
            strcpy(fileSize,name_msg);
            fp=fopen(filename,"wb");
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
    printf(" **********************************\n");
    printf(" Exit -> q & Q\n\n");
}
void changeName()
{
    char nameTemp[100];
    printf("\tInput new name : ");
    scanf("%s",nameTemp);
    strcpy(msg,name);
    strcat(msg,"--> Change Name -->");
    sprintf(name,"[%s]",nameTemp);
    printf("\n\tComplete\n");
}
void menuOptions(int sock)
{
    int select;
    printf("\n\t***** menu mode *****\n");
    printf("\t1. change name\n");
    printf("\t2. clear / update\n");
    printf("\t3. dutchpay\n");
    printf("\t4. file transfer(Only Text file)\n");
    printf("\t5. file download(Only Text file)\n");
    printf("\tthe other key is cancel\n");
    printf("\n\t********************\n");
    printf("\n\t>> ");

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
            printf("\tdutchpay function start\n");
            flag=1;
            break;
        case 4 :
            printf("\tfiletransfer function start\n");
            flag=2;
            break;
        case 5 : 
            printf("\tfiledownload function start\n");
            flag=3;
            break;
        default :
            printf("\tcancel.");
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
    write(sock,msg,strlen("dutch"));
    
    printf("Input How many? ");
    scanf("%d",&howMany);
    sprintf(howm,"%d",howMany);
    write(sock,howm,2); //People

    printf("Input total price : ");
    scanf("%d",&price);
    sprintf(totalPrice,"%d",price);
    write(sock,totalPrice,10); //Total Price
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

    strcpy(msg,"transfer");
    write(sock,msg,strlen("transfer"));
    
    printf("Input filename : ");
    fgets(filename,NORMAL_SIZE,stdin);

    for(i=0;filename[i]!=0;i++)
    {
        if(filename[i]=='\n')
        {
            filename[i]=0;
            break;
        }
    }

    sprintf(name_cnt,"%d",strlen(filename));
    write(sock,name_cnt,2); //filename length

    write(sock,filename,strlen(filename)); //filename notice

    fp=fopen(filename,"rb");
    fseek(fp,0,SEEK_END);
    ifSize = ftell(fp); //fsize=filesize
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
    }
    
    fclose(fp);
}
void fileDownload(int sock)
{
    int i;
    FILE *fp;
    char filebuf[BUF_SIZE];
    int read_cnt;
    char name_cnt[2];
    char filesize[5];
    int ifSize=0;
    char fSize[5];

    strcpy(msg,"download");
    write(sock,msg,strlen("download"));

    printf("Input Filename : ");
    fgets(filename,NORMAL_SIZE,stdin);

     for(i=0;filename[i]!=0;i++)
    {
        if(filename[i]=='\n')
        {
            filename[i]=0;
            break;
        }
    }
    
    sprintf(name_cnt,"%d",strlen(filename));
    write(sock,name_cnt,2);

    write(sock,filename,strlen(filename));

}

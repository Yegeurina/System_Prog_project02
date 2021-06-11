#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>

#define BUF_SIZE 1000
#define MAX_CLIENT 10
#define MAX_IP 30
#define NAME_SIZE 20

void error_handler(char*msg);
void* client_handler(void *arg);
void send_msg(char *msg,int len);
void menu(char port[]);
char* serverState(int count);

char msgpay[BUF_SIZE];
int flag=0;

int client_cnt;
int client_socket[MAX_CLIENT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
    int s_sock, c_sock;
    struct sockaddr_in s_adr, c_adr;
    int c_adr_size;
    pthread_t t_id;

    //set time log
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);

    if(argc!=2) //port input error
    {
        printf(" Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
    //port information
    menu(argv[1]);

    pthread_mutex_init(&mutx,NULL);
    s_sock=socket(PF_INET, SOCK_STREAM, 0);

    //server socket set : TCP, IPv4
    memset(&s_adr, 0, sizeof(s_adr));
    s_adr.sin_family=AF_INET;
    s_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    s_adr.sin_port=htons(atoi(argv[1]));
    
    if (bind(s_sock, (struct sockaddr*)&s_adr, sizeof(s_adr))==-1)    error_handler("Bind Error");
    if(listen(s_sock,5)==-1)    error_handler("Listen Error");

    while(1)
    {
        t=localtime(&timer);
        if(client_cnt<MAX_CLIENT)
        {
            c_adr_size = sizeof(c_adr);
            c_sock=accept(s_sock,(struct sockaddr*)&c_adr,&c_adr_size);
            
            pthread_mutex_lock(&mutx);
            client_socket[client_cnt++]=c_sock;
            pthread_mutex_unlock(&mutx);

            pthread_create(&t_id,NULL,client_handler,(void*)&c_sock); //thread start
            pthread_detach(t_id);

            printf("Connected Client IP : %s",inet_ntoa(c_adr.sin_addr));
            printf("(%4d-%02d-%02d %02d:%02d)\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min);
            printf("User(%d/%d)\n",client_cnt,MAX_CLIENT);
        }
        else
        {
            printf("Too many User\n");
        }
    }
    close(s_sock);
    return 0;
    

}
void* client_handler(void *arg)
{
    int c_sock = *((int *)arg);
    int str_len =0 , i;
    char msg[BUF_SIZE];

    FILE *fp;
    char filename[BUF_SIZE];
    char filebuf[BUF_SIZE];

    char name_cnt[2];

    size_t bufsize =0;
    int nbyte;
    memset(filebuf,0x00,30);
    char filesize[5];

    int read_cnt;

    char totalprice[BUF_SIZE];
    char howm[BUF_SIZE];
    int price = 0;
    int howMany= 0;
    int result = 0;
    char result_c[BUF_SIZE];

    while(1)
    {
        read(c_sock,flag,BUF_SIZE);
        if(!strncmp(flag,"dutch",strlen("dutch")))
        {
            printf("!--DutchPay\n\n");

            read(c_sock,howm,2); //People
            howMany = atoi(howm);

            read(c_sock,totalprice,10); //Total Price
            price = atoi(totalprice);

            strcpy(msg,"people : ");
            strcat(msg,howm);

            strcat(msg,", totalprice : ");
            strcat(msg,totalprice);

            strcat(msg," WON, per person : ");
            result = price / howMany;   //calculate
            sprintf(result_c,"%d",result);
            strcat(msg,result_c);   //msg add result
            strcat(msg," WON\n");
            str_len = strlen(msg);
        }
        else if(!strncmp(flag,"transfer",strlen("transfer")))
        {
            memset(msg,0,sizeof(msg));
            read(c_sock,name_cnt,2);
            read(c_sock,filename,atoi(name_cnt));
            fp=fopen(filename,"wb");
            read(c_sock,filesize,5);

            int fileSize = atoi(filesize);
            read_cnt = read(c_sock,filebuf,fileSize);
            
            fwrite((void *)filebuf,1,read_cnt,fp);
            
            strcpy(msg,filesize);
            strcpy(msg,filename);
            strcat(msg,",stored.\n");
            strcat(msg,filebuf);

            str_len=strlen(msg);
            fclose(fp);
        }
        else if(!strncmp(flag,"download",strlen("download")))
        {
            int ifsize =0;
            char fsize[5];
            memset(msg,0,sizeof(msg));
            read(c_sock,name_cnt,2);
            read(c_sock,filename,atoi(name_cnt));
            
            fp = fopen(filename,"rb");
            
            fseek(fp,0,SEEK_END);
            ifsize=ftell(fp);
            fseek(fp,0,SEEK_SET);

            sprintf(fsize,"%d",ifsize);
            write(c_sock,fsize,5);

            if(fp!=NULL)    read_cnt = fread((void *)filebuf,1,ifsize,fp);

            usleep(5000000);
            strcpy(msg,filebuf);
            fclose(fp);
        }
        else
        {
            str_len = read(c_sock,msg,sizeof(msg));
            if(str_len==0) break;
        }
        
        send_msg(msg,str_len);
    }

    //remove disconnected client
    pthread_mutex_lock(&mutx);
    for(i=0;i<client_cnt;i++)
    {
        if(c_sock==client_socket[i])
        {
            while(i++<client_cnt-1)
                client_socket[i]=client_socket[i+1];
            break;
        }
    }
    client_cnt--;
    printf("(%4d-%02d-%02d %02d:%02d)\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min);
    printf("User(%d/%d)\n",client_cnt,MAX_CLIENT);
    pthread_mutex_unlock(&mutx);
    close(c_sock);
    return NULL;

}
void send_msg(char *msg,int len)
{
    int i;
    pthread_mutex_lock(&mutx);
    for(i=0;i<client_cnt;i++)   write(client_socket[i],msg,len);
    pthread_mutex_unlock(&mutx);
}
char* serverState(int count)
{
    char* stateMsg = malloc(sizeof(char)*NAME_SIZE);
    strcpy(stateMsg,"NONE");
    if(count<5) strcpy(stateMsg,"GOOD");
    else    strcpy(stateMsg,"BAD");
    return stateMsg;
}
void menu(char port[])
{
    system("clear");

    printf("***** chat server *****\n");
    printf("server port : %s\n",port);
    printf("server state : %s\n",serverState(client_cnt));
    printf("max connection : %d\n",MAX_CLIENT);

    printf("\n***** LOG *****\n");
}
void error_handler(char *msg)
{
    fputs(msg,stderr);
    fputs("\n",stderr);
    exit(1);
}
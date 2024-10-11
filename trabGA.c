#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define ECHOMAX 100

// Variáveis globais
int num_clients = 0;
double total_down_mb_tcp = 0;
double total_up_mb_tcp = 0;
double total_down_mb_udp = 0;
double total_up_mb_udp = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int periodicidade = 5;
char command[ECHOMAX];

// Mudar periodicidade
void *change_periodicidade(void *arg)
{
    int sock;
    struct sockaddr_in target;
    char **servIP = (char **)arg;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("ERRO na Criacao do Socket (Change Periodicidade)!\n");
        return NULL;
    }

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(6000);

    while (1)
    {
        // Espera o comando do terminal
        fgets(command, ECHOMAX, stdin);
        // Isola a variável periodicidade
        pthread_mutex_lock(&mutex);
        sscanf(command, "p: %d", &periodicidade);
        pthread_mutex_unlock(&mutex);

        // Enviar o comando para todos os servidores
        for (int i = 0; i < num_clients; i++)
        {
            target.sin_addr.s_addr = inet_addr(servIP[i]);
            sendto(sock, command, strlen(command), 0, (struct sockaddr *)&target, sizeof(target));
        }
    }

    close(sock);
    return NULL;
}

// Contar bytes recebidos TCP
void *execute_bpftrace_tcp_down(void *arg)
{
    FILE *fp;
    char output[64];
    unsigned long long total_down_bytes_tcp = 0;

    // Comando bpftrace para TCP
    char *command = "bpftrace -e 'kprobe:tcp_v4_rcv { printf(\"%llu\\n\", ((struct sk_buff *)arg0)->len); }'";

    // Abre o comando para leitura
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Falha ao executar o comando bpftrace (TCP DOWN).\n");
        pthread_exit(NULL);
    }

    int first_line = 1;

    while (fgets(output, sizeof(output) - 1, fp) != NULL)
    {
        // Ignorar a primeira linha ("attaching 1 probe")
        if (first_line)
        {
            first_line = 0;
            continue;
        }

        total_down_bytes_tcp += strtoull(output, NULL, 10); // Converte o output em número

        // Isolar a variável
        pthread_mutex_lock(&mutex);
        total_down_mb_tcp = total_down_bytes_tcp / 1048576.0; // Converte para MB (1024 x 1024)
        pthread_mutex_unlock(&mutex);
    }

    // Fecha o pipe
    pclose(fp);
    return NULL;
}

// Contar bytes enviados TCP
void *execute_bpftrace_tcp_up(void *arg)
{
    FILE *fp;
    char output[64];
    unsigned long long total_up_bytes_tcp = 0;

    // Comando bpftrace para TCP
    char *command = "bpftrace -e 'kprobe:tcp_sendmsg { printf(\"%llu\\n\", arg2); }'";

    // Abre o comando para leitura
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Falha ao executar o comando bpftrace (TCP UP).\n");
        pthread_exit(NULL);
    }

    int first_line = 1;

    while (fgets(output, sizeof(output) - 1, fp) != NULL)
    {
        // Ignorar a primeira linha ("attaching 1 probe")
        if (first_line)
        {
            first_line = 0;
            continue;
        }

        total_up_bytes_tcp += strtoull(output, NULL, 10); // Converte o output em número

        // Isolar a variável
        pthread_mutex_lock(&mutex);
        total_up_mb_tcp = total_up_bytes_tcp / 1048576.0; // Converte para MB (1024 x 1024)
        pthread_mutex_unlock(&mutex);
    }

    // Fecha o pipe
    pclose(fp);
    return NULL;
}

// Contar bytes recebidos UDP
void *execute_bpftrace_udp_down(void *arg)
{
    FILE *fp;
    char output[64];
    unsigned long long total_down_bytes_udp = 0;

    // Comando bpftrace para UDP
    char *command = "bpftrace -e 'kprobe:udp_rcv { printf(\"%llu\\n\", ((struct sk_buff *)arg0)->len); }'";

    // Abre o comando para leitura
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Falha ao executar o comando bpftrace (UDP).\n");
        pthread_exit(NULL);
    }

    int first_line = 1;

    while (fgets(output, sizeof(output) - 1, fp) != NULL)
    {
        // Ignorar a primeira linha ("attaching 1 probe")
        if (first_line)
        {
            first_line = 0;
            continue;
        }

        total_down_bytes_udp += strtoull(output, NULL, 10); // Converte o output em número

        // Isolar a variável
        pthread_mutex_lock(&mutex);
        total_down_mb_udp = total_down_bytes_udp / 1048576.0; // Converte para MB (1024 x 1024)
        pthread_mutex_unlock(&mutex);
    }

    // Fecha o pipe
    pclose(fp);
    return NULL;
}

// Contar bytes enviados UDP
void *execute_bpftrace_udp_up(void *arg)
{
    FILE *fp;
    char output[64];
    unsigned long long total_up_bytes_udp = 0;

    // Comando bpftrace para UDP
    char *command = "bpftrace -e 'kprobe:udp_send_skb { printf(\"%llu\\n\", ((struct sk_buff *)arg0)->len); }'";

    // Abre o comando para leitura
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Falha ao executar o comando bpftrace (UDP).\n");
        pthread_exit(NULL);
    }

    int first_line = 1;

    while (fgets(output, sizeof(output) - 1, fp) != NULL)
    {
        // Ignorar a primeira linha ("attaching 1 probe")
        if (first_line)
        {
            first_line = 0;
            continue;
        }

        total_up_bytes_udp += strtoull(output, NULL, 10); // Converte o output em número

        // Isolar a variável
        pthread_mutex_lock(&mutex);
        total_up_mb_udp = total_up_bytes_udp / 1048576.0; // Converte para MB (1024 x 1024)
        pthread_mutex_unlock(&mutex);
    }

    // Fecha o pipe
    pclose(fp);
    return NULL;
}

// Servidor
void *server_thread(void *arg)
{
    int sock;
    struct sockaddr_in me, from;
    // Alocação dinâmica do buffer
    char *buffer = (char *)malloc(ECHOMAX);
    socklen_t from_len = sizeof(from);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("ERRO na Criacao do Socket (Server)!\n");
        return NULL;
    }

    memset(&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(6000);

    if (bind(sock, (struct sockaddr *)&me, sizeof(me)) < 0)
    {
        printf("ERRO no Bind (Server)!\n");
        return NULL;
    }

    printf("Servidor esperando mensagens...\n");
    printf("Digite 'p: valor' para mudar a periodicidade\n");

    while (1)
    {
        if (sscanf(buffer, "p: %d", &periodicidade) == 1)
        {
            memset(buffer, 0, sizeof(buffer));
        }
        else
        {
            recvfrom(sock, buffer, ECHOMAX, 0, (struct sockaddr *)&from, &from_len);
            printf("Servidor recebeu de %s: %s\n", inet_ntoa(from.sin_addr), buffer);
        }
    };

    free(buffer);
    close(sock);
    return NULL;
}

// Cliente
void *client_thread(void *arg)
{
    int sock;
    struct sockaddr_in target;
    // Alocação dinâmica do buffer
    char *buffer = (char *)malloc(ECHOMAX);
    char **servIP = (char **)arg;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Socket Falhou (Client)!!!\n");
        return NULL;
    }

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(6000);

    while (1)
    {
        // Protege o acesso aos valores de TCP e UDP
        pthread_mutex_lock(&mutex);
        snprintf(buffer, ECHOMAX, "⬇ %.2f TCP MB, ⬇ %.2f UDP MB --- ⬆ %.2f TCP MB, ⬆ %.2f UDP MB", total_down_mb_tcp, total_down_mb_udp, total_up_mb_tcp, total_up_mb_udp);
        pthread_mutex_unlock(&mutex);

        for (int i = 0; i < num_clients; i++)
        {
            target.sin_addr.s_addr = inet_addr(servIP[i]);
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&target, sizeof(target));
        }

        // Pausa para não enviar de forma incessante
        sleep(periodicidade);
    };

    free(buffer);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <Server IP1> <Server IP2> ... <Server IPn>\n", argv[0]);
        exit(1);
    }

    num_clients = argc - 1;

    pthread_t server_tid, client_tid, tcpdown_tid, tcpup_tid, udpdown_tid, udpup_tid, period_tid;

    // Criar as thread para o servidor
    pthread_create(&server_tid, NULL, server_thread, NULL);
    pthread_create(&client_tid, NULL, client_thread, (void *)&argv[1]);
    pthread_create(&tcpdown_tid, NULL, execute_bpftrace_tcp_down, NULL);
    pthread_create(&tcpup_tid, NULL, execute_bpftrace_tcp_up, NULL);
    pthread_create(&udpdown_tid, NULL, execute_bpftrace_udp_down, NULL);
    pthread_create(&udpup_tid, NULL, execute_bpftrace_udp_up, NULL);
    pthread_create(&period_tid, NULL, change_periodicidade, (void *)&argv[1]);

    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);
    pthread_join(tcpdown_tid, NULL);
    pthread_join(tcpup_tid, NULL);
    pthread_join(udpdown_tid, NULL);
    pthread_join(udpup_tid, NULL);
    pthread_join(period_tid, NULL);

    return 0;
}

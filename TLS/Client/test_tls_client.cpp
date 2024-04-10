#include "make_tls_client.h"

// Our reader thread just reads the socket connection and prints it
void *readerThread(void *conn) {
    int exitReader = 0;

    while(!exitReader) {
        char buffer[128];
        int len = sslRead(conn, buffer, sizeof(buffer));

        if(len < 0) {
            perror("Error reading socket: ");
        }

        if(len > 0) {
            printf("\nReceived: %s\n", buffer);
        }

        exitReader = (len <= 0);
    }

    printf("Server closed connection. Exiting.\n");
    stopClient();
    EXIT_THREAD(conn);
}

void *writerThread(void *conn) {

    int exitWriter = 0;

    while(!exitWriter) {

        char buffer[128];
        printf("Input: ");
        fgets(buffer, 128, stdin);
        printf("\n");

        int len = sslWrite(conn, buffer, sizeof(buffer));

        if(len < 0) {
            perror("Error writing to server: ");
        }

        exitWriter = (len <= 0);
    }

    printf("writer: Server closed connection. Exiting.\n");
    stopClient();
    EXIT_THREAD(conn);
}

#define SERVER_NAME "192.168.80.162"
#define CA_CERT_FNAME "signing.pem"
#define PORT_NUM 5001
#define CLIENT_CERT_FNAME "laptop.crt"
#define CLIENT_KEY_FNAME "laptop.key"
#define SERVER_NAME_ON_CERT "alex.com"

int main() {

    createClient(SERVER_NAME, PORT_NUM, 1, CA_CERT_FNAME, SERVER_NAME_ON_CERT, 1, CLIENT_CERT_FNAME, CLIENT_KEY_FNAME, readerThread, writerThread);

    while(client_is_running());
}

#include "make_tls_client.h"
#include <signal.h>

#define MAX_FNAME_LEN   128
#define MAX_HOST_NAME_LEN   256


// Exit flag
static volatile int _exitFlag = 0;

// Server host name and port
static char _server_name[MAX_HOST_NAME_LEN];
static int _server_port = 0;

// Internal variables to store the certificate and key names, and whether or not to do client side verification, and whether to send
// our key and cert.

static int _verify_server = 0;
static char _ca_cert_fname[MAX_FNAME_LEN];

static int _send_cert = 0;
static char _client_cert_fname[MAX_FNAME_LEN];
static char _client_key_fname[MAX_FNAME_LEN];

// Store the server's name, as per its FQDN entry in its certificate
static char _server_name_on_cert[MAX_HOST_NAME_LEN];

// Variables to store the pointers to the reader and writer threads
static void *(*_reader_thread)(void *);
static void *(*_writer_thread)(void *);

// Function that is called when we exit
#define _closeAndExit(exitCode)\
	close(sockfd);\
	SSL_free(ssl);\
	SSL_CTX_free(ctx);\
	thread_cleanup();\
	cleanup_openssl()

void breakHandler(int dummy) {
    printf("\n\nEXITING BECAUSE YOU HIT CTRL-C. THANKS A LOT!\n");
    _exitFlag = 1;
}

void termHandler(int dummy) {
    printf("\n\nEXITING BECAUSE YOU TERMINATED ME! %%#*#%%**@!!!!!\n\n");
    _exitFlag = 1;
}

void killHandler(int dummy) {
    printf("\n\nEXITING BECAUSE YOU KILLED ME!\n\n");
    _exitFlag = 1;
}

// Our own internal client thread. 
void *_client_loop(void *dummy) {

	// Create a structure to store the server address
    SSL_CTX *ctx;
    SSL *ssl;
    int sockfd;
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	// Create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0)
	{
		perror("Cannot create socket: ");
		exit(-1);
	}

	// Get host IP address
	char hostIP[32];
	struct hostent *he;

	he = gethostbyname(_server_name);

	if(he == NULL)
	{
		herror("Unable to get host IP address: ");
        _closeAndExit(-1);
		exit(-1);
	}

	struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;

	strncpy(hostIP, inet_ntoa(*addr_list[0]), sizeof(hostIP));
	printf("Host %s IP address is %s\n", _server_name, hostIP);

	// Now form the server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(_server_port);
	inet_pton(AF_INET, hostIP, &serv_addr.sin_addr);

	// NEW: Initialize SSL
	init_openssl();

	// NEW: Create context
	ctx = create_context(_ca_cert_fname, _verify_server, 0);

    // Send our certificate if necessary
    if(_send_cert) {
        configure_context(ctx, _client_cert_fname, _client_key_fname);
    }

	// NEW: Enable multithreading in openSSL
	CRYPTO_thread_setup();

	// Now let's connect!
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error connecting: ");
		exit(-1);
	}

	ssl = connectSSL(ctx, sockfd, hostIP);
    
	if(ssl == NULL) {
        printf("Error establishing connection.");
        exit(-1);
    }

    if(_verify_server) {
        printf("CERTIFICATE DATA:\n");
        printCertificate(ssl);

        setHostVerification(ssl, _server_name_on_cert);
        if(!verifyCertificate(ssl))
        {
            printf("SSL Certicate validation failed.\n");
            _closeAndExit(-1);
        }
        else
            printf("SSL SERVER CERTIFICATE IS VALID\n");
    }


	pthread_t _kb, _rd;

    if(_writer_thread != NULL) {
        pthread_create(&_kb, NULL, _writer_thread, (void *) ssl);
        pthread_detach(_kb);
    }

    if(_reader_thread != NULL) {
        pthread_create(&_rd, NULL, _reader_thread, (void *) ssl);
        pthread_detach(_rd);
    }

	// Keep looping until we exit
	while(!_exitFlag);

	// Now join the threads
	printf("Closing socket and exiting.\n");
	_closeAndExit(0);
    return 0;
}
// Make a new TLS client.  This function spawns up to three threads: The main client thread, and the reader/writer threads if provided.
// It must be run together with some form of long-lasting loop to minimize the chances of producing orphan or zombie threads.
// serverName = Host name of server, or IP address
// serverPort = Port to connect to
// verifyServer = Set to 1 to verify server certificate
// caCertFname = Filename of CA certificate. Must be specified if verifyServer is true.
// serverNameOnCert = Name of server, as per its FQDN entry in its certificate. May not be the
//  same as serverName.
// sendCert = Set to 1 to send client certificate.  Needed if server does verification
// clientCertFname = Filename of client certificate. Must be specified if sendCert is true.
// clientPrivateKey = Filename of client private key. Must be specified if sendCert is true.
// readerThread = Pointer to reader thread, or NULL if none.
// writerThread = Pointer to writer thread, or NULL if none.

void createClient(const char *serverName, const int serverPort, int verifyServer, const char *caCertFname, const char *serverNameOnCert, int sendCert, const char *clientCertFname, const char *clientPrivateKey,
    void *(*readerThread)(void *), void *(*writerThread)(void *)) {

    strncpy(_server_name, serverName, MAX_HOST_NAME_LEN);
    _server_port = serverPort;

    _verify_server = verifyServer;
    
    if(_verify_server) {
        strncpy(_ca_cert_fname, caCertFname, MAX_FNAME_LEN);
        strncpy(_server_name_on_cert, serverNameOnCert, MAX_HOST_NAME_LEN);
    }
    
    _send_cert = sendCert;

    if(_send_cert) {
        strncpy(_client_cert_fname, clientCertFname, MAX_FNAME_LEN);
        strncpy(_client_key_fname,  clientPrivateKey, MAX_FNAME_LEN);                                                  
    }

    _reader_thread = readerThread;
    _writer_thread = writerThread;

    // Set up all the signals
    signal(SIGINT, breakHandler);
    signal(SIGTERM, termHandler);
    signal(SIGKILL, killHandler);

    // Launch threads
    pthread_t cl_thread;
    pthread_create(&cl_thread, NULL, _client_loop, NULL);
    pthread_detach(cl_thread);
}

// Call this function to check if the client loop is still running
int client_is_running() {
    return !_exitFlag;
}

// Call this function to kill the client loop
void stopClient() {
    _exitFlag = 1;
}

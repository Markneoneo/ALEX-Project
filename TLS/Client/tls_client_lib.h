#ifndef __TLS_CLIENT_LIB__
#define __TLS_CLIENT_LIB__
#include <openssl/ssl.h>
#include <openssl/err.h>
// Cert verification callback. Implement this in the client
int verify_callback(int preverify, X509_STORE_CTX *x509_ctx);

// Initialize OpenSSL by loading all error strings and algorithms.
void init_openssl();

// Clean up OpenSSL
void cleanup_openssl();

// Create an OpenSSL Context
// CACertName is the filename of the CA's certificate
SSL_CTX *create_context(const char *CACertName);

// Enables host verification
long setHostVerification(SSL *ssl, const char *hostname);

// Creates a new SSL session, attaches the client file
// descriptor to it, initiates the connection and
// returns the SSL session.
// ctx = SSL context
// fd = File descriptor for client connection.
// hostName = Common Name for host. Set to NULL to disable
// host name verification.
//

SSL *connectSSL(SSL_CTX *ctx, int fd, const char *hostName);

// This function prints out the server's certificate details of the SSL session.
// ssl = SSL session

void printCertificate(SSL *ssl);


// This function verifies the certificate. Returns TRUE if 
// the certificate is valid
int verifyCertificate(SSL *ssl);

#endif

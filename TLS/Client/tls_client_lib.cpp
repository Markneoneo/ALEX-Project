#include <string.h>
#include "tls_client_lib.h"
#include "tls_common_lib.h"

// Creates a new SSL session, attaches the client file
// descriptor to it, initiates the connection and
// returns the SSL session.
// ctx = SSL context
// fd = File descriptor for client connection.
// hostName = Common name for the server. Set to NULL to disable host verification.
//

SSL *connectSSL(SSL_CTX *ctx, int fd, const char *hostName)
{
	SSL *ssl = SSL_new(ctx);

/*	// Set host verification if hostName is not NULL
	if(hostName != NULL)
		setHostVerification(ssl, hostName); */

	SSL_set_fd(ssl, fd);
	if(SSL_connect(ssl) != 1)
	{
		ERR_print_errors_fp(stderr);
		SSL_free(ssl);
		return NULL;
	}

	return ssl;
}


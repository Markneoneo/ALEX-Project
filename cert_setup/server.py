# echo-server.py
'''
import socket

HOST = "192.168.88.162"  # Standard loopback interface address (localhost)
PORT = 12345  # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print(f"Connected by {addr}")
        while True:
            data = conn.recv(1024).decode('utf-8')
            if not data:
                break
            print("Received:", data)
'''

import socket
import ssl

HOST = '192.168.88.162'  # Listen on all network interfaces
PORT = 12345  # Port number for the server

def main():
    # Create a socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as sock:
        # Allow to reuse the same address
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, PORT))
        sock.listen(5)
        print(f"Server listening on {HOST}:{PORT}")

        # Wrap the socket with SSL
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        context.load_cert_chain(certfile='alex.crt', keyfile='alex.key')

        while True:
            # Accept new connections
            conn, addr = sock.accept()
            with conn:
                print(f"Connected by {addr}")

                # Wrap the connection with SSL
                connssl = context.wrap_socket(conn, server_side=True)

                try:
                    # Receive data and echo it back
                    while True:
                        data = connssl.recv(1024)
                        if not data:
                            break
                        print(f"Received data: {data.decode('utf-8')}")
                        connssl.sendall(data)
                except ssl.SSLError as e:
                    print(f"SSL error: {e}")
                except Exception as e:
                    print(f"Error: {e}")
                finally:
                    connssl.close()

if __name__ == '__main__':
    main()


import pygame
import socket
import ssl

pygame.init()
screen = pygame.display.set_mode((640, 480))
clock = pygame.time.Clock()

HOST = "192.168.88.162"  # The server's hostname or IP address
PORT = 12345  # The port used by the server

context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH)
context.load_cert_chain(certfile="laptop.crt", keyfile="laptop.key")
context.load_verify_locations(cafile='signing.pem')

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    wrapped_socket = context.wrap_socket(s, server_hostname=HOST)
    wrapped_socket.connect((HOST, PORT))
    
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN or event.type == pygame.KEYUP:
                key_name = pygame.key.name(event.key)
                if key_name == 'q':
                    running = False
                key_state = "Pressed" if event.type == pygame.KEYDOWN else "Released"
                message = f"{key_name}:{key_state}"
                wrapped_socket.sendall(message.encode('utf-8'))
            if event.type == pygame.QUIT:
                running = False

        clock.tick(60)

pygame.quit()

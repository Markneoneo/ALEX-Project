import pygame
import time
import socket  # Import the socket library

# Initialize Pygame
pygame.init()

# Set screen dimensions
SCREEN_WIDTH = 200
SCREEN_HEIGHT = 100

# Set colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Set keyboard mappings
KEY_MAP = {
    pygame.K_w: 'f',
    pygame.K_a: 'l',
    pygame.K_s: 'b',
    pygame.K_d: 'r'
}

# Initialize a TCP socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the Raspberry Pi
# Replace 'RASPBERRY_PI_IP' with the Raspberry Pi's IP address
# and 'PORT' with the port number you are listening on the Raspberry Pi

client_socket.connect(('192.168.88.162', 12345))

def get_input():
    keys = pygame.key.get_pressed()
    for key, state in KEY_MAP.items():
        if keys[key]:
            return state
    return 's'

def main():
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption('Key Press Detection')

    last_state = None
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        
        data = client_socket.recv(1).decode('utf-8')
        if data == 'c':
            # Get the current state
            current_state = get_input()
            #if current_state != last_state:
            # Send the current state to the Raspberry Pi
            client_socket.send(current_state.encode('utf-8'))
            #last_state = current_state

            

        # Update the display
        pygame.display.flip()

        # Limit frames per second
        time.sleep(0.1)

    pygame.quit()
    client_socket.close()  # Close the socket when done

if __name__ == '__main__':
    main()

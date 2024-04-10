import pygame
import time

# Initialize Pygame
pygame.init()

# Set screen dimensions
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600

# Set colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Set keyboard mappings
KEY_MAP = {
    pygame.K_w: 'forward',
    pygame.K_a: 'left',
    pygame.K_s: 'backward',
    pygame.K_d: 'right'
}

def get_input():
    keys = pygame.key.get_pressed()
    for key, state in KEY_MAP.items():
        if keys[key]:
            return state
    return 'STOP'

def draw_ui(screen, state_log):
    font = pygame.font.SysFont(None, 24)
    y_offset = 20
    for i, log_entry in enumerate(state_log):
        text = font.render(log_entry, True, WHITE)
        screen.blit(text, (20, y_offset + i * 20))

def main():
    # Set up the display
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
    pygame.display.set_caption("Keyboard Input Example")

    clock = pygame.time.Clock()

    state = 'STOP'
    state_log = []

    running = True
    while running:
        current_state = get_input()

        if current_state != state:
            state = current_state
            state_log.append(f"State changed to: {state}")

        if len(state_log) > 25:  # Limit log size
            state_log.pop(0)  # Remove oldest entry

        # Handle quit event
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        screen.fill(BLACK)
        draw_ui(screen, state_log)
        pygame.display.flip()

        clock.tick(60)

    pygame.quit()

if __name__ == "__main__":
    main()

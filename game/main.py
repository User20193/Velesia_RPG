import sys
import os

# We need to make sure the compiled 'engine' module can be found.
# When run from the game/ folder, it should be in the same folder.
# If someone runs from root, we append the game/ folder to sys.path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    import engine
except ImportError as e:
    print(f"Failed to import engine module: {e}")
    print("Please make sure you have compiled the C++ code successfully and the .pyd/.so file is in the 'game' folder.")
    sys.exit(1)

def main():
    # 1. Initialize the Engine
    # Create an 800x600 window with a title
    eng = engine.Engine(800, 600, "My 2D RPG (Python + C++)")

    # 2. Setup Texture Manager
    tex_mgr = eng.get_texture_manager()

    # To load a texture later, you can do:
    # tex_mgr.load_texture("player", "assets/player.png")
    # For now, we will use draw_rectangle to represent the player

    # 3. Setup Camera
    camera = engine.GameCamera()
    camera.set_offset(400.0, 300.0) # Center of the 800x600 screen
    camera.set_zoom(1.0)

    # Player state
    player_x = 400.0
    player_y = 300.0
    player_size = 32
    player_speed = 300.0 # Pixels per second

    print("Engine started successfully!")
    print("Use W, A, S, D to move. Move the mouse to aim, Click Left Mouse Button to shoot/attack.")

    # 3. Main Game Loop
    while eng.is_running():
        # --- UPDATE ---
        dt = eng.get_delta_time()

        # Movement logic (Top-Down)
        if eng.is_key_down(engine.Keys.KEY_W) or eng.is_key_down(engine.Keys.KEY_UP):
            player_y -= player_speed * dt
        if eng.is_key_down(engine.Keys.KEY_S) or eng.is_key_down(engine.Keys.KEY_DOWN):
            player_y += player_speed * dt
        if eng.is_key_down(engine.Keys.KEY_A) or eng.is_key_down(engine.Keys.KEY_LEFT):
            player_x -= player_speed * dt
        if eng.is_key_down(engine.Keys.KEY_D) or eng.is_key_down(engine.Keys.KEY_RIGHT):
            player_x += player_speed * dt

        # Update camera to follow player (center of the player)
        camera.set_target(player_x + player_size / 2.0, player_y + player_size / 2.0)

        # Get mouse position in the world
        mouse_screen_x = eng.get_mouse_x()
        mouse_screen_y = eng.get_mouse_y()
        mouse_world_x, mouse_world_y = camera.get_screen_to_world(mouse_screen_x, mouse_screen_y)

        # Attack logic
        is_attacking = eng.is_mouse_button_down(engine.MouseButtons.MOUSE_BUTTON_LEFT)

        # --- DRAW ---
        eng.begin_drawing()

        # Draw background (a dark green color like grass)
        eng.clear_background(34, 139, 34)

        # Start drawing in the camera's perspective
        camera.begin_mode()

        # 1. Draw some world objects to see camera moving
        # Let's draw some dark green trees/bushes
        eng.draw_rectangle(200, 200, 64, 64, 0, 100, 0)
        eng.draw_rectangle(600, 150, 64, 64, 0, 100, 0)
        eng.draw_rectangle(300, 500, 64, 64, 0, 100, 0)

        # 2. Draw the player (Red square)
        eng.draw_rectangle(int(player_x), int(player_y), player_size, player_size, 255, 0, 0)

        # 3. Draw a line from player center to mouse position (Aiming line)
        if is_attacking:
            # If attacking, draw a thick yellow "laser" or "sword swing" line
            eng.draw_line(int(player_x + player_size/2), int(player_y + player_size/2),
                          int(mouse_world_x), int(mouse_world_y), 255, 255, 0)
        else:
            # If just aiming, draw a faint white line
            eng.draw_line(int(player_x + player_size/2), int(player_y + player_size/2),
                          int(mouse_world_x), int(mouse_world_y), 255, 255, 255, 100)

        # Stop drawing in camera perspective
        camera.end_mode()

        # You can draw UI here (it won't move with the camera)
        # For example, health bar at top-left corner would go here

        eng.end_drawing()

if __name__ == "__main__":
    main()

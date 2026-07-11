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

    # Player state
    player_x = 400.0
    player_y = 300.0
    player_speed = 300.0 # Pixels per second

    print("Engine started successfully!")
    print("Use W, A, S, D or Arrow Keys to move.")

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

        # --- DRAW ---
        eng.begin_drawing()

        # Draw background (a dark green color like grass)
        eng.clear_background(34, 139, 34)

        # Draw the player (Red square, 32x32 pixels)
        # We cast to int because draw_rectangle expects integers
        eng.draw_rectangle(int(player_x), int(player_y), 32, 32, 255, 0, 0)

        # If you had a texture loaded, you would draw it like this:
        # tex_mgr.draw_texture("player", player_x, player_y)

        eng.end_drawing()

if __name__ == "__main__":
    main()

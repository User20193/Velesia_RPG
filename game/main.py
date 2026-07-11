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

    # 4. Setup ECS (Scene)
    scene = eng.get_scene()

    # --- Create Player Entity ---
    player_entity = scene.create_entity()
    scene.add_transform(player_entity, 400.0, 300.0)
    scene.add_collider(player_entity, 32.0, 32.0)
    player_speed = 300.0 # Pixels per second

    # Animation state (Python side for now, can be moved to C++ component later)
    current_frame = 0
    frame_timer = 0.0
    frame_delay = 0.15 # seconds per frame

    # --- Create Obstacles Entities ---
    obstacle_data = [
        (200.0, 200.0, 64.0, 64.0),
        (600.0, 150.0, 64.0, 64.0),
        (300.0, 500.0, 64.0, 64.0)
    ]
    obstacles = []
    for ox, oy, ow, oh in obstacle_data:
        obs_ent = scene.get_pooled_entity()
        scene.add_transform(obs_ent, ox, oy)
        scene.add_collider(obs_ent, ow, oh)
        obstacles.append(obs_ent)

    # Debug state
    debug_mode = False

    print("Engine started successfully!")
    print("Use W, A, S, D to move. Move the mouse to aim, Click Left Mouse Button to shoot/attack.")
    print("Press F3 to toggle Debug Mode.")

    # 3. Main Game Loop
    while eng.is_running():
        # --- UPDATE ---
        dt = eng.get_delta_time()

        # Toggle Debug
        if eng.is_key_pressed(engine.Keys.KEY_F3):
            debug_mode = not debug_mode

        # Get Player Transform
        px, py = scene.get_transform(player_entity)

        # Save previous position for collision response
        prev_x = px
        prev_y = py

        is_moving = False
        # Movement logic (Top-Down)
        if eng.is_key_down(engine.Keys.KEY_W) or eng.is_key_down(engine.Keys.KEY_UP):
            py -= player_speed * dt
            is_moving = True
        if eng.is_key_down(engine.Keys.KEY_S) or eng.is_key_down(engine.Keys.KEY_DOWN):
            py += player_speed * dt
            is_moving = True
        if eng.is_key_down(engine.Keys.KEY_A) or eng.is_key_down(engine.Keys.KEY_LEFT):
            px -= player_speed * dt
            is_moving = True
        if eng.is_key_down(engine.Keys.KEY_D) or eng.is_key_down(engine.Keys.KEY_RIGHT):
            px += player_speed * dt
            is_moving = True

        # Collision Check (ECS approach)
        for obs_ent in obstacles:
            if scene.has_collider(obs_ent):
                ox, oy = scene.get_transform(obs_ent)
                # Hardcoded sizes for now, ideally retrieved from a Collider component getter
                ow, oh = 64.0, 64.0
                player_size = 32.0

                # If player collides with an obstacle
                if eng.check_collision_recs(px, py, player_size, player_size, ox, oy, ow, oh):
                    # Simple collision response: snap back to previous position
                    px = prev_x
                    py = prev_y

        # Update Player Transform in ECS
        scene.set_transform(player_entity, px, py)

        # Animation logic (only animate when moving)
        if is_moving:
            frame_timer += dt
            if frame_timer >= frame_delay:
                frame_timer = 0.0
                current_frame = (current_frame + 1) % 4
        else:
            current_frame = 0 # Idle frame
            frame_timer = 0.0

        # Update camera to follow player (center of the player)
        camera.set_target(px + player_size / 2.0, py + player_size / 2.0)

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

        # 1. Draw some world objects (obstacles) from ECS
        for obs_ent in obstacles:
            ox, oy = scene.get_transform(obs_ent)
            ow, oh = 64.0, 64.0
            eng.draw_rectangle(int(ox), int(oy), int(ow), int(oh), 0, 100, 0)
            if debug_mode:
                eng.draw_rectangle_lines(int(ox), int(oy), int(ow), int(oh), 255, 0, 0) # Red hitbox

        # 2. Draw the player from ECS
        # Here we simulate an animation.
        # If we had a loaded texture:
        # tex_mgr.draw_texture_rec("player", current_frame * player_size, 0, player_size, player_size, px, py)
        # But for now, we'll just draw a solid color that slightly changes color based on the frame to prove it works
        anim_color = 200 + (current_frame * 15)
        eng.draw_rectangle(int(px), int(py), int(player_size), int(player_size), anim_color, 0, 0)

        if debug_mode:
            eng.draw_rectangle_lines(int(px), int(py), int(player_size), int(player_size), 255, 255, 0) # Yellow hitbox for player

        # 3. Draw a line from player center to mouse position (Aiming line)
        if is_attacking:
            eng.draw_line(int(px + player_size/2), int(py + player_size/2),
                          int(mouse_world_x), int(mouse_world_y), 255, 255, 0)
        else:
            eng.draw_line(int(px + player_size/2), int(py + player_size/2),
                          int(mouse_world_x), int(mouse_world_y), 255, 255, 255, 100)

        # Stop drawing in camera perspective
        camera.end_mode()

        # --- DRAW UI ---
        # UI is drawn AFTER end_mode(), so it stays on the screen

        if debug_mode:
            fps = eng.get_fps()
            eng.draw_rectangle(10, 10, 200, 70, 0, 0, 0, 150) # Dark semi-transparent background
            eng.draw_text(f"FPS: {fps}", 20, 20, 20, 0, 255, 0)
            eng.draw_text(f"Player: {int(px)}, {int(py)}", 20, 45, 20, 255, 255, 255)

        eng.end_drawing()

if __name__ == "__main__":
    main()

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
    print("========================================")
    print("   Initializing RPG Engine...   ")
    print("========================================")

    # --- Pre-generate required procedural assets ---
    assets_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
    if not os.path.exists(assets_dir):
        os.makedirs(assets_dir)

    generator = engine.TextureGenerator()
    generator.set_seed(12345)

    grass_path = os.path.join(assets_dir, "grass_bg.png")
    stone_path = os.path.join(assets_dir, "stone_obs.png")
    wood_path = os.path.join(assets_dir, "wood_obs.png")

    if not os.path.exists(grass_path):
        print("Generating grass background texture...")
        generator.generate_grass(grass_path, 256, 256)

    if not os.path.exists(stone_path):
        print("Generating stone obstacle texture...")
        generator.generate_stone(stone_path, 64, 64)

    if not os.path.exists(wood_path):
        print("Generating wood obstacle texture...")
        generator.generate_wood(wood_path, 32, 32)

    # 1. Initialize the Engine
    # Create an 800x600 window with a title
    eng = engine.Engine(800, 600, "My 2D RPG (Python + C++)")

    # 2. Setup Texture Manager
    tex_mgr = eng.get_texture_manager()
    tex_mgr.load_texture("bg_grass", grass_path)
    tex_mgr.load_texture("obs_stone", stone_path)
    tex_mgr.load_texture("obs_wood", wood_path)

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
    player_size = 32.0

    # Animation state (Python side for now, can be moved to C++ component later)
    current_frame = 0
    frame_timer = 0.0
    frame_delay = 0.15 # seconds per frame

    import random

    # --- Create Obstacles Entities (Massive World for Chunk Testing) ---
    print("Generating world...")
    for _ in range(500):
        obs_ent = scene.get_pooled_entity()
        ox = random.uniform(-2000.0, 2000.0)
        oy = random.uniform(-2000.0, 2000.0)
        # We'll use width to determine texture later
        ow = random.choice([32.0, 64.0])
        oh = ow # keep them square for simplicity
        scene.add_transform(obs_ent, ox, oy)
        scene.add_collider(obs_ent, ow, oh)

    # --- Create Enemies ---
    for _ in range(50):
        enemy_ent = scene.get_pooled_entity()
        ex = random.uniform(-1000.0, 1000.0)
        ey = random.uniform(-1000.0, 1000.0)
        scene.add_transform(enemy_ent, ex, ey)
        scene.add_collider(enemy_ent, 32.0, 32.0)
        scene.add_enemy(enemy_ent, 50.0, 150.0, 10.0) # hp, speed, damage

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

        # 1. Update Chunking System
        # Mark only entities within 1000 pixels of player as 'Active'
        scene.update_active_chunks(px, py, 1000.0)
        active_entities = scene.get_active_entities()

        # 2. Process Enemies (AI)
        # Enemies only move if they are in an active chunk
        for ent in active_entities:
            if scene.has_enemy(ent):
                ex, ey = scene.get_transform(ent)

                # Simple AI: Move towards player
                dx = px - ex
                dy = py - ey
                dist = (dx**2 + dy**2)**0.5

                if dist > 0 and dist < 500: # Aggro radius
                    enemy_speed = 100.0 * dt
                    ex += (dx / dist) * enemy_speed
                    ey += (dy / dist) * enemy_speed
                    scene.set_transform(ent, ex, ey)

        # 3. Collision Check (Only against ACTIVE entities with colliders)
        for ent in active_entities:
            if ent != player_entity and scene.has_collider(ent) and not scene.has_enemy(ent):
                ox, oy = scene.get_transform(ent)
                ow, oh = scene.get_collider(ent)

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

        eng.clear_background(0, 0, 0)

        # Start drawing in the camera's perspective
        camera.begin_mode()

        # Draw infinite procedural background by tiling the 256x256 grass texture
        # We calculate the starting tile based on the camera position to ensure it covers the screen
        cam_x = px - 400
        cam_y = py - 300
        start_tx = int(cam_x // 256) * 256
        start_ty = int(cam_y // 256) * 256

        # Draw a 4x4 grid of tiles to cover the 800x600 screen + scrolling margin
        for ty in range(start_ty - 256, start_ty + 600 + 256, 256):
            for tx in range(start_tx - 256, start_tx + 800 + 256, 256):
                tex_mgr.draw_texture("bg_grass", tx, ty)

        # 1. Draw WORLD entities (Only active ones!)
        for ent in active_entities:
            if ent == player_entity:
                continue # Draw player last

            if scene.has_enemy(ent):
                ex, ey = scene.get_transform(ent)
                ew, eh = scene.get_collider(ent)
                eng.draw_rectangle(int(ex), int(ey), int(ew), int(eh), 255, 0, 255) # Magenta enemies
                if debug_mode:
                    eng.draw_rectangle_lines(int(ex), int(ey), int(ew), int(eh), 255, 0, 0)
            elif scene.has_collider(ent):
                ox, oy = scene.get_transform(ent)
                ow, oh = scene.get_collider(ent)

                # If it's a 64x64 obstacle, draw stone, if 32x32 draw wood
                if ow >= 64.0:
                    tex_mgr.draw_texture("obs_stone", int(ox), int(oy))
                else:
                    tex_mgr.draw_texture("obs_wood", int(ox), int(oy))

                if debug_mode:
                    eng.draw_rectangle_lines(int(ox), int(oy), int(ow), int(oh), 255, 0, 0)

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

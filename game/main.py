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

    # --- Assets Directory ---
    assets_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
    if not os.path.exists(assets_dir):
        os.makedirs(assets_dir)

    # Texture generator disabled as per user request
    # generator = engine.TextureGenerator()
    # generator.set_seed(12345)

    grass_path = os.path.join(assets_dir, "grass_bg.png")
    stone_path = os.path.join(assets_dir, "stone_obs.png")
    wood_path = os.path.join(assets_dir, "wood_obs.png")

    # if not os.path.exists(grass_path):
    #     print("Generating grass background texture...")
    #     generator.generate_grass(grass_path, 256, 256)

    # if not os.path.exists(stone_path):
    #     print("Generating stone obstacle texture...")
    #     generator.generate_stone(stone_path, 64, 64)

    # if not os.path.exists(wood_path):
    #     print("Generating wood obstacle texture...")
    #     generator.generate_wood(wood_path, 32, 32)

    # 1. Initialize the Engine
    # Create an 800x600 window with a title
    eng = engine.Engine(800, 600, "Velesia RPG")

    # 2. Setup Texture Manager
    tex_mgr = eng.get_texture_manager()
    tex_mgr.load_texture("bg_grass", grass_path)
    tex_mgr.load_texture("obs_stone", stone_path)
    tex_mgr.load_texture("obs_wood", wood_path)
    tex_mgr.load_texture("menu_bg", "game/assets/menu_bg.jpg")

    eng.load_font("title", "game/assets/title_font.ttf", 64)
    eng.load_font("button", "game/assets/button_font.ttf", 32)
    eng.load_font("button_hover", "game/assets/button_font.ttf", 36)

    # 3. Setup Camera
    camera = engine.GameCamera()
    camera.set_offset(400.0, 300.0) # Center of the 800x600 screen
    camera.set_zoom(1.0)

    # 4. Setup ECS (Scene)
    scene = eng.get_scene()

    # --- Game State ---
    class GameState:
        MENU = 0
        PLAYING = 1
        SETTINGS = 2
        SAVES = 3
        GAME_OVER = 4

    current_state = GameState.MENU

    # Button helper
    def draw_button(text, x, y, width, height, is_hovered):
        if is_hovered:
            eng.draw_rectangle(int(x), int(y), int(width), int(height), 80, 80, 80, 200)
            text_width = eng.measure_text_ex("button_hover", text, 36, 1.0)
            text_x = x + (width - text_width) / 2
            eng.draw_text_ex("button_hover", text, text_x, y + (height - 36) / 2, 36, 1.0, 255, 255, 255)
        else:
            eng.draw_rectangle(int(x), int(y), int(width), int(height), 50, 50, 50, 200)
            eng.draw_rectangle_lines(int(x), int(y), int(width), int(height), 100, 100, 100)
            text_width = eng.measure_text_ex("button", text, 32, 1.0)
            text_x = x + (width - text_width) / 2
            eng.draw_text_ex("button", text, text_x, y + (height - 32) / 2, 32, 1.0, 200, 200, 200)

    # --- Create Player Entity ---
    player_entity = scene.create_entity()

    player_speed = 300.0 # Pixels per second
    player_size = 32.0
    player_max_hp = 100.0
    player_damage = 25.0

    # Global variables to reset
    player_hp = 100.0
    player_attack_cooldown = 0.0
    current_frame = 0
    frame_timer = 0.0
    frame_delay = 0.15 # seconds per frame

    import random

    world_entities = []

    def reset_game():
        nonlocal player_hp, player_attack_cooldown
        player_hp = player_max_hp
        player_attack_cooldown = 0.0

        # Reset Player
        scene.add_transform(player_entity, 400.0, 300.0)
        scene.add_collider(player_entity, player_size, player_size)

        # Clear old entities
        for ent in world_entities:
            scene.return_pooled_entity(ent)
        world_entities.clear()

        # --- Create Obstacles Entities ---
        for _ in range(500):
            obs_ent = scene.get_pooled_entity()
            ox = random.uniform(-2000.0, 2000.0)
            oy = random.uniform(-2000.0, 2000.0)
            ow = random.choice([32.0, 64.0])
            oh = ow
            scene.add_transform(obs_ent, ox, oy)
            scene.add_collider(obs_ent, ow, oh)
            world_entities.append(obs_ent)

        # --- Create Enemies ---
        for _ in range(50):
            enemy_ent = scene.get_pooled_entity()
            ex = random.uniform(-1000.0, 1000.0)
            ey = random.uniform(-1000.0, 1000.0)
            scene.add_transform(enemy_ent, ex, ey)
            scene.add_collider(enemy_ent, 32.0, 32.0)
            scene.add_enemy(enemy_ent, 50.0, 150.0, 10.0)
            world_entities.append(enemy_ent)

    # Initialize first game
    reset_game()

    # Debug state
    debug_mode = False

    print("Engine started successfully!")
    print("Use W, A, S, D to move. Move the mouse to aim, Click Left Mouse Button to shoot/attack.")
    print("Press F3 to toggle Debug Mode.")

    # 3. Main Game Loop
    while eng.is_running():
        # --- UPDATE ---
        dt = eng.get_delta_time()

        mouse_screen_x = eng.get_mouse_x()
        mouse_screen_y = eng.get_mouse_y()
        is_clicked = eng.is_mouse_button_pressed(engine.MouseButtons.MOUSE_BUTTON_LEFT)

        # ==========================================
        #                 MAIN MENU
        # ==========================================
        if current_state == GameState.MENU:
            eng.begin_drawing()
            eng.clear_background(0, 0, 0)

            # Draw Background
            tex_mgr.draw_texture("menu_bg", 0, 0)

            # Draw Title
            title_text = "Velesia RPG"
            title_width = eng.measure_text_ex("title", title_text, 64, 2.0)
            eng.draw_text_ex("title", title_text, (800 - title_width) / 2, 100, 64, 2.0, 220, 20, 20)

            # Buttons
            btn_w, btn_h = 250, 50
            btn_x = (800 - btn_w) / 2

            buttons = [
                ("Играть", btn_x, 250, GameState.PLAYING),
                ("Настройки", btn_x, 320, GameState.SETTINGS),
                ("Сохранения", btn_x, 390, GameState.SAVES),
                ("Выход", btn_x, 460, -1)
            ]

            for text, bx, by, action in buttons:
                hover = eng.check_collision_recs(mouse_screen_x, mouse_screen_y, 1, 1, bx, by, btn_w, btn_h)
                draw_button(text, bx, by, btn_w, btn_h, hover)

                if hover and is_clicked:
                    if action == -1:
                        return # Exit the game
                    elif action == GameState.PLAYING:
                        reset_game()
                        current_state = GameState.PLAYING
                    else:
                        current_state = action

            eng.end_drawing()
            continue

        # ==========================================
        #             SETTINGS & SAVES
        # ==========================================
        if current_state == GameState.SETTINGS or current_state == GameState.SAVES:
            eng.begin_drawing()
            eng.clear_background(30, 30, 30)

            title = "Настройки" if current_state == GameState.SETTINGS else "Сохранения"
            title_width = eng.measure_text_ex("title", title, 48, 2.0)
            eng.draw_text_ex("title", title, (800 - title_width) / 2, 100, 48, 2.0, 255, 255, 255)

            # Back Button
            btn_w, btn_h = 200, 50
            bx, by = (800 - btn_w) / 2, 450
            hover = eng.check_collision_recs(mouse_screen_x, mouse_screen_y, 1, 1, bx, by, btn_w, btn_h)
            draw_button("Назад", bx, by, btn_w, btn_h, hover)

            if hover and is_clicked:
                current_state = GameState.MENU

            eng.end_drawing()
            continue

        # ==========================================
        #                 GAME OVER
        # ==========================================
        if current_state == GameState.GAME_OVER:
            eng.begin_drawing()
            eng.clear_background(20, 0, 0)

            title = "ВЫ ПОГИБЛИ"
            title_width = eng.measure_text_ex("title", title, 64, 2.0)
            eng.draw_text_ex("title", title, (800 - title_width) / 2, 200, 64, 2.0, 255, 50, 50)

            # Main Menu Button
            btn_w, btn_h = 300, 50
            bx, by = (800 - btn_w) / 2, 350
            hover = eng.check_collision_recs(mouse_screen_x, mouse_screen_y, 1, 1, bx, by, btn_w, btn_h)
            draw_button("В Главное Меню", bx, by, btn_w, btn_h, hover)

            if hover and is_clicked:
                current_state = GameState.MENU

            eng.end_drawing()
            continue

        # ==========================================
        #                 PLAYING
        # ==========================================

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
        if player_attack_cooldown > 0:
            player_attack_cooldown -= dt

        # Combat (Damage enemies)
        if is_attacking and player_attack_cooldown <= 0.0:
            player_attack_cooldown = 0.3 # Attack speed
            # Check all active enemies
            for ent in active_entities:
                if scene.has_enemy(ent):
                    ex, ey = scene.get_transform(ent)
                    ew, eh = scene.get_collider(ent)

                    # Check if mouse is hovering over the enemy
                    if (mouse_world_x >= ex and mouse_world_x <= ex + ew and
                        mouse_world_y >= ey and mouse_world_y <= ey + eh):

                        # Distance check (melee range)
                        dist_to_mouse = ((px + player_size/2 - mouse_world_x)**2 + (py + player_size/2 - mouse_world_y)**2)**0.5
                        if dist_to_mouse < 100.0:
                            hp = scene.get_enemy_hp(ent)
                            hp -= player_damage
                            if hp <= 0:
                                scene.return_pooled_entity(ent)
                            else:
                                scene.set_enemy_hp(ent, hp)

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

                # Draw Health Bar
                hp = scene.get_enemy_hp(ent)
                max_hp = scene.get_enemy_max_hp(ent)
                hp_perc = max(0.0, hp / max_hp)

                # Background red bar
                eng.draw_rectangle(int(ex), int(ey - 10), int(ew), 6, 200, 0, 0)
                # Foreground green bar
                eng.draw_rectangle(int(ex), int(ey - 10), int(ew * hp_perc), 6, 0, 200, 0)

                if debug_mode:
                    eng.draw_rectangle_lines(int(ex), int(ey), int(ew), int(eh), 255, 0, 0)

                # Enemy damaging player logic
                if eng.check_collision_recs(px, py, player_size, player_size, ex, ey, ew, eh):
                    player_hp -= 10.0 * dt # DPS
                    if player_hp <= 0:
                        player_hp = 0
                        current_state = GameState.GAME_OVER

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

        # Player Health Bar UI
        eng.draw_rectangle(20, 20, 200, 20, 100, 100, 100) # BG
        hp_perc = player_hp / player_max_hp
        eng.draw_rectangle(20, 20, int(200 * hp_perc), 20, 255, 50, 50) # FG
        eng.draw_text_ex("button", f"HP: {int(player_hp)} / {int(player_max_hp)}", 25, 22, 16, 1.0, 255, 255, 255)

        if debug_mode:
            fps = eng.get_fps()
            eng.draw_rectangle(10, 50, 200, 70, 0, 0, 0, 150) # Dark semi-transparent background
            eng.draw_text(f"FPS: {fps}", 20, 60, 20, 0, 255, 0)
            eng.draw_text(f"Player: {int(px)}, {int(py)}", 20, 85, 20, 255, 255, 255)

        eng.end_drawing()

if __name__ == "__main__":
    main()

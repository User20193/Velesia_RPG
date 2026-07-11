import engine
import time

def main():
    eng = engine.Engine(800, 600, "Test CCL")
    tm = eng.get_texture_manager()

    # Trigger AssetManager to use AutoDetectLoader
    tm.load_sprite_sheet("test_sheet", "assets/test_spritesheet.png")

    # Render loop just to show it doesn't crash
    frames = 0
    while eng.is_running() and frames < 10:
        eng.begin_drawing()
        eng.clear_background(0,0,0)
        tm.draw_sprite_frame("test_sheet", frames % 4, 100.0, 100.0)
        eng.end_drawing()
        frames += 1

if __name__ == "__main__":
    main()

import sys
import os

# Ensure the engine module can be found
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    import engine
except ImportError as e:
    print(f"Failed to import engine module: {e}")
    sys.exit(1)

def main():
    print("========================================")
    print("   Procedural Texture Generator Tool   ")
    print("========================================")

    # Initialize the generator
    generator = engine.TextureGenerator()

    # You can change the seed to get completely different textures!
    generator.set_seed(4242)

    # Ensure assets directory exists
    assets_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
    if not os.path.exists(assets_dir):
        os.makedirs(assets_dir)

    print("Generating Grass Texture (256x256)...")
    if generator.generate_grass(os.path.join(assets_dir, "proc_grass.png"), 256, 256):
        print(" -> Success! Saved to assets/proc_grass.png")
    else:
        print(" -> Failed.")

    print("Generating Water Texture (256x256)...")
    if generator.generate_water(os.path.join(assets_dir, "proc_water.png"), 256, 256):
        print(" -> Success! Saved to assets/proc_water.png")
    else:
        print(" -> Failed.")

    print("Generating Dirt Texture (256x256)...")
    if generator.generate_dirt(os.path.join(assets_dir, "proc_dirt.png"), 256, 256):
        print(" -> Success! Saved to assets/proc_dirt.png")
    else:
        print(" -> Failed.")

    print("\nGeneration complete. You can now use these seamless textures in your game!")

if __name__ == "__main__":
    main()

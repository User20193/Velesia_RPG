from PIL import Image, ImageDraw
img = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)

# Draw frame 1 at (0, 0)
draw.rectangle([0, 0, 31, 31], fill=(255, 0, 0, 255))
# Draw frame 2 at (33, 0) -> leave 1 pixel gap!
draw.rectangle([33, 0, 64, 31], fill=(0, 255, 0, 255))
# Draw frame 3 at (0, 33) -> leave 1 pixel gap!
draw.rectangle([0, 33, 31, 64], fill=(0, 0, 255, 255))

img.save('game/assets/test_spritesheet_gap.png')

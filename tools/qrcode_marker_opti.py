import io
from PIL import Image
import segno

best = (0, None, {})

for i in range(4):
    for n in range(99999):
        qrcode = segno.make(n, mask=i)

        out = io.BytesIO()
        qrcode.save(out, scale=1, kind='png')
        out.seek(0)  # Important to let PIL / Pillow load the image
        img = Image.open(out)

        score = 0

        for x in range(img.width):
            for y in range(img.height):
                pixel = img.getpixel((x, y))
                score += pixel/255
        if score <= best[0] or best[1] is None:
            best = (score, img, {'n': n, 'mask': i})

score, qrcode, option = best
print(score, option)
qrcode.save('test.png')
import random
import os
from pathlib import Path
from wand.image import Image
from wand.color import Color
from wand.drawing import Drawing

def add_defects_to_pdf(input_pdf_path, output_pdf_path, num_spots):
    with Image(filename=input_pdf_path, resolution=300) as img:
        # Appliquer une transformation aléatoire
        apply_random_transformations(img)

        # Ajouter du flou
        # img.blur(radius=0, sigma=1)

        # # Ajouter des taches
        with Drawing() as draw:
            draw.fill_color = Color('black')
            for _ in range(num_spots):
                x = random.randint(0, img.width)
                y = random.randint(0, img.height)
                radius = random.randint(20, 50)
                draw.circle((x, y), (x + radius, y + radius))
            draw(img)

        # Sauvegarder le PDF modifié
        img.save(filename=output_pdf_path)

def apply_random_transformations(img):
    # Rotation aléatoire entre -3 et 3 degrés
    angle = random.uniform(-3, 3)
    img.rotate(angle)

    # Translation aléatoire
    dx = random.randint(-3, 3)
    dy = random.randint(-3, 3)
    img.page = (img.width, img.height, dx, dy)

# Chemin relatif au script actuel
script_dir = Path(__file__).parent
output_dir = script_dir / 'copie_pourrie'
output_dir.mkdir(exist_ok=True)

# Utilisation de la fonction
input_pdf_path = script_dir / '../../copies/copy.png'
num_spots = 5

for i in range(1, 11):
    output_pdf_path = output_dir / f'output_{i}.png'
    add_defects_to_pdf(str(input_pdf_path), str(output_pdf_path), num_spots)

print(output_dir)

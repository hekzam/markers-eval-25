import random
from wand.image import Image
from wand.color import Color
from wand.drawing import Drawing

def add_defects_to_pdf(input_pdf_path, output_pdf_path, num_spots):
    with Image(filename=input_pdf_path, resolution=300) as img:
        # Ajouter du flou
        img.blur(radius=0, sigma=10)  # Réduire le flou en diminuant le sigma
        
        # Ajouter des taches
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

# Utilisation de la fonction
input_pdf_path = '/home/asulf/typst-lib/copies/point.pdf'
num_spots = 5  # Nombre de taches à ajouter

for i in range(1, 11):
    output_pdf_path = f'/home/asulf/pdf_pourrisseur/copie_pourrie/output{i}.pdf'
    add_defects_to_pdf(input_pdf_path, output_pdf_path, num_spots)
import random
import numpy as np
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageEnhance
import argparse

# MAX RANDOM CONSTANTS
SALT_PEPPER = 0.08
CONTRAST = 0.8, 1.2
BRIGHTNESS = 0.8, 1.2
GAUSSIAN = 2, 10
SPOTS = 2, 5
ROTATE = 3
TRANSLATION = -80, 80

def add_defects_to_image(input_image_path, output_image_path, params):
    """Applies only the specified transformations."""
    img = Image.open(input_image_path).convert("RGB")

    # Apply only the transformations present in the parameters
    if 'rotation' in params :
        img = apply_rotation(img, params['rotation'])

    if 'translation' in params:
        img = apply_translation(img, params['translation'])

    if 'contrast' in params:
        img = modify_contrast(img, params['contrast'])

    if 'brightness' in params:
        img = modify_brightness(img, params['brightness'])

    if 'gaussian' in params:
        img = gaussian_noise(img, params['gaussian'])

    if 'salt_pepper' in params:
        img = salt_pepper_noise(img, params['salt_pepper'])

    if 'spot' in params:
        img = add_random_spots(img, params['spot'])
    
    img.save(output_image_path)

def apply_rotation(img, rotation=None):
    """Applies a specific rotation or one proportional to the percentage."""
    if rotation is None:
        rotation = random.uniform(-ROTATE, ROTATE)
    return img.rotate(rotation, expand=False)

def apply_translation(img, translation=None):
    """Applies a translation based on a percentage."""
    if translation is None:
        translation = random.uniform(TRANSLATION[0], TRANSLATION[1]), random.uniform(TRANSLATION[0], TRANSLATION[1])
    dx = translation[0]
    dy = translation[1]
    return img.transform(img.size, Image.AFFINE, (1, 0, dx, 0, 1, dy))

def modify_contrast(img, factor=None):
    """Adjusts contrast based on a percentage."""
    if factor is None:
        factor = random.uniform(0, 100)
    contrast_range = CONTRAST[1] - CONTRAST[0]
    factor = CONTRAST[0] + (factor / 100) * contrast_range
    enhancer = ImageEnhance.Contrast(img)
    return enhancer.enhance(factor)

def modify_brightness(img, factor=None):
    """Adjusts brightness based on a percentage."""
    if factor is None:
        factor = random.uniform(0, 100)
    brightness_range = BRIGHTNESS[1] - BRIGHTNESS[0]
    factor = BRIGHTNESS[0] + (factor / 100) * brightness_range
    enhancer = ImageEnhance.Brightness(img)
    return enhancer.enhance(factor)

def gaussian_noise(img, factor=None):
    """Adds Gaussian noise based on a percentage."""
    if factor is None:
        factor = random.uniform(0, 100)
    noise_level = GAUSSIAN[0] + (factor / 100) * (GAUSSIAN[1] - GAUSSIAN[0])
    img_array = np.array(img)
    noise = np.random.normal(0, noise_level, img_array.shape).astype(np.int16)
    noisy_img = np.clip(img_array + noise, 0, 255).astype(np.uint8)
    return Image.fromarray(noisy_img)

def salt_pepper_noise(img, prob=None):
    """Adds salt-and-pepper noise based on a percentage."""
    if prob is None:
        prob = random.uniform(0, 100)
    prob = (prob / 100) * SALT_PEPPER
    img_array = np.array(img)
    num_noisy = int(prob * img_array.size / 3)

    for _ in range(num_noisy):
        x = random.randint(0, img_array.shape[0] - 1)
        y = random.randint(0, img_array.shape[1] - 1)
        img_array[x, y] = [0, 0, 0] if random.random() < 0.5 else [255, 255, 255]

    return Image.fromarray(img_array)

def add_random_spots(img, nb_spots=None):
    """Adds black circular spots."""
    if nb_spots is None:
        nb_spots = random.randint(SPOTS[0], SPOTS[1])
    nb_spots = int(nb_spots)
    draw = ImageDraw.Draw(img)
    for _ in range(nb_spots):
        x = random.randint(0, img.width)
        y = random.randint(0, img.height)
        radius = random.randint(10, 30)
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill='black')
    return img

def afficher_usage():
    """Displays the usage instructions and exits the program."""
    print("""
Usage: script.py [options]

Available options:  
    rotation=<angle>          : Applies a rotation (in degrees).  
    translation=dx,dy         : Applies a translation (displacement in x and y).  
    contrast=<value>          : Adjusts the contrast (floating-point value).  
    brightness=<value>        : Adjusts the brightness (floating-point value).  
    gaussian=<value>          : Applies Gaussian blur (sigma).  
    salt_pepper=<value>       : Adds salt-and-pepper noise (density).  
    spot=<value>              : Applies a spot effect (intensity).  

Behavior:  
- If one or more options are specified, only these will be applied.  
- If an option is provided without a value, a random value will be used.  
- If no options are provided, all transformations will be applied with random values.
    """)
    sys.exit(1)

def parse_arguments(args):
    """Parses the arguments to customize transformations."""
    transformations = {}

    for arg in args:
        if '=' in arg:
            key, value = arg.split('=')
            key = key.lower()
            if key in ['rotation', 'contrast', 'brightness', 'gaussian', 'salt_pepper', 'spot']:
                transformations[key] = float(value)
            elif key == 'translation':
                try:
                    dx, dy = map(int, value.split(','))
                    transformations[key] = (dx, dy)
                except ValueError:
                    print(f"Incorrect format for translation : {value}")
                    afficher_usage()
                    sys.exit(1)
            elif key == 'spot':
                transformations[key] = int(value)
        else:
            key = arg.lower()
            if key in ['rotation', 'translation', 'contrast', 'brightness', 'gaussian', 'salt_pepper', 'spot']:
                transformations[key] = None
            else:
                afficher_usage()

    return transformations

# Input and output paths
script_dir = Path(__file__).parent
output_dir = script_dir / 'copie_pourrie'
output_dir.mkdir(exist_ok=True)

input_image_path = script_dir / '../../copies/original.png'

parser = argparse.ArgumentParser(description='Applies random or specified printer/scanner transformation to an image')
parser.add_argument('-r', '--rotation', type=int, metavar='', nargs='?', required=False, help='rotation=<angle> : Applies a rotation (in degrees).')
parser.add_argument('-t', '--translation', type=tuple, metavar='', nargs='?', required=False, help='translation=dx,dy : Applies a translation (displacement in x and y).')
parser.add_argument('-c', '--contrast', type=float, metavar='', nargs='?', required=False, help='contrast=<value> : Adjusts the contrast (floating-point value).')
parser.add_argument('-b', '--brightness', type=float, metavar='', nargs='?', required=False, help='brightness=<value> : Adjusts the brightness (floating-point value).')
parser.add_argument('-g', '--gaussian', type=float, metavar='', nargs='?', required=False, help='gaussian=<value> : Applies Gaussian blur (sigma).')
parser.add_argument('-s', '--salt_pepper', type=float, metavar='', nargs='?', required=False, help='salt_pepper=<value> : Adds salt-and-pepper noise (density).')
parser.add_argument('-p', '--spot', type=float, metavar='', nargs='?', required=False, help='spot=<value> : Applies a spot effect (intensity).')
parser.add_argument('-n', '--nb_copy', type=int, metavar='', nargs='?', required=False, help='nb_copy=<value> : Number of copies.')


args = parser.parse_args()
params = vars(args)
params = {k: v for k, v in vars(args).items() if v is not None}


# Vérifier si au moins un argument est spécifié
if any(value is not None for value in params.values()):
    # Garder uniquement les arguments spécifiés et supprimer les autres
    params = {k: v for k, v in params.items() if v is not None}
else:
    
    params = {k: None for k in ['rotation', 'translation', 'contrast', 'brightness', 'gaussian', 'salt_pepper', 'spot']}

print(params)


for i in range(10):
    output_image_path = output_dir / f'output_{i}.png'
    add_defects_to_image(input_image_path, output_image_path, params)
print(f"Modified images saved in: {output_dir}")



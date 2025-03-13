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

def add_defects_to_image(img, output_image_path, params):
    """Applies only the specified transformations."""

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
    if rotation == 0:
        return img
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

# Input and output paths
script_dir = Path(__file__).parent
output_dir = script_dir / 'noisy_copies'
output_dir.mkdir(exist_ok=True)

input_image_path = script_dir / '../../copies/original.png'

parser = argparse.ArgumentParser(description='Applies random or specified printer/scanner transformation to an image')

parser.add_argument('-r', '--rotation', type=int, default=0, help='rotation=<angle> : Applies a rotation (in degrees).')
parser.add_argument('-t', '--translation', type=tuple, default=(0,0), help='translation=dx,dy : Applies a translation (displacement in x and y).')          #PB
parser.add_argument('-c', '--contrast', type=float, default=0.0, help='contrast=<value> : Adjusts the contrast (floating-point value).')
parser.add_argument('-b', '--brightness', type=float, default=0.0, help='brightness=<value> : Adjusts the brightness (floating-point value).')
parser.add_argument('-g', '--gaussian', type=float, default=0.0, help='gaussian=<value> : Applies Gaussian blur (sigma).')
parser.add_argument('-s', '--salt_pepper', type=float, default=0.0, help='salt_pepper=<value> : Adds salt-and-pepper noise (density).')
parser.add_argument('-p', '--spot', type=float, default=0.0, help='spot=<value> : Applies a spot effect (intensity).')
parser.add_argument('-n', '--nb_copy', type=int, default=0, help='nb_copy=<value> : Number of copies.')

args = parser.parse_args()
default_args = parser.parse_args([])
params = vars(args)
params_default = vars(default_args)

is_default = True
nb_copy=10

for key, value in params.items():
    if params_default[key] != value:
        is_default = False
        break
if params["nb_copy"]!=0:
    nb_copy=params["nb_copy"]

if is_default:
    params = {k: None for k, v in vars(args).items()}

img = Image.open(input_image_path).convert("RGB")

for i in range(nb_copy):
    output_image_path = output_dir / f'output_{i}.png'
    add_defects_to_image(img, output_image_path, params)

print(f"Modified images saved in: {output_dir}")
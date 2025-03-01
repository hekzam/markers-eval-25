import cv2
import numpy as np
import fitz  # PyMuPDF
from pdf2image import convert_from_path

# Fonction pour récupérer le DPI du PDF
def get_pdf_dpi(pdf_path):
    doc = fitz.open(pdf_path)
    meta = doc.metadata
    dpi_x, dpi_y = None, None
    
    for page in doc:
        images = page.get_images(full=True)
        if images:
            xref = images[0][0]
            base_image = doc.extract_image(xref)
            if "dpi" in base_image:
                dpi_x, dpi_y = base_image["dpi"]
                break
    
    return dpi_x if dpi_x else 300  # Retourne 300 DPI par défaut si non trouvé

def pixels_to_mm(pixels, dpi):
    return (pixels * 25.4) / dpi

def extract_corner_points(pdf_path):
    dpi = get_pdf_dpi(pdf_path)
    images = convert_from_path(pdf_path, dpi=dpi)
    image = np.array(images[0])
    image_gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    
    blurred = cv2.GaussianBlur(image_gray, (5, 5), 0)
    binary = cv2.adaptiveThreshold(blurred, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                   cv2.THRESH_BINARY_INV, 11, 2)
    
    contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    detected_circles = []
    
    for cnt in contours:
        perimeter = cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, 0.02 * perimeter, True)
        area = cv2.contourArea(cnt)
        
        if len(approx) > 5 and 100 < area < 5000:
            (x, y), radius = cv2.minEnclosingCircle(cnt)
            detected_circles.append((int(x), int(y), int(radius)))
    
    detected_circles = sorted(detected_circles, key=lambda c: (c[1], c[0]))
    
    if len(detected_circles) < 4:
        print("Erreur : Moins de 4 points détectés.")
        return None
    
    top_left = detected_circles[0]
    top_right = detected_circles[-1]
    bottom_left = detected_circles[-2]
    bottom_right = detected_circles[-3]
    
    width = top_right[0] - top_left[0]
    height = bottom_left[1] - top_left[1]
    avg_radius = sum([top_left[2], top_right[2], bottom_left[2], bottom_right[2]]) / 4
    
    return {
        "dpi": dpi,
        "top_left": top_left,
        "top_right": top_right,
        "bottom_left": bottom_left,
        "bottom_right": bottom_right,
        "width": width,
        "height": height,
        "average_radius": avg_radius,
        "width_mm": pixels_to_mm(width, dpi),
        "height_mm": pixels_to_mm(height, dpi),
        "average_diameter_mm": pixels_to_mm(2 * avg_radius, dpi)
    }

# Exemple d'utilisation
pdf_path = "/home/asulf/typst-lib/copies/point.pdf"
result = extract_corner_points(pdf_path)
if result:
    print("DPI détecté :", result["dpi"])
    print("Points détectés :")
    print("Haut gauche:", result["top_left"])
    print("Haut droit:", result["top_right"])
    print("Bas gauche:", result["bottom_left"])
    print("Bas droit:", result["bottom_right"])
    print("Largeur:", result["width"], "pixels ->", result["width_mm"], "mm")
    print("Hauteur:", result["height"], "pixels ->", result["height_mm"], "mm")
    print("Taille moyenne des cercles:", 2 * result["average_radius"], "pixels ->", result["average_diameter_mm"], "mm")

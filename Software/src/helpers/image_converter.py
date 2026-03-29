from PIL import Image
import numpy as np
import os   
import io

def generate_header_guard(header_file):
    # Remove the directory part and replace dots with underscores
    base_name = os.path.basename(header_file).replace('.', '_').upper(  )
    return (base_name,f"ICONS_{base_name}")

def image_to_rgb565_header(image_path, output_path, max_width=320, max_height=240):
    if not os.path.exists(image_path):
        raise FileNotFoundError(f"Das Eingabebild '{image_path}' wurde nicht gefunden.")
    
    img = Image.open(image_path).convert("RGB")
    img.thumbnail((max_width, max_height), Image.LANCZOS)
    width, height = img.size
    print(f"Finale Bildgröße: {width}x{height}")   

    pixels = np.array(img)

    # Convert RGB888 to RGB565
    r = (pixels[:,:,0] >> 3).astype(np.uint16)
    g = (pixels[:,:,1] >> 2).astype(np.uint16)
    b = (pixels[:,:,2] >> 3).astype(np.uint16)
    rgb565 = (r << 11) | (g << 5) | b
    flat = rgb565.flatten()
    
    # Generate the header guard name based on the output file name
    base_name,header_guard = generate_header_guard(output_path)   

    with open(output_path, "w") as f:
        f.write(f"#ifndef {header_guard}\n")
        f.write(f"#define {header_guard}\n\n")
        f.write("#include <Arduino.h> // Required for uint16_t and PROGMEM on ESP32\n")
        f.write(f"const uint16_t {base_name}_WIDTH = {width};\n")
        f.write(f"const uint16_t {base_name}_HEIGHT = {height};\n")
        f.write(f"const uint16_t {base_name}[] PROGMEM = {{\n")
        for i, val in enumerate(flat):
            f.write(f"0x{val:04x}")
            if i < len(flat) - 1:
                f.write(",")
            if (i + 1) % 16 == 0:
                f.write("\n")
        f.write("\n};\n\n")
        f.write("#endif // {header_guard}\n")
    
    print(f"Fertig! {len(flat)} Pixel → {output_path}")



# Beispielaufrufe
image_to_rgb565_header("/home/archduke/downloads/miku_taur.jpg", "/home/archduke/downloads/miku_taur.h")
image_to_rgb565_header("/home/archduke/downloads/taur1.jpg", "/home/archduke/downloads/taur1.h")


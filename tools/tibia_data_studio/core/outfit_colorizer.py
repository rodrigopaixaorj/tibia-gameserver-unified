"""
Colorizador de Outfits e Criaturas do Tibia (Paleta Oficial de 133 Cores CipSoft)
Aplica as máscaras de Head (Amarelo), Body (Vermelho), Legs (Verde) e Feet (Azul) sobre a camada base.
"""

import re
from typing import Tuple, List
from PIL import Image

# Tabela Oficial de 133 Cores do Tibia (0 a 132) da CipSoft em formato HEX
OFFICIAL_TIBIA_HEX = [
    "FFFFFF", "FFD4BF", "FFE9BF", "FFFFBF", "E9FFBF", "D4FFBF",
    "BFFFBF", "BFFFD4", "BFFFE9", "BFFFFF", "BFE9FF", "BFD4FF",
    "BFBFFF", "D4BFFF", "E9BFFF", "FFBFFF", "FFBFE9", "FFBFD4",
    "FFBFBF", "DADADA", "BF9F8F", "BFAF8F", "BFBF8F", "AFBF8F",
    "9FBF8F", "8FBF8F", "8FBF9F", "8FBFAF", "8FBFBF", "8FAFBF",
    "8F9FBF", "8F8FBF", "9F8FBF", "AF8FBF", "BF8FBF", "BF8FAF",
    "BF8F9F", "BF8F8F", "B6B6B6", "BF7F5F", "BFAF8F", "BFBF5F",
    "9FBF5F", "7FBF5F", "5FBF5F", "5FBF7F", "5FBF9F", "5FBFBF",
    "5F9FBF", "5F7FBF", "5F5FBF", "7F5FBF", "9F5FBF", "BF5FBF",
    "BF5F9F", "BF5F7F", "BF5F5F", "919191", "BF6A3F", "BF943F",
    "BFBF3F", "94BF3F", "6ABF3F", "3FBF3F", "3FBF6A", "3FBF94",
    "3FBFBF", "3F94BF", "3F6ABF", "3F3FBF", "6A3FBF", "943FBF",
    "BF3FBF", "BF3F94", "BF3F6A", "BF3F3F", "6D6D6D", "FF5500",
    "FFAA00", "FFFF00", "AAFF00", "54FF00", "00FF00", "00FF54",
    "00FFAA", "00FFFF", "00A9FF", "0055FF", "0000FF", "5500FF",
    "A900FF", "FE00FF", "FF00AA", "FF0055", "FF0000", "484848",
    "BF3F00", "BF7F00", "BFBF00", "7FBF00", "3FBF00", "00BF00",
    "00BF3F", "00BF7F", "00BFBF", "007FBF", "003FBF", "0000BF",
    "3F00BF", "7F00BF", "BF00BF", "BF007F", "BF003F", "BF0000",
    "242424", "7F2A00", "7F5500", "7F7F00", "557F00", "2A7F00",
    "007F00", "007F2A", "007F55", "007F7F", "00547F", "002A7F",
    "00007F", "2A007F", "54007F", "7F007F", "7F0055", "7F002A",
    "7F0000"
]

def _hex_to_rgb(hex_str: str) -> Tuple[int, int, int]:
    h = hex_str.strip().lstrip("#")
    if len(h) == 6:
        return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16))
    return (255, 255, 255)

TIBIA_PALETTE: List[Tuple[int, int, int]] = [_hex_to_rgb(h) for h in OFFICIAL_TIBIA_HEX]

def get_palette_rgb(color_id: int) -> Tuple[int, int, int]:
    """Retorna o valor RGB (0..255) para um índice de cor da paleta oficial do Tibia (0..132)"""
    if 0 <= color_id < len(TIBIA_PALETTE):
        return TIBIA_PALETTE[color_id]
    return (255, 255, 255)

def parse_outfit_colors(colors_str: str) -> Tuple[int, int, int, int]:
    """Converte string de cores em qualquer formato (ex: '113 120 95 115', '113-120-95-115', '113, 120, 95, 115') em 4 inteiros"""
    if not colors_str:
        return (0, 0, 0, 0)
    
    # Extrai todos os números da string
    parts = re.findall(r"\d+", str(colors_str))
    
    head = int(parts[0]) if len(parts) > 0 else 0
    body = int(parts[1]) if len(parts) > 1 else 0
    legs = int(parts[2]) if len(parts) > 2 else 0
    feet = int(parts[3]) if len(parts) > 3 else 0
    
    return (head, body, legs, feet)

def colorize_tile(base_img: Image.Image, mask_img: Image.Image, head_id: int, body_id: int, legs_id: int, feet_id: int) -> Image.Image:
    """Aplica a colorização sobre um tile de 32x32 combinando a camada base e a máscara de cores da CipSoft"""
    if not mask_img:
        return base_img

    head_rgb = get_palette_rgb(head_id)
    body_rgb = get_palette_rgb(body_id)
    legs_rgb = get_palette_rgb(legs_id)
    feet_rgb = get_palette_rgb(feet_id)

    base_raw = bytearray(base_img.tobytes())
    mask_raw = bytearray(mask_img.tobytes())
    out_pixels = bytearray(len(base_raw))

    num_pixels = len(base_raw) // 4

    for i in range(num_pixels):
        idx = i * 4
        b_r = base_raw[idx]
        b_g = base_raw[idx + 1]
        b_b = base_raw[idx + 2]
        b_a = base_raw[idx + 3]

        if b_a == 0:
            out_pixels[idx] = 0
            out_pixels[idx + 1] = 0
            out_pixels[idx + 2] = 0
            out_pixels[idx + 3] = 0
            continue

        m_r = mask_raw[idx]
        m_g = mask_raw[idx + 1]
        m_b = mask_raw[idx + 2]
        m_a = mask_raw[idx + 3]

        target_color = None
        if m_a > 0:
            # Amarelo puro (Head)
            if m_r > 200 and m_g > 200 and m_b < 50:
                target_color = head_rgb
            # Vermelho puro (Body)
            elif m_r > 200 and m_g < 50 and m_b < 50:
                target_color = body_rgb
            # Verde puro (Legs)
            elif m_r < 50 and m_g > 200 and m_b < 50:
                target_color = legs_rgb
            # Azul puro (Feet)
            elif m_r < 50 and m_g < 50 and m_b > 200:
                target_color = feet_rgb

        if target_color:
            c_r, c_g, c_b = target_color
            out_pixels[idx] = (b_r * c_r) // 255
            out_pixels[idx + 1] = (b_g * c_g) // 255
            out_pixels[idx + 2] = (b_b * c_b) // 255
            out_pixels[idx + 3] = b_a
        else:
            out_pixels[idx] = b_r
            out_pixels[idx + 1] = b_g
            out_pixels[idx + 2] = b_b
            out_pixels[idx + 3] = b_a

    return Image.frombytes("RGBA", base_img.size, bytes(out_pixels))

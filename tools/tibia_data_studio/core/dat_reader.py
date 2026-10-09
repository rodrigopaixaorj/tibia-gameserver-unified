"""
Parser do arquivo Tibia.dat (Formato Oficial Tibia 7.4 - 7.72)
Carrega 100% das definições de itens, criaturas/outfits, efeitos e projéteis.
"""

import os
import struct
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

@dataclass
class ThingType:
    id: int
    category: str # "item", "creature", "effect", "distance"
    width: int = 1
    height: int = 1
    exact_size: int = 32
    layers: int = 1
    pattern_x: int = 1
    pattern_y: int = 1
    pattern_z: int = 1
    animations: int = 1
    sprites: List[int] = field(default_factory=list)
    flags: Dict[str, Any] = field(default_factory=dict)

    def get_sprite_index(self, w=0, h=0, l=0, x=0, y=0, z=0, a=0) -> Optional[int]:
        index = ((((((a * self.pattern_z + z) * self.pattern_y + y) * self.pattern_x + x) * self.layers + l) * self.height + h) * self.width + w)
        if 0 <= index < len(self.sprites):
            return self.sprites[index]
        return None

class DatReader:
    def __init__(self, file_path: str = None):
        self.file_path = file_path
        self.signature = 0
        self.items_count = 0
        self.creatures_count = 0
        self.effects_count = 0
        self.distances_count = 0
        
        self.items: Dict[int, ThingType] = {}
        self.creatures: Dict[int, ThingType] = {}
        self.effects: Dict[int, ThingType] = {}
        self.distances: Dict[int, ThingType] = {}
        self.is_loaded = False

        if file_path and os.path.exists(file_path):
            self.load(file_path)

    def load(self, file_path: str) -> bool:
        self.file_path = file_path
        if not os.path.exists(file_path):
            return False

        with open(file_path, "rb") as f:
            data = f.read()

        pos = 0
        self.signature = struct.unpack_from("<I", data, pos)[0]
        pos += 4

        self.items_count, self.creatures_count, self.effects_count, self.distances_count = struct.unpack_from("<HHHH", data, pos)
        pos += 8

        self.items.clear()
        self.creatures.clear()
        self.effects.clear()
        self.distances.clear()

        # Categorias do Tibia 7.72
        categories = [
            ("item", 100, self.items_count, self.items),
            ("creature", 1, self.creatures_count, self.creatures),
            ("effect", 1, self.effects_count, self.effects),
            ("distance", 1, self.distances_count, self.distances),
        ]

        for cat_name, min_id, max_id, target_dict in categories:
            for item_id in range(min_id, max_id + 1):
                thing = ThingType(id=item_id, category=cat_name)
                
                # Leitura de flags até encontrar 0xFF (Fim de Atributos)
                while pos < len(data):
                    opt = data[pos]
                    pos += 1
                    if opt == 0xFF:
                        break
                    
                    # Opcodes e parâmetros conforme o formato Tibia 7.72
                    if opt == 0:  # Ground (2 bytes: velocidade)
                        thing.flags["ground"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 1:  # GroundBorder / Clip
                        thing.flags["clip"] = True
                    elif opt == 2:  # OnBottom
                        thing.flags["bottom"] = True
                    elif opt == 3:  # OnTop
                        thing.flags["top"] = True
                    elif opt == 4:  # Container
                        thing.flags["container"] = True
                    elif opt == 5:  # Stackable
                        thing.flags["stackable"] = True
                    elif opt == 6:  # ForceUse
                        thing.flags["forceuse"] = True
                    elif opt == 7:  # MultiUse
                        thing.flags["multiuse"] = True
                    elif opt == 8:  # Writable (2 bytes: maxTextLen)
                        thing.flags["writable"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 9:  # WritableOnce (2 bytes: maxTextLen)
                        thing.flags["writable_once"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 10: # FluidContainer
                        thing.flags["fluid_container"] = True
                    elif opt == 11: # Splash
                        thing.flags["splash"] = True
                    elif opt == 12: # NotWalkable / Unpass
                        thing.flags["unpass"] = True
                    elif opt == 13: # NotMoveable / Unmove
                        thing.flags["unmove"] = True
                    elif opt == 14: # BlockProjectiles / Unthrow
                        thing.flags["unthrow"] = True
                    elif opt == 15: # BlockPathFind
                        thing.flags["block_path"] = True
                    elif opt == 16: # Pickupable / Take
                        thing.flags["take"] = True
                    elif opt == 17: # Hangable
                        thing.flags["hangable"] = True
                    elif opt == 18: # HookEast
                        thing.flags["hook_east"] = True
                    elif opt == 19: # HookSouth
                        thing.flags["hook_south"] = True
                    elif opt == 20: # Rotatable
                        thing.flags["rotatable"] = True
                    elif opt == 21: # Light (4 bytes: intensity:2, color:2)
                        thing.flags["light_level"] = struct.unpack_from("<H", data, pos)[0]
                        thing.flags["light_color"] = struct.unpack_from("<H", data, pos+2)[0]
                        pos += 4
                    elif opt == 22: # DontHide
                        thing.flags["dont_hide"] = True
                    elif opt == 23: # Translucent
                        thing.flags["translucent"] = True
                    elif opt == 24: # Displacement / Shift (4 bytes: x:2, y:2)
                        thing.flags["shift_x"] = struct.unpack_from("<H", data, pos)[0]
                        thing.flags["shift_y"] = struct.unpack_from("<H", data, pos+2)[0]
                        pos += 4
                    elif opt == 25: # Height / Elevation (2 bytes: elevation)
                        thing.flags["elevation"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 26: # LyingCorpse
                        thing.flags["lying_corpse"] = True
                    elif opt == 27: # AnimateAlways
                        thing.flags["animate_always"] = True
                    elif opt == 28: # Automap / Minimap (2 bytes: color)
                        thing.flags["minimap_color"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 29: # LensHelp (2 bytes: help id)
                        thing.flags["lens_help"] = struct.unpack_from("<H", data, pos)[0]
                        pos += 2
                    elif opt == 30: # FullGround
                        thing.flags["full_ground"] = True
                    elif opt == 31: # Look / IdleAnim
                        thing.flags["idle_anim"] = True
                    elif opt == 32: # Cloth
                        thing.flags["cloth"] = True

                # Leitura da estrutura de sprites
                thing.width = data[pos]; pos += 1
                thing.height = data[pos]; pos += 1
                if thing.width > 1 or thing.height > 1:
                    thing.exact_size = data[pos]; pos += 1
                
                thing.layers = data[pos]; pos += 1
                thing.pattern_x = data[pos]; pos += 1
                thing.pattern_y = data[pos]; pos += 1
                thing.pattern_z = data[pos]; pos += 1
                thing.animations = data[pos]; pos += 1

                total_sprites = (
                    thing.width * thing.height * thing.layers *
                    thing.pattern_x * thing.pattern_y * thing.pattern_z *
                    thing.animations
                )

                sprite_bytes = data[pos : pos + (total_sprites * 2)]
                pos += total_sprites * 2
                thing.sprites = list(struct.unpack(f"<{total_sprites}H", sprite_bytes))

                target_dict[item_id] = thing

        self.is_loaded = True
        return True

    def get_item(self, item_id: int) -> Optional[ThingType]:
        return self.items.get(item_id)

    def get_creature(self, look_type: int) -> Optional[ThingType]:
        return self.creatures.get(look_type)

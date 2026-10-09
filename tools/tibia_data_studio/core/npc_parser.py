"""
Parser e Serializador de Arquivos de NPC (.npc) (CipSoft / OTServ)
Permite gerenciar propriedades gerais (Nome, Sexo, Outfit, Posição Home) e regras de Diálogo/Comportamento.
"""

import os
import re
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

@dataclass
class NpcDialogRule:
    triggers: str
    response: str

@dataclass
class NpcData:
    filename: str
    name: str = ""
    sex: str = "male"
    race: int = 1
    outfit_type: int = 128
    outfit_colors: str = "0-0-0-0"
    home_pos: str = "[32000,32000,7]"
    radius: int = 2
    go_strength: int = 10
    behaviour_lines: List[str] = field(default_factory=list)

    def to_npc_text(self) -> str:
        lines = [
            "# GIMUD - Graphical Interface Multi User Dungeon",
            f"# {self.filename}: Datenbank fuer {self.name}",
            "",
            f'Name = "{self.name}"',
            f"Sex = {self.sex}",
            f"Race = {self.race}",
            f"Outfit = ({self.outfit_type},{self.outfit_colors})",
            f"Home = {self.home_pos}",
            f"Radius = {self.radius}",
            f"GoStrength = {self.go_strength}",
            "",
            "Behaviour = {"
        ]

        for b in self.behaviour_lines:
            lines.append(f"{b}")

        lines.append("}")
        return "\n".join(lines)

class NpcManager:
    def __init__(self, npc_dir: str = None):
        self.npc_dir = npc_dir
        self.npcs: Dict[str, NpcData] = {}
        self.is_loaded = False

        if npc_dir and os.path.exists(npc_dir):
            self.load_all(npc_dir)

    def load_all(self, npc_dir: str) -> bool:
        self.npc_dir = npc_dir
        if not os.path.exists(npc_dir):
            return False

        self.npcs.clear()
        for fname in sorted(os.listdir(npc_dir)):
            if fname.endswith(".npc"):
                full_path = os.path.join(npc_dir, fname)
                n = self.parse_npc_file(full_path, fname)
                if n:
                    self.npcs[fname] = n

        self.is_loaded = True
        return True

    def parse_npc_file(self, file_path: str, filename: str) -> Optional[NpcData]:
        try:
            with open(file_path, "r", encoding="latin-1") as f:
                content = f.read()

            npc = NpcData(filename=filename)

            def get_str(key, default=""):
                m = re.search(rf'^{key}\s*=\s*"?([^"\n\r]+)"?', content, re.MULTILINE | re.IGNORECASE)
                return m.group(1).strip() if m else default

            def get_int(key, default=0):
                m = re.search(rf"^{key}\s*=\s*(\d+)", content, re.MULTILINE | re.IGNORECASE)
                return int(m.group(1)) if m else default

            npc.name = get_str("Name", filename.replace(".npc", ""))
            npc.sex = get_str("Sex", "male")
            npc.race = get_int("Race", 1)
            npc.radius = get_int("Radius", 2)
            npc.go_strength = get_int("GoStrength", 10)

            # Home
            home_m = re.search(r"^Home\s*=\s*(\[[^\]]+\])", content, re.MULTILINE | re.IGNORECASE)
            if home_m:
                npc.home_pos = home_m.group(1)

            # Outfit
            outfit_m = re.search(r"^Outfit\s*=\s*\(\s*(\d+)\s*,\s*([0-9\-]+)\s*\)", content, re.MULTILINE | re.IGNORECASE)
            if outfit_m:
                npc.outfit_type = int(outfit_m.group(1))
                npc.outfit_colors = outfit_m.group(2)

            # Behaviour
            beh_m = re.search(r"Behaviour\s*=\s*\{(.*)\}", content, re.DOTALL | re.IGNORECASE)
            if beh_m:
                raw_beh = beh_m.group(1)
                for line in raw_beh.splitlines():
                    stripped = line.strip()
                    if stripped and not stripped.startswith("#"):
                        npc.behaviour_lines.append(stripped)

            return npc
        except Exception as e:
            print(f"Erro ao parsear NPC {filename}: {e}")
            return None

    def save_npc(self, npc: NpcData, target_dir: str = None) -> bool:
        target_dir = target_dir or self.npc_dir
        if not target_dir:
            return False

        full_path = os.path.join(target_dir, npc.filename)
        with open(full_path, "w", encoding="latin-1") as f:
            f.write(npc.to_npc_text())
        self.npcs[npc.filename] = npc
        return True

    def get_all_list(self) -> List[Dict[str, Any]]:
        result = []
        for fname, npc in self.npcs.items():
            result.append({
                "filename": npc.filename,
                "name": npc.name,
                "sex": npc.sex,
                "race": npc.race,
                "outfit_type": npc.outfit_type,
                "outfit_colors": npc.outfit_colors,
                "home_pos": npc.home_pos,
                "radius": npc.radius,
                "go_strength": npc.go_strength,
                "behaviour_count": len(npc.behaviour_lines),
                "behaviour_lines": npc.behaviour_lines
            })
        return result

"""
Parser e Serializador de Arquivos de Monstros (.mon) (CipSoft / OTServ)
Permite gerenciar atributos, skills, spells, imunidades/flags, falas e a tabela de LOOT completa.
"""

import os
import re
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

@dataclass
class LootItem:
    item_id: int
    count: int = 1
    chance: int = 1000  # Chance em permilagem (1000 = 100%, 1 = 0.1%)

    @property
    def percentage(self) -> float:
        return self.chance / 10.0

@dataclass
class MonsterData:
    filename: str
    race_number: int = 0
    name: str = ""
    article: str = "a"
    outfit_type: int = 0
    outfit_colors: str = "0-0-0-0"
    corpse: int = 0
    blood: str = "Blood"
    experience: int = 0
    summon_cost: int = 0
    flee_threshold: int = 0
    attack: int = 0
    defend: int = 0
    armor: int = 0
    poison: int = 0
    lose_target: int = 0
    strategy: str = "(100, 0, 0, 0)"
    
    hitpoints: int = 100
    speed: int = 100
    
    flags: List[str] = field(default_factory=list)
    skills_raw: List[str] = field(default_factory=list)
    spells_raw: List[str] = field(default_factory=list)
    inventory: List[LootItem] = field(default_factory=list)
    talk: List[str] = field(default_factory=list)

    def to_mon_text(self) -> str:
        lines = [
            "# Tibia - graphical Multi-User-Dungeon",
            "# MonsterRace File",
            "",
            f"RaceNumber    = {self.race_number}",
            f'Name          = "{self.name}"',
            f'Article       = "{self.article}"',
            f"Outfit        = ({self.outfit_type}, {self.outfit_colors})",
            f"Corpse        = {self.corpse}",
            f"Blood         = {self.blood}",
            f"Experience    = {self.experience}",
            f"SummonCost    = {self.summon_cost}",
            f"FleeThreshold = {self.flee_threshold}",
            f"Attack        = {self.attack}",
            f"Defend        = {self.defend}",
            f"Armor         = {self.armor}",
            f"Poison        = {self.poison}",
            f"LoseTarget    = {self.lose_target}",
            f"Strategy      = {self.strategy}",
            ""
        ]

        # Flags
        if self.flags:
            flags_inner = ",\n                 ".join(self.flags)
            lines.append(f"Flags         = {{{flags_inner}}}")
            lines.append("")

        # Skills
        if self.skills_raw:
            skills_inner = ",\n                 ".join(self.skills_raw)
            lines.append(f"Skills        = {{{skills_inner}}}")
            lines.append("")
        else:
            lines.append(f"Skills        = {{(HitPoints, {self.hitpoints}, 0, {self.hitpoints}, 0, 0, 0),")
            lines.append(f"                 (GoStrength, {self.speed}, 0, {self.speed}, 0, 0, 0)}}")
            lines.append("")

        # Spells
        if self.spells_raw:
            spells_inner = ",\n                 ".join(self.spells_raw)
            lines.append(f"Spells        = {{{spells_inner}}}")
            lines.append("")

        # Inventory / Loot
        if self.inventory:
            loot_lines = [f"({item.item_id}, {item.count}, {item.chance})" for item in self.inventory]
            loot_inner = ",\n                 ".join(loot_lines)
            lines.append(f"Inventory     = {{{loot_inner}}}")
            lines.append("")
        else:
            lines.append("Inventory     = {}")
            lines.append("")

        # Talk
        if self.talk:
            talk_quoted = [f'"{t}"' for t in self.talk]
            talk_inner = ",\n                 ".join(talk_quoted)
            lines.append(f"Talk          = {{{talk_inner}}}")
            lines.append("")

        return "\n".join(lines)

class MonsterManager:
    def __init__(self, mon_dir: str = None):
        self.mon_dir = mon_dir
        self.monsters: Dict[str, MonsterData] = {}
        self.is_loaded = False

        if mon_dir and os.path.exists(mon_dir):
            self.load_all(mon_dir)

    def load_all(self, mon_dir: str) -> bool:
        self.mon_dir = mon_dir
        if not os.path.exists(mon_dir):
            return False

        self.monsters.clear()
        for fname in sorted(os.listdir(mon_dir)):
            if fname.endswith(".mon"):
                full_path = os.path.join(mon_dir, fname)
                m = self.parse_mon_file(full_path, fname)
                if m:
                    self.monsters[fname] = m

        self.is_loaded = True
        return True

    def parse_mon_file(self, file_path: str, filename: str) -> Optional[MonsterData]:
        try:
            with open(file_path, "r", encoding="latin-1") as f:
                content = f.read()

            mon = MonsterData(filename=filename)

            # Extração simples por regex de propriedades escalares
            def get_int(key, default=0):
                m = re.search(rf"^{key}\s*=\s*(\d+)", content, re.MULTILINE | re.IGNORECASE)
                return int(m.group(1)) if m else default

            def get_str(key, default=""):
                m = re.search(rf'^{key}\s*=\s*"([^"]*)"', content, re.MULTILINE | re.IGNORECASE)
                return m.group(1) if m else default

            mon.race_number = get_int("RaceNumber", 0)
            mon.name = get_str("Name", filename.replace(".mon", ""))
            mon.article = get_str("Article", "a")
            mon.corpse = get_int("Corpse", 0)
            
            blood_m = re.search(r"^Blood\s*=\s*(\w+)", content, re.MULTILINE | re.IGNORECASE)
            mon.blood = blood_m.group(1) if blood_m else "Blood"

            mon.experience = get_int("Experience", 0)
            mon.summon_cost = get_int("SummonCost", 0)
            mon.flee_threshold = get_int("FleeThreshold", 0)
            mon.attack = get_int("Attack", 0)
            mon.defend = get_int("Defend", 0)
            mon.armor = get_int("Armor", 0)
            mon.poison = get_int("Poison", 0)
            mon.lose_target = get_int("LoseTarget", 0)

            # Strategy
            strat_m = re.search(r"^Strategy\s*=\s*\(([^)]+)\)", content, re.MULTILINE | re.IGNORECASE)
            if strat_m:
                mon.strategy = f"({strat_m.group(1)})"

            # Outfit
            outfit_m = re.search(r"^Outfit\s*=\s*\(\s*(\d+)(?:\s*,\s*([0-9\-\s,]+))?\s*\)", content, re.MULTILINE | re.IGNORECASE)
            if outfit_m:
                mon.outfit_type = int(outfit_m.group(1))
                raw_colors = outfit_m.group(2)
                if raw_colors:
                    c_parts = re.findall(r"\d+", raw_colors)
                    if len(c_parts) >= 4:
                        mon.outfit_colors = f"{c_parts[0]}-{c_parts[1]}-{c_parts[2]}-{c_parts[3]}"
                    elif len(c_parts) > 0:
                        mon.outfit_colors = "-".join(c_parts)
                    else:
                        mon.outfit_colors = "0-0-0-0"
                else:
                    mon.outfit_colors = "0-0-0-0"
            else:
                simple_outfit_m = re.search(r"^Outfit\s*=\s*(\d+)", content, re.MULTILINE | re.IGNORECASE)
                if simple_outfit_m:
                    mon.outfit_type = int(simple_outfit_m.group(1))
                    mon.outfit_colors = "0-0-0-0"

            # Flags
            flags_m = re.search(r"^Flags\s*=\s*\{([^}]*)\}", content, re.MULTILINE | re.IGNORECASE | re.DOTALL)
            if flags_m:
                raw_flags = flags_m.group(1)
                mon.flags = [f.strip() for f in re.split(r"[,\s\n\r]+", raw_flags) if f.strip()]

            # Skills
            skills_m = re.search(r"^Skills\s*=\s*\{([^}]*)\}", content, re.MULTILINE | re.IGNORECASE | re.DOTALL)
            if skills_m:
                raw_skills = skills_m.group(1)
                skill_items = re.findall(r"\(([^)]+)\)", raw_skills)
                for s in skill_items:
                    mon.skills_raw.append(f"({s.strip()})")
                    parts = [p.strip() for p in s.split(",")]
                    if parts and parts[0].lower() == "hitpoints" and len(parts) > 1:
                        try:
                            mon.hitpoints = int(parts[1])
                        except ValueError:
                            pass
                    elif parts and parts[0].lower() == "gostrength" and len(parts) > 1:
                        try:
                            mon.speed = int(parts[1])
                        except ValueError:
                            pass

            # Spells
            spells_m = re.search(r"^Spells\s*=\s*\{([^}]*)\}", content, re.MULTILINE | re.IGNORECASE | re.DOTALL)
            if spells_m:
                raw_spells = spells_m.group(1).strip()
                if raw_spells:
                    # Quebra por linhas / vírgulas respeitando parâmetros
                    mon.spells_raw = [sp.strip() for sp in re.split(r",\s*(?=[A-Za-z])", raw_spells) if sp.strip()]

            # Inventory / Loot
            inv_m = re.search(r"^Inventory\s*=\s*\{([^}]*)\}", content, re.MULTILINE | re.IGNORECASE | re.DOTALL)
            if inv_m:
                raw_inv = inv_m.group(1)
                loot_matches = re.findall(r"\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", raw_inv)
                for item_id_s, count_s, chance_s in loot_matches:
                    mon.inventory.append(LootItem(
                        item_id=int(item_id_s),
                        count=int(count_s),
                        chance=int(chance_s)
                    ))

            # Talk
            talk_m = re.search(r"^Talk\s*=\s*\{([^}]*)\}", content, re.MULTILINE | re.IGNORECASE | re.DOTALL)
            if talk_m:
                raw_talk = talk_m.group(1)
                mon.talk = re.findall(r'"([^"]*)"', raw_talk)

            return mon
        except Exception as e:
            print(f"Erro ao parsear monstro {filename}: {e}")
            return None

    def save_monster(self, mon: MonsterData, target_dir: str = None) -> bool:
        target_dir = target_dir or self.mon_dir
        if not target_dir:
            return False

        full_path = os.path.join(target_dir, mon.filename)
        with open(full_path, "w", encoding="latin-1") as f:
            f.write(mon.to_mon_text())
        self.monsters[mon.filename] = mon
        return True

    def get_all_list(self) -> List[Dict[str, Any]]:
        result = []
        for fname, mon in self.monsters.items():
            result.append({
                "filename": mon.filename,
                "name": mon.name,
                "race_number": mon.race_number,
                "experience": mon.experience,
                "hitpoints": mon.hitpoints,
                "speed": mon.speed,
                "attack": mon.attack,
                "defend": mon.defend,
                "armor": mon.armor,
                "outfit_type": mon.outfit_type,
                "outfit_colors": mon.outfit_colors,
                "corpse": mon.corpse,
                "blood": mon.blood,
                "flee_threshold": mon.flee_threshold,
                "flags": mon.flags,
                "skills_raw": mon.skills_raw,
                "spells_raw": mon.spells_raw,
                "talk": mon.talk,
                "inventory": [
                    {
                        "item_id": it.item_id,
                        "count": it.count,
                        "chance": it.chance,
                        "percentage": it.percentage
                    }
                    for it in mon.inventory
                ]
            })
        return result

"""
Parser e Serializador do arquivo moveuse.dat (CipSoft / OTServ)
Organiza regras por seções (ex: Beds, Doors, Furniture, Keys, Fun) com suporte a adição, edição e deleção.
"""

import os
import re
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

@dataclass
class MoveUseRule:
    id: int
    section: str
    raw_rule: str
    trigger: str = ""
    conditions_str: str = ""
    actions_str: str = ""
    item_ids: List[int] = field(default_factory=list)

    @classmethod
    def from_line(cls, rule_id: int, section: str, line: str) -> "MoveUseRule":
        rule = cls(id=rule_id, section=section, raw_rule=line)
        
        # Extrai IDs de itens mencionados na regra para visualização rápida
        found_ids = [int(m) for m in re.findall(r"\b\d{3,5}\b", line)]
        rule.item_ids = list(dict.fromkeys(found_ids)) # Unique

        if "->" in line:
            left, right = line.split("->", 1)
            left_parts = [p.strip() for p in left.split(",", 1)]
            rule.trigger = left_parts[0] if left_parts else ""
            rule.conditions_str = left_parts[1] if len(left_parts) > 1 else ""
            rule.actions_str = right.strip()
        else:
            rule.trigger = line
        return rule

    def to_line(self) -> str:
        if self.trigger and (self.conditions_str or self.actions_str):
            cond = f", {self.conditions_str}" if self.conditions_str else ""
            return f"{self.trigger}{cond} -> {self.actions_str}"
        return self.raw_rule

@dataclass
class MoveUseSection:
    name: str
    rules: List[MoveUseRule] = field(default_factory=list)

class MoveUseParser:
    def __init__(self, file_path: str = None):
        self.file_path = file_path
        self.sections: List[MoveUseSection] = []
        self.header_comments: List[str] = []
        self.is_loaded = False

        if file_path and os.path.exists(file_path):
            self.load(file_path)

    def load(self, file_path: str) -> bool:
        self.file_path = file_path
        if not os.path.exists(file_path):
            return False

        with open(file_path, "r", encoding="latin-1") as f:
            content = f.read()

        self.sections.clear()
        self.header_comments.clear()

        lines = content.splitlines()
        current_section = None
        rule_counter = 1
        in_header = True

        for line in lines:
            stripped = line.strip()
            if not stripped:
                continue

            if in_header and stripped.startswith("#"):
                self.header_comments.append(line)
                continue

            # Início de seção: BEGIN "Nome"
            begin_match = re.match(r'^BEGIN\s+"?([^"]*)"?', stripped, re.IGNORECASE)
            if begin_match:
                in_header = False
                sec_name = begin_match.group(1).strip()
                current_section = MoveUseSection(name=sec_name)
                self.sections.append(current_section)
                continue

            # Fim de seção: END
            if re.match(r"^END\b", stripped, re.IGNORECASE):
                current_section = None
                continue

            # Comentários dentro do arquivo
            if stripped.startswith("#"):
                continue

            # Regra
            sec_name = current_section.name if current_section else "Global"
            if current_section is None:
                current_section = MoveUseSection(name="Global")
                self.sections.append(current_section)

            rule = MoveUseRule.from_line(rule_counter, sec_name, stripped)
            current_section.rules.append(rule)
            rule_counter += 1

        self.is_loaded = True
        return True

    def save(self, file_path: str = None) -> bool:
        target_path = file_path or self.file_path
        if not target_path:
            return False

        os.makedirs(os.path.dirname(os.path.abspath(target_path)), exist_ok=True)

        lines = []
        if self.header_comments:
            lines.extend(self.header_comments)
            lines.append("")
        else:
            lines.append("# Tibia - graphical Multi-User-Dungeon")
            lines.append("# Move/Use Rules File")
            lines.append("")

        for section in self.sections:
            if section.name == "Global":
                for rule in section.rules:
                    lines.append(rule.to_line())
                lines.append("")
            else:
                lines.append(f'BEGIN "{section.name}"')
                for rule in section.rules:
                    lines.append(rule.to_line())
                lines.append("END")
                lines.append("")

        content = "\n".join(lines)
        with open(target_path, "w", encoding="latin-1") as f:
            f.write(content)

        return True

    def get_summary(self) -> List[Dict[str, Any]]:
        result = []
        for sec in self.sections:
            result.append({
                "name": sec.name,
                "count": len(sec.rules),
                "rules": [
                    {
                        "id": r.id,
                        "section": r.section,
                        "trigger": r.trigger,
                        "conditions": r.conditions_str,
                        "actions": r.actions_str,
                        "raw": r.raw_rule,
                        "item_ids": r.item_ids
                    }
                    for r in sec.rules
                ]
            })
        return result

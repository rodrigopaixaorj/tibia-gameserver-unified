"""
Parser e Serializador do arquivo objects.srv (CipSoft / OTServ)
Permite carregar, editar, filtrar, validar e salvar todos os tipos de itens do servidor.
"""

import os
import re
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional

@dataclass
class ServerObject:
    type_id: int
    name: str = ""
    description: str = ""
    flags: List[str] = field(default_factory=list)
    attributes: Dict[str, Any] = field(default_factory=dict)
    comment: str = ""

    def to_srv_text(self) -> str:
        lines = []
        if self.comment:
            lines.append(f"TypeID      = {self.type_id} # {self.comment}")
        else:
            lines.append(f"TypeID      = {self.type_id}")

        lines.append(f'Name        = "{self.name}"')
        if self.description:
            lines.append(f'Description = "{self.description}"')

        if self.flags:
            flags_str = ", ".join(self.flags)
            lines.append(f"Flags       = {{{flags_str}}}")

        if self.attributes:
            attrs_list = []
            for k, v in self.attributes.items():
                attrs_list.append(f"{k}={v}")
            attrs_str = ", ".join(attrs_list)
            lines.append(f"Attributes  = {{{attrs_str}}}")

        return "\n".join(lines)

class ObjectsParser:
    def __init__(self, file_path: str = None):
        self.file_path = file_path
        self.objects: Dict[int, ServerObject] = {}
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

        self.objects.clear()
        self.header_comments.clear()

        lines = content.splitlines()
        current_obj = None
        in_header = True

        for line in lines:
            stripped = line.strip()
            if not stripped:
                continue

            if in_header and stripped.startswith("#"):
                self.header_comments.append(line)
                continue

            # Início de um novo objeto
            if re.match(r"^TypeID\s*=", stripped, re.IGNORECASE):
                in_header = False
                # Salva o objeto anterior
                if current_obj is not None:
                    self.objects[current_obj.type_id] = current_obj

                # Extrai TypeID e comentário opcional
                comment_match = re.search(r"#\s*(.*)$", stripped)
                comment = comment_match.group(1).strip() if comment_match else ""
                
                # Remove comentário da linha para pegar o número
                clean_line = re.sub(r"#.*$", "", stripped).strip()
                type_val = int(clean_line.split("=")[1].strip())
                current_obj = ServerObject(type_id=type_val, comment=comment)
                continue

            if current_obj is None:
                continue

            # Nome
            if re.match(r"^Name\s*=", stripped, re.IGNORECASE):
                val_part = stripped.split("=", 1)[1].strip()
                name_match = re.search(r'"([^"]*)"', val_part)
                if name_match:
                    current_obj.name = name_match.group(1)
                else:
                    current_obj.name = val_part

            # Descrição
            elif re.match(r"^Description\s*=", stripped, re.IGNORECASE):
                val_part = stripped.split("=", 1)[1].strip()
                desc_match = re.search(r'"([^"]*)"', val_part)
                if desc_match:
                    current_obj.description = desc_match.group(1)
                else:
                    current_obj.description = val_part

            # Flags
            elif re.match(r"^Flags\s*=", stripped, re.IGNORECASE):
                match = re.search(r"\{([^}]*)\}", stripped)
                if match:
                    raw_flags = match.group(1)
                    flags = [f.strip() for f in raw_flags.split(",") if f.strip()]
                    current_obj.flags = flags

            # Attributes
            elif re.match(r"^Attributes\s*=", stripped, re.IGNORECASE):
                match = re.search(r"\{([^}]*)\}", stripped)
                if match:
                    raw_attrs = match.group(1)
                    pairs = [p.strip() for p in raw_attrs.split(",") if p.strip()]
                    for pair in pairs:
                        if "=" in pair:
                            k, v = pair.split("=", 1)
                            k = k.strip()
                            v = v.strip()
                            # Tenta converter para int ou float
                            try:
                                if "." in v:
                                    val = float(v)
                                else:
                                    val = int(v)
                            except ValueError:
                                val = v.strip('"')
                            current_obj.attributes[k] = val

        if current_obj is not None:
            self.objects[current_obj.type_id] = current_obj

        self.is_loaded = True
        return True

    def save(self, file_path: str = None) -> bool:
        target_path = file_path or self.file_path
        if not target_path:
            return False

        # Cria diretório se não existir
        os.makedirs(os.path.dirname(os.path.abspath(target_path)), exist_ok=True)

        lines = []
        if self.header_comments:
            lines.extend(self.header_comments)
            lines.append("")
        else:
            lines.append("# Tibia - graphical Multi-User-Dungeon")
            lines.append("# ObjectTypes File for Server")
            lines.append("")

        for type_id in sorted(self.objects.keys()):
            obj = self.objects[type_id]
            lines.append(obj.to_srv_text())
            lines.append("")

        content = "\n".join(lines)
        with open(target_path, "w", encoding="latin-1") as f:
            f.write(content)

        return True

    def get_all_list(self) -> List[Dict[str, Any]]:
        result = []
        for type_id in sorted(self.objects.keys()):
            obj = self.objects[type_id]
            result.append({
                "type_id": obj.type_id,
                "name": obj.name,
                "description": obj.description,
                "flags": obj.flags,
                "attributes": obj.attributes,
                "comment": obj.comment
            })
        return result

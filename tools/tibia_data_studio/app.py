"""
Tibia Data Studio - Backend Server (Python Flask API)
Fornece APIs REST de alto desempenho para Itens, Monstros, NPCs, MoveUse e Sprites.
"""

import os
import sys
import json
from io import BytesIO
from flask import Flask, jsonify, request, send_file, render_template
from PIL import Image

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, BASE_DIR)

from core.spr_reader import SprReader
from core.dat_reader import DatReader
from core.objects_parser import ObjectsParser, ServerObject
from core.moveuse_parser import MoveUseParser, MoveUseRule, MoveUseSection
from core.monster_parser import MonsterManager, MonsterData, LootItem
from core.npc_parser import NpcManager, NpcData
from core.backup_manager import BackupManager
from core.functions_doc import get_full_catalog
from core.outfit_colorizer import colorize_tile, parse_outfit_colors

app = Flask(__name__, static_folder="static", template_folder="templates")

# Estados globais
spr_reader = SprReader()
dat_reader = DatReader()
objects_parser = ObjectsParser()
moveuse_parser = MoveUseParser()
monster_manager = MonsterManager()
npc_manager = NpcManager()
backup_mgr = BackupManager()

CONFIG_FILE = os.path.join(BASE_DIR, "config.json")

current_config = {
    "data_path": os.path.abspath(os.path.join(BASE_DIR, "..", "..", "..", "CipSoft Server", "tibia-game_data")),
    "client_path": r"C:\Users\Rodrigo\Desktop\Jogos\Tibia772"
}

def load_saved_config():
    global current_config
    if os.path.exists(CONFIG_FILE):
        try:
            with open(CONFIG_FILE, "r", encoding="utf-8") as f:
                saved = json.load(f)
                current_config.update(saved)
        except Exception as e:
            print(f"Erro ao carregar config.json: {e}")

def save_config_to_file():
    try:
        with open(CONFIG_FILE, "w", encoding="utf-8") as f:
            json.dump(current_config, f, indent=2)
    except Exception as e:
        print(f"Erro ao salvar config.json: {e}")

def auto_load_data():
    """Carrega os dados padrão do servidor e do cliente Tibia"""
    load_saved_config()
    
    # Carrega dados do servidor
    data_path = current_config["data_path"]
    if os.path.exists(data_path):
        obj_path = os.path.join(data_path, "dat", "objects.srv")
        if os.path.exists(obj_path):
            objects_parser.load(obj_path)

        mv_path = os.path.join(data_path, "dat", "moveuse.dat")
        if os.path.exists(mv_path):
            moveuse_parser.load(mv_path)

        mon_path = os.path.join(data_path, "mon")
        if os.path.exists(mon_path):
            monster_manager.load_all(mon_path)

        npc_path = os.path.join(data_path, "npc")
        if os.path.exists(npc_path):
            npc_manager.load_all(npc_path)

    # Carrega arquivos do cliente (Tibia.spr e Tibia.dat)
    client_path = current_config["client_path"]
    if client_path and os.path.exists(client_path):
        load_client_files(client_path)

def load_client_files(client_path: str):
    # Procura Tibia.spr (com suporte a maiúsculas e minúsculas)
    spr_path = os.path.join(client_path, "Tibia.spr")
    if not os.path.exists(spr_path):
        spr_path = os.path.join(client_path, "tibia.spr")

    # Procura Tibia.dat
    dat_path = os.path.join(client_path, "Tibia.dat")
    if not os.path.exists(dat_path):
        dat_path = os.path.join(client_path, "tibia.dat")

    if os.path.exists(spr_path):
        spr_reader.load(spr_path)
    if os.path.exists(dat_path):
        dat_reader.load(dat_path)

# ----------------- ROTAS WEB & STATIC -----------------

@app.route("/")
def index():
    return render_template("index.html")

# ----------------- CONFIGURAÇÕES & DIRETÓRIOS -----------------

@app.route("/api/config", methods=["GET"])
def get_config():
    return jsonify({
        "data_path": current_config["data_path"],
        "client_path": current_config["client_path"],
        "objects_loaded": objects_parser.is_loaded,
        "objects_count": len(objects_parser.objects),
        "moveuse_loaded": moveuse_parser.is_loaded,
        "moveuse_sections": len(moveuse_parser.sections),
        "monsters_loaded": monster_manager.is_loaded,
        "monsters_count": len(monster_manager.monsters),
        "npcs_loaded": npc_manager.is_loaded,
        "npcs_count": len(npc_manager.npcs),
        "spr_loaded": spr_reader.is_loaded,
        "spr_count": spr_reader.sprite_count,
        "dat_loaded": dat_reader.is_loaded,
        "dat_items": len(dat_reader.items),
        "dat_creatures": len(dat_reader.creatures)
    })

@app.route("/api/config/load", methods=["POST"])
def set_config_load():
    try:
        data = request.json or {}
        data_path = data.get("data_path", current_config["data_path"]).strip()
        client_path = data.get("client_path", current_config["client_path"]).strip()

        if data_path:
            current_config["data_path"] = data_path
            obj_path = os.path.join(data_path, "dat", "objects.srv")
            if os.path.exists(obj_path):
                objects_parser.load(obj_path)

            mv_path = os.path.join(data_path, "dat", "moveuse.dat")
            if os.path.exists(mv_path):
                moveuse_parser.load(mv_path)

            mon_path = os.path.join(data_path, "mon")
            if os.path.exists(mon_path):
                monster_manager.load_all(mon_path)

            npc_path = os.path.join(data_path, "npc")
            if os.path.exists(npc_path):
                npc_manager.load_all(npc_path)

        if client_path:
            current_config["client_path"] = client_path
            if os.path.exists(client_path):
                load_client_files(client_path)

        save_config_to_file()
        return get_config()
    except Exception as e:
        print(f"Erro em set_config_load: {e}")
        return jsonify({"error": str(e)}), 500

# ----------------- SPRITES & RENDERING -----------------

@app.route("/api/sprite/<int:sprite_id>")
def get_sprite(sprite_id: int):
    zoom = int(request.args.get("zoom", 1))
    zoom = max(1, min(8, zoom))

    if spr_reader.is_loaded and sprite_id > 0:
        png_bytes = spr_reader.get_sprite_png_bytes(sprite_id, zoom=zoom)
    else:
        img = Image.new("RGBA", (32 * zoom, 32 * zoom), (40, 44, 52, 180))
        buf = BytesIO()
        img.save(buf, format="PNG")
        png_bytes = buf.getvalue()

    return send_file(BytesIO(png_bytes), mimetype="image/png")

def render_thing_composite(thing, zoom=1, anim=0, colors_str=""):
    """Renderiza um item ou criatura compondo todos os seus tiles (ex: 64x64, 32x64) e aplicando colorização de outfit"""
    if not thing or not thing.sprites or not spr_reader.is_loaded:
        img = Image.new("RGBA", (32 * zoom, 32 * zoom), (30, 34, 42, 200))
        buf = BytesIO()
        img.save(buf, format="PNG")
        return buf.getvalue()

    width = thing.width
    height = thing.height
    base_img = Image.new("RGBA", (width * 32, height * 32), (0, 0, 0, 0))

    head, body, legs, feet = parse_outfit_colors(colors_str)
    has_mask = (thing.layers >= 2)

    dir_x = 2 if thing.pattern_x >= 3 else 0

    for w in range(width):
        for h in range(height):
            px = (width - 1 - w) * 32
            py = (height - 1 - h) * 32

            if has_mask:
                # Camada 0: Base, Camada 1: Máscara
                spr_base_id = thing.get_sprite_index(w=w, h=h, l=0, x=dir_x, a=anim)
                spr_mask_id = thing.get_sprite_index(w=w, h=h, l=1, x=dir_x, a=anim)
                
                spr_base = spr_reader.get_sprite_image(spr_base_id) if spr_base_id else None
                spr_mask = spr_reader.get_sprite_image(spr_mask_id) if spr_mask_id else None

                if spr_base:
                    if spr_mask:
                        colored_tile = colorize_tile(spr_base, spr_mask, head, body, legs, feet)
                        base_img.alpha_composite(colored_tile, (px, py))
                    else:
                        base_img.alpha_composite(spr_base, (px, py))
            else:
                for l in range(thing.layers):
                    sprite_id = thing.get_sprite_index(w=w, h=h, l=l, x=dir_x, a=anim)
                    if sprite_id and sprite_id > 0:
                        sprite_img = spr_reader.get_sprite_image(sprite_id)
                        base_img.alpha_composite(sprite_img, (px, py))

    if zoom > 1:
        base_img = base_img.resize((width * 32 * zoom, height * 32 * zoom), Image.Resampling.NEAREST)

    buf = BytesIO()
    base_img.save(buf, format="PNG")
    return buf.getvalue()

@app.route("/api/item_sprite/<int:type_id>")
def get_item_sprite(type_id: int):
    zoom = int(request.args.get("zoom", 1))
    zoom = max(1, min(8, zoom))

    if dat_reader.is_loaded:
        item = dat_reader.get_item(type_id)
        if item:
            png_bytes = render_thing_composite(item, zoom=zoom)
            return send_file(BytesIO(png_bytes), mimetype="image/png")

    img = Image.new("RGBA", (32 * zoom, 32 * zoom), (30, 34, 42, 200))
    buf = BytesIO()
    img.save(buf, format="PNG")
    return send_file(BytesIO(buf.getvalue()), mimetype="image/png")

@app.route("/api/outfit_sprite/<int:look_type>")
def get_outfit_sprite(look_type: int):
    zoom = int(request.args.get("zoom", 1))
    zoom = max(1, min(8, zoom))
    colors_str = request.args.get("colors", "")

    if dat_reader.is_loaded:
        creature = dat_reader.get_creature(look_type)
        if creature:
            png_bytes = render_thing_composite(creature, zoom=zoom, colors_str=colors_str)
            return send_file(BytesIO(png_bytes), mimetype="image/png")

    img = Image.new("RGBA", (32 * zoom, 32 * zoom), (35, 40, 50, 200))
    buf = BytesIO()
    img.save(buf, format="PNG")
    return send_file(BytesIO(buf.getvalue()), mimetype="image/png")

MAGIC_EFFECTS = [
    {"id": 1, "name": "Draw Blood (Red Spark)", "desc": "Faísca vermelha de impacto físico / Red physical spark"},
    {"id": 2, "name": "Lose Energy (Blue Spark)", "desc": "Faísca azul de impacto mágico / Blue energy spark"},
    {"id": 3, "name": "Poff / Puff", "desc": "Fumaça azul/cinza de desaparecimento / Smoke puff"},
    {"id": 4, "name": "Block (Yellow Spark)", "desc": "Faísca amarela de bloqueio em escudo / Yellow shield spark"},
    {"id": 5, "name": "Magic Blue (Energy)", "desc": "Raio mágico azul / Teleport / Blue energy rays"},
    {"id": 6, "name": "Magic Red (Fire)", "desc": "Chama mágica vermelha / Fogo / Red fire flare"},
    {"id": 7, "name": "Magic Green (Poison)", "desc": "Brilho verde de veneno / Green poison spark"},
    {"id": 8, "name": "Hit Area (Fire Area)", "desc": "Área de chamas e fogo / Fire area blast"},
    {"id": 9, "name": "Teleport Yellow", "desc": "Explosão amarela de teleport / Yellow teleport flash"},
    {"id": 10, "name": "Mort Area (Death / SD)", "desc": "Efeito de morte (Sudden Death) / Death damage area"},
    {"id": 11, "name": "Sound Green (Music Notes)", "desc": "Notas musicais verdes / Green musical notes"},
    {"id": 12, "name": "Sound Red (Music Notes)", "desc": "Notas musicais vermelhas / Red musical notes"},
    {"id": 13, "name": "Poison Area", "desc": "Nuvem tóxica de veneno em área / Poison toxic cloud"},
    {"id": 14, "name": "Yellow Energy", "desc": "Explosão amarela / Faísca de choque / Yellow spark wave"},
    {"id": 15, "name": "Explosion", "desc": "Explosão de terra (Explosion rune) / Ground explosion"},
    {"id": 16, "name": "Plant Attack (Stones)", "desc": "Pedras caindo / Ataque de planta / Falling rocks"},
    {"id": 17, "name": "Blue Energy (Lightning)", "desc": "Feixe de eletricidade azul / Energy lightning"},
    {"id": 18, "name": "Firework (Giant Explosion)", "desc": "Fogos de artifício com estrelas / Fireworks blast"},
    {"id": 19, "name": "Dice Roll", "desc": "Dado rolando (1 a 6) / Rolling dice"},
    {"id": 20, "name": "Gift / Present", "desc": "Presente de natal animado / Animated gift box"},
    {"id": 21, "name": "Critical / Tear", "desc": "Lágrima / Brilho de golpe crítico / Critical hit tears"},
    {"id": 22, "name": "Stun / Thunder", "desc": "Trovão / Atordoamento / Thunder stun"},
    {"id": 23, "name": "Assassin Smoke", "desc": "Fumaça preta de assassino / Black assassin smoke"},
    {"id": 24, "name": "Bubble", "desc": "Bolhas de água mágicas / Water bubbles"},
    {"id": 25, "name": "Hearts / Love", "desc": "Corações vermelhos apaixonados / Red love hearts"}
]

@app.route("/api/effects", methods=["GET"])
def get_magic_effects():
    return jsonify({"effects": MAGIC_EFFECTS})

@app.route("/api/effect_sprite/<int:effect_id>")
def get_effect_sprite(effect_id: int):
    zoom = int(request.args.get("zoom", 1))
    zoom = max(1, min(8, zoom))

    if dat_reader.is_loaded:
        effect = dat_reader.effects.get(effect_id)
        if effect and effect.sprites:
            anim_idx = min(len(effect.sprites) // 2, len(effect.sprites) - 1)
            sprite_id = effect.sprites[anim_idx]
            if sprite_id > 0:
                png_bytes = spr_reader.get_sprite_png_bytes(sprite_id, zoom=zoom)
                return send_file(BytesIO(png_bytes), mimetype="image/png")

    img = Image.new("RGBA", (32 * zoom, 32 * zoom), (45, 35, 60, 200))
    buf = BytesIO()
    img.save(buf, format="PNG")
    return send_file(BytesIO(buf.getvalue()), mimetype="image/png")

# ----------------- ITENS (OBJECTS.SRV) -----------------

@app.route("/api/objects", methods=["GET"])
def get_objects():
    query = request.args.get("q", "").lower().strip()
    flag_filter = request.args.get("flag", "").strip()
    page = int(request.args.get("page", 1))
    limit = int(request.args.get("limit", 50))

    all_objs = objects_parser.get_all_list()

    filtered = []
    for obj in all_objs:
        if query:
            match_id = str(obj["type_id"]).startswith(query)
            match_name = query in obj["name"].lower()
            match_desc = query in obj["description"].lower()
            if not (match_id or match_name or match_desc):
                continue

        if flag_filter:
            # Aceita várias flags separadas por vírgula (todas devem estar presentes)
            required_flags = [f.strip() for f in flag_filter.split(",") if f.strip()]
            if not all(f in obj["flags"] for f in required_flags):
                continue

        filtered.append(obj)

    total = len(filtered)
    start_idx = (page - 1) * limit
    end_idx = start_idx + limit
    paginated = filtered[start_idx:end_idx]

    return jsonify({
        "total": total,
        "page": page,
        "limit": limit,
        "pages": (total + limit - 1) // limit if limit > 0 else 1,
        "items": paginated
    })

@app.route("/api/objects/<int:type_id>", methods=["GET"])
def get_object(type_id: int):
    obj = objects_parser.objects.get(type_id)
    if not obj:
        return jsonify({"error": "Objeto não encontrado"}), 404
    return jsonify({
        "type_id": obj.type_id,
        "name": obj.name,
        "description": obj.description,
        "flags": obj.flags,
        "attributes": obj.attributes,
        "comment": obj.comment
    })

@app.route("/api/objects/save", methods=["POST"])
def save_object():
    data = request.json or {}
    type_id = int(data.get("type_id", -1))
    if type_id < 0:
        return jsonify({"error": "TypeID inválido"}), 400

    obj = ServerObject(
        type_id=type_id,
        name=data.get("name", ""),
        description=data.get("description", ""),
        flags=data.get("flags", []),
        attributes=data.get("attributes", {}),
        comment=data.get("comment", "")
    )

    objects_parser.objects[type_id] = obj

    if objects_parser.file_path:
        backup_mgr.create_backup(objects_parser.file_path)
        objects_parser.save()

    return jsonify({"success": True, "type_id": type_id, "message": f"Item {type_id} salvo com sucesso!"})

@app.route("/api/objects/delete", methods=["POST"])
def delete_object():
    data = request.json or {}
    type_id = int(data.get("type_id", -1))
    if type_id in objects_parser.objects:
        del objects_parser.objects[type_id]
        if objects_parser.file_path:
            backup_mgr.create_backup(objects_parser.file_path)
            objects_parser.save()
        return jsonify({"success": True, "message": f"Item {type_id} removido."})
    return jsonify({"error": "Item não encontrado"}), 404

# ----------------- MONSTROS (.MON) -----------------

@app.route("/api/monsters", methods=["GET"])
def get_monsters():
    query = request.args.get("q", "").lower().strip()
    all_monsters = monster_manager.get_all_list()

    filtered = []
    for m in all_monsters:
        if query:
            match_name = query in m["name"].lower()
            match_file = query in m["filename"].lower()
            match_race = str(m["race_number"]).startswith(query)
            if not (match_name or match_file or match_race):
                continue
        filtered.append(m)

    return jsonify({
        "total": len(filtered),
        "monsters": filtered
    })

@app.route("/api/monsters/<filename>", methods=["GET"])
def get_monster(filename: str):
    m = monster_manager.monsters.get(filename)
    if not m:
        return jsonify({"error": "Monstro não encontrado"}), 404
    return jsonify({
        "filename": m.filename,
        "name": m.name,
        "article": m.article,
        "race_number": m.race_number,
        "outfit_type": m.outfit_type,
        "outfit_colors": m.outfit_colors,
        "corpse": m.corpse,
        "blood": m.blood,
        "experience": m.experience,
        "summon_cost": m.summon_cost,
        "flee_threshold": m.flee_threshold,
        "attack": m.attack,
        "defend": m.defend,
        "armor": m.armor,
        "poison": m.poison,
        "lose_target": m.lose_target,
        "strategy": m.strategy,
        "hitpoints": m.hitpoints,
        "speed": m.speed,
        "flags": m.flags,
        "skills_raw": m.skills_raw,
        "spells_raw": m.spells_raw,
        "talk": m.talk,
        "inventory": [
            {
                "item_id": it.item_id,
                "count": it.count,
                "chance": it.chance,
                "percentage": it.percentage
            }
            for it in m.inventory
        ]
    })

@app.route("/api/monsters/save", methods=["POST"])
def save_monster():
    data = request.json or {}
    filename = data.get("filename", "").strip()
    if not filename:
        return jsonify({"error": "Nome de arquivo inválido"}), 400

    if not filename.endswith(".mon"):
        filename += ".mon"

    inv_list = []
    for item in data.get("inventory", []):
        inv_list.append(LootItem(
            item_id=int(item.get("item_id", 0)),
            count=int(item.get("count", 1)),
            chance=int(item.get("chance", 1000))
        ))

    mon = MonsterData(
        filename=filename,
        race_number=int(data.get("race_number", 0)),
        name=data.get("name", ""),
        article=data.get("article", "a"),
        outfit_type=int(data.get("outfit_type", 0)),
        outfit_colors=data.get("outfit_colors", "0-0-0-0"),
        corpse=int(data.get("corpse", 0)),
        blood=data.get("blood", "Blood"),
        experience=int(data.get("experience", 0)),
        summon_cost=int(data.get("summon_cost", 0)),
        flee_threshold=int(data.get("flee_threshold", 0)),
        attack=int(data.get("attack", 0)),
        defend=int(data.get("defend", 0)),
        armor=int(data.get("armor", 0)),
        poison=int(data.get("poison", 0)),
        lose_target=int(data.get("lose_target", 0)),
        strategy=data.get("strategy", "(100, 0, 0, 0)"),
        hitpoints=int(data.get("hitpoints", 100)),
        speed=int(data.get("speed", 100)),
        flags=data.get("flags", []),
        skills_raw=data.get("skills_raw", []),
        spells_raw=data.get("spells_raw", []),
        inventory=inv_list,
        talk=data.get("talk", [])
    )

    if monster_manager.mon_dir:
        full_p = os.path.join(monster_manager.mon_dir, filename)
        backup_mgr.create_backup(full_p)
        monster_manager.save_monster(mon)

    return jsonify({"success": True, "filename": filename, "message": f"Monstro {mon.name} salvo com sucesso!"})

# ----------------- MOVEUSE (MOVEUSE.DAT) -----------------

@app.route("/api/moveuse", methods=["GET"])
def get_moveuse():
    query = request.args.get("q", "").lower().strip()
    summary = moveuse_parser.get_summary()

    if query:
        filtered_summary = []
        for sec in summary:
            matched_rules = [
                r for r in sec["rules"]
                if query in r["raw"].lower() or query in str(r["item_ids"])
            ]
            if matched_rules:
                filtered_summary.append({
                    "name": sec["name"],
                    "count": len(matched_rules),
                    "rules": matched_rules
                })
        return jsonify({"sections": filtered_summary})

    return jsonify({"sections": summary})

@app.route("/api/moveuse/save", methods=["POST"])
def save_moveuse():
    data = request.json or {}
    sections_data = data.get("sections", [])

    new_sections = []
    rule_id_seq = 1

    for s in sections_data:
        sec = MoveUseSection(name=s.get("name", "Geral"))
        for r in s.get("rules", []):
            raw = r.get("raw") or f"{r.get('trigger', 'Use')}, {r.get('conditions', '')} -> {r.get('actions', '')}"
            rule_obj = MoveUseRule.from_line(rule_id_seq, sec.name, raw)
            sec.rules.append(rule_obj)
            rule_id_seq += 1
        new_sections.append(sec)

    moveuse_parser.sections = new_sections

    if moveuse_parser.file_path:
        backup_mgr.create_backup(moveuse_parser.file_path)
        moveuse_parser.save()

    return jsonify({"success": True, "message": "Arquivo moveuse.dat salvo com sucesso!"})

@app.route("/api/moveuse/docs", methods=["GET"])
def get_moveuse_docs():
    lang = request.args.get("lang", "pt_BR")
    return jsonify(get_full_catalog(lang))

# ----------------- NPCS (.NPC) -----------------

@app.route("/api/npcs", methods=["GET"])
def get_npcs():
    query = request.args.get("q", "").lower().strip()
    all_npcs = npc_manager.get_all_list()

    filtered = []
    for n in all_npcs:
        if query:
            match_name = query in n["name"].lower()
            match_file = query in n["filename"].lower()
            if not (match_name or match_file):
                continue
        filtered.append(n)

    return jsonify({
        "total": len(filtered),
        "npcs": filtered
    })

@app.route("/api/npcs/<filename>", methods=["GET"])
def get_npc(filename: str):
    npc = npc_manager.npcs.get(filename)
    if not npc:
        return jsonify({"error": "NPC não encontrado"}), 404
    return jsonify({
        "filename": npc.filename,
        "name": npc.name,
        "sex": npc.sex,
        "race": npc.race,
        "outfit_type": npc.outfit_type,
        "outfit_colors": npc.outfit_colors,
        "home_pos": npc.home_pos,
        "radius": npc.radius,
        "go_strength": npc.go_strength,
        "behaviour_lines": npc.behaviour_lines
    })

@app.route("/api/npcs/save", methods=["POST"])
def save_npc():
    data = request.json or {}
    filename = data.get("filename", "").strip()
    if not filename:
        return jsonify({"error": "Nome de arquivo inválido"}), 400

    if not filename.endswith(".npc"):
        filename += ".npc"

    npc = NpcData(
        filename=filename,
        name=data.get("name", ""),
        sex=data.get("sex", "male"),
        race=int(data.get("race", 1)),
        outfit_type=int(data.get("outfit_type", 128)),
        outfit_colors=data.get("outfit_colors", "0-0-0-0"),
        home_pos=data.get("home_pos", "[32000,32000,7]"),
        radius=int(data.get("radius", 2)),
        go_strength=int(data.get("go_strength", 10)),
        behaviour_lines=data.get("behaviour_lines", [])
    )

    if npc_manager.npc_dir:
        full_p = os.path.join(npc_manager.npc_dir, filename)
        backup_mgr.create_backup(full_p)
        npc_manager.save_npc(npc)

    return jsonify({"success": True, "filename": filename, "message": f"NPC {npc.name} salvo com sucesso!"})

if __name__ == "__main__":
    auto_load_data()
    port = int(os.environ.get("PORT", 8080))
    print(f"=======================================================")
    print(f"   TIBIA DATA STUDIO - INICIANDO SERVIDOR")
    print(f"   Acesse no seu navegador: http://localhost:{port}")
    print(f"=======================================================")
    app.run(host="0.0.0.0", port=port, debug=False)

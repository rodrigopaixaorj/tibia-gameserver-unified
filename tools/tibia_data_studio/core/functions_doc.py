"""
Catálogo Completo de Funções, Eventos, Condições e Ações da CipSoft (Move/Use Engine)
Fornece metadados, esquemas de parâmetros, explicações e exemplos práticos em Português (pt_BR), Inglês (en) e Espanhol (es).
"""

from typing import List, Dict, Any

# =============================================================================
# CATÁLOGO DE EVENTOS (TRIGGERS)
# =============================================================================
EVENTS_I18N = {
    "Use": {
        "pt_BR": {
            "name": "Use",
            "category": "Interação",
            "desc": "Disparado quando o jogador clica com 'Use' diretamente sobre o objeto (Obj1).",
            "example": "Use, IsType(Obj1, 2487) -> Change(Obj1, 2495, 0)"
        },
        "en": {
            "name": "Use",
            "category": "Interaction",
            "desc": "Triggered when the player clicks 'Use' directly on the object (Obj1).",
            "example": "Use, IsType(Obj1, 2487) -> Change(Obj1, 2495, 0)"
        },
        "es": {
            "name": "Use",
            "category": "Interacción",
            "desc": "Disparado cuando el jugador hace clic en 'Use' directamente sobre el objeto (Obj1).",
            "example": "Use, IsType(Obj1, 2487) -> Change(Obj1, 2495, 0)"
        }
    },
    "MultiUse": {
        "pt_BR": {
            "name": "MultiUse (Use With)",
            "category": "Interação",
            "desc": "Disparado quando o jogador usa o objeto 1 (Obj1 - ex: chave, corda, pá) sobre o objeto 2 (Obj2 - ex: porta, buraco).",
            "example": "MultiUse, IsType(Obj1, 2970), IsType(Obj2, 1724) -> Change(Obj2, 1726, 0)"
        },
        "en": {
            "name": "MultiUse (Use With)",
            "category": "Interaction",
            "desc": "Triggered when the player uses object 1 (Obj1 - e.g. key, rope, shovel) on object 2 (Obj2 - e.g. door, hole).",
            "example": "MultiUse, IsType(Obj1, 2970), IsType(Obj2, 1724) -> Change(Obj2, 1726, 0)"
        },
        "es": {
            "name": "MultiUse (Usar Con)",
            "category": "Interacción",
            "desc": "Disparado cuando el jugador usa el objeto 1 (Obj1 - ej: llave, cuerda, pala) sobre el objeto 2 (Obj2 - ej: puerta, agujero).",
            "example": "MultiUse, IsType(Obj1, 2970), IsType(Obj2, 1724) -> Change(Obj2, 1726, 0)"
        }
    },
    "Movement": {
        "pt_BR": {
            "name": "Movement",
            "category": "Movimento",
            "desc": "Disparado quando um objeto ou criatura é arrastada ou caminha sobre o tile.",
            "example": "Movement, IsType(Obj1, 2147) -> Effect(Obj1, 15)"
        },
        "en": {
            "name": "Movement",
            "category": "Movement",
            "desc": "Triggered when an object or creature is dragged or walks onto the tile.",
            "example": "Movement, IsType(Obj1, 2147) -> Effect(Obj1, 15)"
        },
        "es": {
            "name": "Movement",
            "category": "Movimiento",
            "desc": "Disparado cuando un objeto o criatura es arrastrada o camina sobre la casilla (tile).",
            "example": "Movement, IsType(Obj1, 2147) -> Effect(Obj1, 15)"
        }
    },
    "Collision": {
        "pt_BR": {
            "name": "Collision (Pisar sobre)",
            "category": "Movimento",
            "desc": "Disparado no momento exato em que uma criatura ou jogador pisa em cima do objeto (ex: switch, alavanca de piso, armadilha).",
            "example": "Collision, IsType(Obj1, 1558) -> Change(Obj1, 1559, 0), Effect(Obj1, 3)"
        },
        "en": {
            "name": "Collision (Step on)",
            "category": "Movement",
            "desc": "Triggered at the exact moment a creature or player steps on the object (e.g. floor switch, pressure lever, trap).",
            "example": "Collision, IsType(Obj1, 1558) -> Change(Obj1, 1559, 0), Effect(Obj1, 3)"
        },
        "es": {
            "name": "Collision (Pisar sobre)",
            "category": "Movimiento",
            "desc": "Disparado en el momento exacto en que una criatura o jugador pisa sobre el objeto (ej: interruptor de suelo, trampa).",
            "example": "Collision, IsType(Obj1, 1558) -> Change(Obj1, 1559, 0), Effect(Obj1, 3)"
        }
    },
    "Separation": {
        "pt_BR": {
            "name": "Separation (Sair de cima)",
            "category": "Movimento",
            "desc": "Disparado quando o jogador ou criatura sai de cima do objeto (ex: placa de pressão voltando ao normal).",
            "example": "Separation, IsType(Obj1, 1559) -> Change(Obj1, 1558, 0)"
        },
        "en": {
            "name": "Separation (Step off)",
            "category": "Movement",
            "desc": "Triggered when the player or creature steps off the object (e.g. pressure plate returning to idle state).",
            "example": "Separation, IsType(Obj1, 1559) -> Change(Obj1, 1558, 0)"
        },
        "es": {
            "name": "Separation (Salir de encima)",
            "category": "Movimiento",
            "desc": "Disparado cuando el jugador o criatura sale de encima del objeto (ej: placa de presión volviendo a su estado normal).",
            "example": "Separation, IsType(Obj1, 1559) -> Change(Obj1, 1558, 0)"
        }
    }
}

# =============================================================================
# CATÁLOGO DE CONDIÇÕES
# =============================================================================
CONDITIONS_RAW = [
    {
        "id": "IsType",
        "template": "IsType ({Target},{TypeID})",
        "example": "IsType (Obj1,2487)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "TypeID", "type": "item_id", "default": 100}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsType (Verificar Tipo do Item)",
                "category": "Itens & Tipos",
                "desc": "Verifica se o alvo (Obj1 ou Obj2) corresponde ao TypeID especificado.",
                "param_descs": ["Alvo a testar (Obj1, Obj2)", "ID do tipo de item a verificar"]
            },
            "en": {
                "name": "IsType (Check Item Type)",
                "category": "Items & Types",
                "desc": "Checks if the target (Obj1 or Obj2) matches the specified TypeID.",
                "param_descs": ["Target to test (Obj1, Obj2)", "Item TypeID to check"]
            },
            "es": {
                "name": "IsType (Verificar Tipo de Objeto)",
                "category": "Objetos y Tipos",
                "desc": "Comprueba si el objetivo (Obj1 o Obj2) coincide con el TypeID especificado.",
                "param_descs": ["Objetivo a comprobar (Obj1, Obj2)", "ID del tipo de objeto a verificar"]
            }
        }
    },
    {
        "id": "IsPosition",
        "template": "IsPosition ({Target},{Position})",
        "example": "IsPosition (Obj1,[32360,32199,7])",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsPosition (Verificar Posição Exata)",
                "category": "Posição & Mapa",
                "desc": "Verifica se o alvo está exatamente nas coordenadas [X, Y, Z] informadas.",
                "param_descs": ["Alvo a testar (Obj1, User)", "Coordenadas absolutas [X, Y, Z]"]
            },
            "en": {
                "name": "IsPosition (Check Exact Position)",
                "category": "Position & Map",
                "desc": "Checks if the target is exactly at the given [X, Y, Z] coordinates.",
                "param_descs": ["Target to test (Obj1, User)", "Absolute coordinates [X, Y, Z]"]
            },
            "es": {
                "name": "IsPosition (Verificar Posición Exacta)",
                "category": "Posición y Mapa",
                "desc": "Comprueba si el objetivo está exactamente en las coordenadas [X, Y, Z] indicadas.",
                "param_descs": ["Objetivo a comprobar (Obj1, User)", "Coordenadas absolutas [X, Y, Z]"]
            }
        }
    },
    {
        "id": "IsPlayer",
        "template": "IsPlayer ({Target})",
        "example": "IsPlayer (User)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "User"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsPlayer (Verificar se é Jogador)",
                "category": "Jogador",
                "desc": "Verifica se o alvo é um jogador real (e não monstro/NPC).",
                "param_descs": ["Alvo a testar (User)"]
            },
            "en": {
                "name": "IsPlayer (Check if Player)",
                "category": "Player",
                "desc": "Checks if the target is a real player (and not a monster/NPC).",
                "param_descs": ["Target to test (User)"]
            },
            "es": {
                "name": "IsPlayer (Verificar si es Jugador)",
                "category": "Jugador",
                "desc": "Comprueba si el objetivo es un jugador real (y no un monstruo/NPC).",
                "param_descs": ["Objetivo a comprobar (User)"]
            }
        }
    },
    {
        "id": "IsCreature",
        "template": "IsCreature ({Target})",
        "example": "IsCreature (Obj1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Target"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsCreature (Verificar se é Criatura)",
                "category": "Criaturas",
                "desc": "Verifica se o alvo é qualquer criatura viva (Jogador, Monstro ou NPC).",
                "param_descs": ["Alvo a testar"]
            },
            "en": {
                "name": "IsCreature (Check if Creature)",
                "category": "Creatures",
                "desc": "Checks if the target is any living creature (Player, Monster, or NPC).",
                "param_descs": ["Target to test"]
            },
            "es": {
                "name": "IsCreature (Verificar si es Criatura)",
                "category": "Criaturas",
                "desc": "Comprueba si el objetivo es cualquier criatura viva (Jugador, Monstruo o NPC).",
                "param_descs": ["Objetivo a comprobar"]
            }
        }
    },
    {
        "id": "HasFlag",
        "template": "HasFlag ({Target},{Flag})",
        "example": "HasFlag (Obj1,Container)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Flag", "type": "flag", "default": "Container"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasFlag (Verificar Flag do Item)",
                "category": "Itens & Tipos",
                "desc": "Verifica se o objeto possui uma flag específica do objects.srv ativa.",
                "param_descs": ["Alvo", "Nome da Flag (ex: Container, Unmove, Bank)"]
            },
            "en": {
                "name": "HasFlag (Check Item Flag)",
                "category": "Items & Types",
                "desc": "Checks if the object has a specific objects.srv flag active.",
                "param_descs": ["Target", "Flag Name (e.g. Container, Unmove, Bank)"]
            },
            "es": {
                "name": "HasFlag (Verificar Flag de Objeto)",
                "category": "Objetos y Tipos",
                "desc": "Comprueba si el objeto tiene una flag específica de objects.srv activa.",
                "param_descs": ["Objetivo", "Nombre de la Flag (ej: Container, Unmove, Bank)"]
            }
        }
    },
    {
        "id": "HasLevel",
        "template": "HasLevel ({User},{Level})",
        "example": "HasLevel (User,100)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Level", "type": "number", "default": 100}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasLevel (Verificar Level Mínimo)",
                "category": "Jogador",
                "desc": "Verifica se o jogador possui no mínimo o level requerido (muito usado em portas de level).",
                "param_descs": ["Jogador", "Level mínimo"]
            },
            "en": {
                "name": "HasLevel (Check Minimum Level)",
                "category": "Player",
                "desc": "Checks if the player meets the minimum required level (commonly used on level doors).",
                "param_descs": ["Player", "Minimum level"]
            },
            "es": {
                "name": "HasLevel (Verificar Nivel Mínimo)",
                "category": "Jugador",
                "desc": "Comprueba si el jugador tiene el nivel mínimo requerido (usado en puertas de nivel).",
                "param_descs": ["Jugador", "Nivel mínimo"]
            }
        }
    },
    {
        "id": "HasProfession",
        "template": "HasProfession ({User},{ProfessionID})",
        "example": "HasProfession (User,4)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "ProfessionID", "type": "number", "default": 4}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasProfession (Verificar Vocação)",
                "category": "Jogador",
                "desc": "Verifica a vocação do jogador (1=Sorcerer, 2=Druid, 3=Paladin, 4=Knight).",
                "param_descs": ["Jogador", "ID da vocação (1..4)"]
            },
            "en": {
                "name": "HasProfession (Check Vocation)",
                "category": "Player",
                "desc": "Checks the player's vocation (1=Sorcerer, 2=Druid, 3=Paladin, 4=Knight).",
                "param_descs": ["Player", "Vocation ID (1..4)"]
            },
            "es": {
                "name": "HasProfession (Verificar Vocación)",
                "category": "Jugador",
                "desc": "Comprueba la vocación del jugador (1=Sorcerer, 2=Druid, 3=Paladin, 4=Knight).",
                "param_descs": ["Jugador", "ID de la vocación (1..4)"]
            }
        }
    },
    {
        "id": "HasRight",
        "template": "HasRight ({User},{Right})",
        "example": "HasRight (User,PREMIUM_ACCOUNT)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Right", "type": "text", "default": "PREMIUM_ACCOUNT"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasRight (Verificar Direito/Privilégio)",
                "category": "Jogador",
                "desc": "Verifica se o jogador possui direito especial (ex: PREMIUM_ACCOUNT, GAMEMASTER).",
                "param_descs": ["Jogador", "Direito (ex: PREMIUM_ACCOUNT)"]
            },
            "en": {
                "name": "HasRight (Check Privilege/Right)",
                "category": "Player",
                "desc": "Checks if the player has a special privilege (e.g. PREMIUM_ACCOUNT, GAMEMASTER).",
                "param_descs": ["Player", "Privilege (e.g. PREMIUM_ACCOUNT)"]
            },
            "es": {
                "name": "HasRight (Verificar Derecho/Privilegio)",
                "category": "Jugador",
                "desc": "Comprueba si el jugador tiene un privilegio especial (ej: PREMIUM_ACCOUNT, GAMEMASTER).",
                "param_descs": ["Jugador", "Derecho (ej: PREMIUM_ACCOUNT)"]
            }
        }
    },
    {
        "id": "HasQuestValue",
        "template": "HasQuestValue ({User},{QuestNumber},{Value})",
        "example": "HasQuestValue (User,1000,1)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "QuestNumber", "type": "number", "default": 1000},
            {"name": "Value", "type": "number", "default": 1}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasQuestValue (Verificar Progresso de Quest)",
                "category": "Quests",
                "desc": "Verifica se o valor de storage/quest do jogador é igual ao especificado.",
                "param_descs": ["Jogador", "Número da Quest/Storage ID", "Valor esperado"]
            },
            "en": {
                "name": "HasQuestValue (Check Quest Progress)",
                "category": "Quests",
                "desc": "Checks if the player's storage/quest value matches the specified number.",
                "param_descs": ["Player", "Quest/Storage ID number", "Expected value"]
            },
            "es": {
                "name": "HasQuestValue (Verificar Progreso de Misión)",
                "category": "Quests",
                "desc": "Comprueba si el valor de storage/misión del jugador es igual al especificado.",
                "param_descs": ["Jugador", "Número de Quest/Storage ID", "Valor esperado"]
            }
        }
    },
    {
        "id": "MayLogout",
        "template": "MayLogout ({User})",
        "example": "MayLogout (User)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "MayLogout (Pode Deslogar)",
                "category": "Jogador",
                "desc": "Verifica se o jogador está livre de PZ lock/combate e pode deslogar (usado em camas).",
                "param_descs": ["Jogador"]
            },
            "en": {
                "name": "MayLogout (May Logout)",
                "category": "Player",
                "desc": "Checks if the player is free from PZ lock/combat and can log out (used in beds).",
                "param_descs": ["Player"]
            },
            "es": {
                "name": "MayLogout (Puede Desconectar)",
                "category": "Jugador",
                "desc": "Comprueba si el jugador está libre de combate/PZ y puede desconectarse (usado en camas).",
                "param_descs": ["Jugador"]
            }
        }
    },
    {
        "id": "IsHouse",
        "template": "IsHouse ({Target})",
        "example": "IsHouse (Obj1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsHouse (Pertence a uma Casa)",
                "category": "Casas",
                "desc": "Verifica se o item ou tile está dentro de uma casa registrada.",
                "param_descs": ["Alvo"]
            },
            "en": {
                "name": "IsHouse (Belongs to a House)",
                "category": "Houses",
                "desc": "Checks if the item or tile is inside a registered house.",
                "param_descs": ["Target"]
            },
            "es": {
                "name": "IsHouse (Pertenece a una Casa)",
                "category": "Casas",
                "desc": "Comprueba si el objeto o casilla está dentro de una casa registrada.",
                "param_descs": ["Objetivo"]
            }
        }
    },
    {
        "id": "IsHouseOwner",
        "template": "IsHouseOwner ({Target},{User})",
        "example": "IsHouseOwner (Obj1,User)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "User", "type": "target", "default": "User"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsHouseOwner (É o Dono da Casa)",
                "category": "Casas",
                "desc": "Verifica se o usuário é o proprietário legítimo da casa.",
                "param_descs": ["Objeto na casa", "Jogador"]
            },
            "en": {
                "name": "IsHouseOwner (Is House Owner)",
                "category": "Houses",
                "desc": "Checks if the user is the legitimate owner of the house.",
                "param_descs": ["Object in house", "Player"]
            },
            "es": {
                "name": "IsHouseOwner (Es el Dueño de la Casa)",
                "category": "Casas",
                "desc": "Comprueba si el usuario es el dueño legítimo de la casa.",
                "param_descs": ["Objeto en la casa", "Jugador"]
            }
        }
    },
    {
        "id": "IsProtectionZone",
        "template": "IsProtectionZone ({Target})",
        "example": "IsProtectionZone (Obj1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsProtectionZone (Zona de Proteção PZ)",
                "category": "Posição & Mapa",
                "desc": "Verifica se o local é uma Protection Zone.",
                "param_descs": ["Alvo"]
            },
            "en": {
                "name": "IsProtectionZone (Protection Zone PZ)",
                "category": "Position & Map",
                "desc": "Checks if the location is a Protection Zone.",
                "param_descs": ["Target"]
            },
            "es": {
                "name": "IsProtectionZone (Zona de Protección PZ)",
                "category": "Posición y Mapa",
                "desc": "Comprueba si el lugar es una Zona de Protección.",
                "param_descs": ["Objetivo"]
            }
        }
    },
    {
        "id": "IsObjectThere",
        "template": "IsObjectThere ({Position},{TypeID})",
        "example": "IsObjectThere ([32360,32199,7],2147)",
        "params_meta": [
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"},
            {"name": "TypeID", "type": "item_id", "default": 2147}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsObjectThere (Item Presente no Mapa)",
                "category": "Posição & Mapa",
                "desc": "Verifica se determinado TypeID está presente na coordenada [X, Y, Z].",
                "param_descs": ["Coordenada [X,Y,Z]", "ID do item"]
            },
            "en": {
                "name": "IsObjectThere (Item Present on Map)",
                "category": "Position & Map",
                "desc": "Checks if a specific TypeID exists at the [X, Y, Z] coordinate.",
                "param_descs": ["Coordinate [X,Y,Z]", "Item TypeID"]
            },
            "es": {
                "name": "IsObjectThere (Objeto Presente en el Mapa)",
                "category": "Posición y Mapa",
                "desc": "Comprueba si determinado TypeID está presente en la coordenada [X, Y, Z].",
                "param_descs": ["Coordenada [X,Y,Z]", "ID del objeto"]
            }
        }
    },
    {
        "id": "IsCreatureThere",
        "template": "IsCreatureThere ({Position})",
        "example": "IsCreatureThere ([32360,32199,7])",
        "params_meta": [
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsCreatureThere (Criatura Presente no Tile)",
                "category": "Posição & Mapa",
                "desc": "Verifica se há qualquer criatura na coordenada [X, Y, Z].",
                "param_descs": ["Coordenada [X,Y,Z]"]
            },
            "en": {
                "name": "IsCreatureThere (Creature Present on Tile)",
                "category": "Position & Map",
                "desc": "Checks if any creature is at the [X, Y, Z] coordinate.",
                "param_descs": ["Coordinate [X,Y,Z]"]
            },
            "es": {
                "name": "IsCreatureThere (Criatura Presente en la Casilla)",
                "category": "Posición y Mapa",
                "desc": "Comprueba si hay alguna criatura en la coordenada [X, Y, Z].",
                "param_descs": ["Coordenada [X,Y,Z]"]
            }
        }
    },
    {
        "id": "IsObjectInInventory",
        "template": "IsObjectInInventory ({User},{TypeID})",
        "example": "IsObjectInInventory (User,2000)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "TypeID", "type": "item_id", "default": 2000}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsObjectInInventory (Item no Inventário)",
                "category": "Jogador",
                "desc": "Verifica se o jogador carrega determinado TypeID no inventário.",
                "param_descs": ["Jogador", "ID do item"]
            },
            "en": {
                "name": "IsObjectInInventory (Item in Inventory)",
                "category": "Player",
                "desc": "Checks if the player carries the specified TypeID in their inventory.",
                "param_descs": ["Player", "Item TypeID"]
            },
            "es": {
                "name": "IsObjectInInventory (Objeto en Inventario)",
                "category": "Jugador",
                "desc": "Comprueba si el jugador lleva determinado TypeID en su inventario.",
                "param_descs": ["Jugador", "ID del objeto"]
            }
        }
    },
    {
        "id": "IsDressed",
        "template": "IsDressed ({User},{Slot},{TypeID})",
        "example": "IsDressed (User,4,2472)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Slot", "type": "number", "default": 1},
            {"name": "TypeID", "type": "item_id", "default": 2472}
        ],
        "i18n": {
            "pt_BR": {
                "name": "IsDressed (Equipado no Slot)",
                "category": "Jogador",
                "desc": "Verifica se o jogador está vestindo o TypeID no slot especificado.",
                "param_descs": ["Jogador", "Slot (1=Head, 4=Armor, 7=Legs, 8=Boots)", "ID do item"]
            },
            "en": {
                "name": "IsDressed (Equipped in Slot)",
                "category": "Player",
                "desc": "Checks if the player is wearing the specified TypeID in the given equipment slot.",
                "param_descs": ["Player", "Slot (1=Head, 4=Armor, 7=Legs, 8=Boots)", "Item TypeID"]
            },
            "es": {
                "name": "IsDressed (Equipado en Casilla/Slot)",
                "category": "Jugador",
                "desc": "Comprueba si el jugador está usando el TypeID en el slot especificado.",
                "param_descs": ["Jugador", "Slot (1=Head, 4=Armor, 7=Legs, 8=Boots)", "ID del objeto"]
            }
        }
    },
    {
        "id": "Random",
        "template": "Random ({Chance})",
        "example": "Random (50)",
        "params_meta": [
            {"name": "Chance", "type": "number", "default": 50}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Random (Probabilidade %)",
                "category": "Aleatório",
                "desc": "Aplica uma chance percentual (1 a 100) para a regra ser acionada.",
                "param_descs": ["Porcentagem de chance (1..100)"]
            },
            "en": {
                "name": "Random (Probability %)",
                "category": "Random",
                "desc": "Applies a percentage chance (1 to 100) for the rule to trigger.",
                "param_descs": ["Percentage chance (1..100)"]
            },
            "es": {
                "name": "Random (Probabilidad %)",
                "category": "Aleatorio",
                "desc": "Aplica una probabilidad porcentual (1 a 100) para activar la regla.",
                "param_descs": ["Porcentaje de probabilidad (1..100)"]
            }
        }
    },
    {
        "id": "HasText",
        "template": "HasText ({Target},\"{Text}\")",
        "example": "HasText (Obj1,\"Open Sesame\")",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Text", "type": "string", "default": "Open Sesame"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "HasText (Texto Específico)",
                "category": "Itens & Tipos",
                "desc": "Verifica se uma carta, placa ou livro possui o texto especificado.",
                "param_descs": ["Alvo", "Texto a buscar"]
            },
            "en": {
                "name": "HasText (Specific Text)",
                "category": "Items & Types",
                "desc": "Checks if a letter, sign, or book contains the specified text.",
                "param_descs": ["Target", "Text to search"]
            },
            "es": {
                "name": "HasText (Texto Específico)",
                "category": "Objetos y Tipos",
                "desc": "Comprueba si una carta, placa o libro contiene el texto especificado.",
                "param_descs": ["Objetivo", "Texto a buscar"]
            }
        }
    }
]

# =============================================================================
# CATÁLOGO DE AÇÕES / EFEITOS
# =============================================================================
ACTIONS_RAW = [
    {
        "id": "Change",
        "template": "Change({Target},{NewTypeID},{Data})",
        "example": "Change(Obj1,2495,0)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "NewTypeID", "type": "item_id", "default": 100},
            {"name": "Data", "type": "number", "default": 0}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Change (Transformar Item)",
                "category": "Transformação & Criação",
                "desc": "Transforma o item alvo em outro TypeID (ex: alavanca aberta -> fechada, porta trancada -> aberta).",
                "param_descs": ["Alvo a transformar", "Novo TypeID do item", "Subtipo/Data (geralmente 0)"]
            },
            "en": {
                "name": "Change (Transform Item)",
                "category": "Transformation & Creation",
                "desc": "Transforms the target item into another TypeID (e.g. open lever -> closed, locked door -> open).",
                "param_descs": ["Target to transform", "New item TypeID", "Subtype/Data (usually 0)"]
            },
            "es": {
                "name": "Change (Transformar Objeto)",
                "category": "Transformación y Creación",
                "desc": "Transforma el objeto objetivo en otro TypeID (ej: palanca abierta -> cerrada, puerta cerrada -> abierta).",
                "param_descs": ["Objetivo a transformar", "Nuevo TypeID del objeto", "Subtipo/Data (usualmente 0)"]
            }
        }
    },
    {
        "id": "ChangeRel",
        "template": "ChangeRel({Target},{Offset},{OldTypeID},{NewTypeID},{Data})",
        "example": "ChangeRel(Obj1,[0,1,0],2488,2496,0)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Offset", "type": "vector", "default": "[0,1,0]"},
            {"name": "OldTypeID", "type": "item_id", "default": 2488},
            {"name": "NewTypeID", "type": "item_id", "default": 2496},
            {"name": "Data", "type": "number", "default": 0}
        ],
        "i18n": {
            "pt_BR": {
                "name": "ChangeRel (Transformar Item Relativo)",
                "category": "Transformação & Criação",
                "desc": "Transforma um item localizado a um deslocamento relativo [dx, dy, dz] do alvo (ex: outra metade da cama ou porta dupla).",
                "param_descs": ["Alvo de referência", "Deslocamento [dx, dy, dz]", "TypeID antigo esperado", "Novo TypeID", "Subtipo/Data"]
            },
            "en": {
                "name": "ChangeRel (Transform Relative Item)",
                "category": "Transformation & Creation",
                "desc": "Transforms an item located at a relative offset [dx, dy, dz] from target (e.g. second bed half, double doors).",
                "param_descs": ["Reference target", "Offset [dx, dy, dz]", "Expected old TypeID", "New TypeID", "Subtype/Data"]
            },
            "es": {
                "name": "ChangeRel (Transformar Objeto Relativo)",
                "category": "Transformación y Creación",
                "desc": "Transforma un objeto situado en un desplazamiento relativo [dx, dy, dz] respecto al objetivo (ej: otra mitad de cama o puerta doble).",
                "param_descs": ["Objetivo de referencia", "Desplazamiento [dx, dy, dz]", "TypeID antiguo esperado", "Nuevo TypeID", "Subtipo/Data"]
            }
        }
    },
    {
        "id": "Create",
        "template": "Create({Target},{TypeID},{Amount})",
        "example": "Create(Obj1,2160,1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "TypeID", "type": "item_id", "default": 2160},
            {"name": "Amount", "type": "number", "default": 0}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Create (Criar Item no Alvo)",
                "category": "Transformação & Criação",
                "desc": "Cria um novo item na mesma posição do alvo.",
                "param_descs": ["Alvo de referência", "ID do item a criar", "Quantidade / Subtipo"]
            },
            "en": {
                "name": "Create (Create Item on Target)",
                "category": "Transformation & Creation",
                "desc": "Creates a new item at the target's position.",
                "param_descs": ["Reference target", "Item ID to create", "Amount / Subtype"]
            },
            "es": {
                "name": "Create (Crear Objeto en el Objetivo)",
                "category": "Transformación y Creación",
                "desc": "Crea un nuevo objeto en la misma posición del objetivo.",
                "param_descs": ["Objetivo de referencia", "ID del objeto a crear", "Cantidad / Subtipo"]
            }
        }
    },
    {
        "id": "CreateOnMap",
        "template": "CreateOnMap({Position},{TypeID},{Amount})",
        "example": "CreateOnMap([32360,32199,7],2160,1)",
        "params_meta": [
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"},
            {"name": "TypeID", "type": "item_id", "default": 2160},
            {"name": "Amount", "type": "number", "default": 0}
        ],
        "i18n": {
            "pt_BR": {
                "name": "CreateOnMap (Criar Item em Coordenada)",
                "category": "Transformação & Criação",
                "desc": "Cria um novo item nas coordenadas exatas [X, Y, Z] do mapa.",
                "param_descs": ["Coordenadas [X, Y, Z]", "ID do item a criar", "Quantidade"]
            },
            "en": {
                "name": "CreateOnMap (Create Item at Coordinate)",
                "category": "Transformation & Creation",
                "desc": "Creates a new item at exact [X, Y, Z] map coordinates.",
                "param_descs": ["Coordinates [X, Y, Z]", "Item ID to create", "Amount"]
            },
            "es": {
                "name": "CreateOnMap (Crear Objeto en Coordenada)",
                "category": "Transformación y Creación",
                "desc": "Crea un nuevo objeto en las coordenadas exactas [X, Y, Z] del mapa.",
                "param_descs": ["Coordenadas [X, Y, Z]", "ID del objeto a crear", "Cantidad"]
            }
        }
    },
    {
        "id": "Delete",
        "template": "Delete({Target})",
        "example": "Delete(Obj1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Delete (Destruir/Deletar Item)",
                "category": "Destruição",
                "desc": "Remove e deleta o item alvo do jogo (ex: consumir uma chave, poção usada, mobília desembalada).",
                "param_descs": ["Item a deletar"]
            },
            "en": {
                "name": "Delete (Destroy/Delete Item)",
                "category": "Destruction",
                "desc": "Removes and deletes the target item from the game (e.g. consume key, used potion, unpacked furniture).",
                "param_descs": ["Item to delete"]
            },
            "es": {
                "name": "Delete (Destruir/Eliminar Objeto)",
                "category": "Destrucción",
                "desc": "Elimina el objeto objetivo del juego (ej: consumir llave, poción usada, mueble desempaquetado).",
                "param_descs": ["Objeto a eliminar"]
            }
        }
    },
    {
        "id": "DeleteInInventory",
        "template": "DeleteInInventory({User},{TypeID},{Amount})",
        "example": "DeleteInInventory(User,2148,100)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "TypeID", "type": "item_id", "default": 2148},
            {"name": "Amount", "type": "number", "default": 1}
        ],
        "i18n": {
            "pt_BR": {
                "name": "DeleteInInventory (Remover do Inventário)",
                "category": "Destruição",
                "desc": "Remove itens da mochila/inventário do jogador.",
                "param_descs": ["Jogador", "ID do item", "Quantidade a remover"]
            },
            "en": {
                "name": "DeleteInInventory (Remove from Inventory)",
                "category": "Destruction",
                "desc": "Removes items from the player's backpack/inventory.",
                "param_descs": ["Player", "Item ID", "Amount to remove"]
            },
            "es": {
                "name": "DeleteInInventory (Eliminar del Inventario)",
                "category": "Destrucción",
                "desc": "Elimina objetos de la mochila/inventario del jugador.",
                "param_descs": ["Jugador", "ID del objeto", "Cantidad a eliminar"]
            }
        }
    },
    {
        "id": "Effect",
        "template": "Effect({Target},{EffectID})",
        "example": "Effect(Obj1,3)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "EffectID", "type": "number", "default": 3}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Effect (Efeito Mágico)",
                "category": "Efeitos & Mensagens",
                "desc": "Executa uma animação de efeito visual (ex: 3=Poof, 4=Flash, 15=Magic, 19=Fireworks, 22=Explosion).",
                "param_descs": ["Alvo para o efeito (Obj1, User)", "ID do Efeito (1..25)"]
            },
            "en": {
                "name": "Effect (Magic Effect)",
                "category": "Effects & Messages",
                "desc": "Plays a visual effect animation (e.g. 3=Poof, 4=Flash, 15=Magic, 19=Fireworks, 22=Explosion).",
                "param_descs": ["Target for effect (Obj1, User)", "Effect ID (1..25)"]
            },
            "es": {
                "name": "Effect (Efecto Mágico)",
                "category": "Efectos y Mensajes",
                "desc": "Ejecuta una animación de efecto visual (ej: 3=Poof, 4=Flash, 15=Magic, 19=Fireworks, 22=Explosion).",
                "param_descs": ["Objetivo para el efecto (Obj1, User)", "ID del Efecto (1..25)"]
            }
        }
    },
    {
        "id": "EffectOnMap",
        "template": "EffectOnMap({Position},{EffectID})",
        "example": "EffectOnMap([32360,32199,7],19)",
        "params_meta": [
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"},
            {"name": "EffectID", "type": "number", "default": 3}
        ],
        "i18n": {
            "pt_BR": {
                "name": "EffectOnMap (Efeito em Coordenada)",
                "category": "Efeitos & Mensagens",
                "desc": "Executa um efeito visual nas coordenadas [X, Y, Z] do mapa.",
                "param_descs": ["Coordenadas [X, Y, Z]", "ID do Efeito"]
            },
            "en": {
                "name": "EffectOnMap (Effect at Coordinate)",
                "category": "Effects & Messages",
                "desc": "Plays a visual effect animation at exact map coordinates [X, Y, Z].",
                "param_descs": ["Coordinates [X, Y, Z]", "Effect ID"]
            },
            "es": {
                "name": "EffectOnMap (Efecto en Coordenada)",
                "category": "Efectos y Mensajes",
                "desc": "Ejecuta un efecto visual en las coordenadas [X, Y, Z] del mapa.",
                "param_descs": ["Coordenadas [X, Y, Z]", "ID del Efecto"]
            }
        }
    },
    {
        "id": "Text",
        "template": "Text({Target},\"{Message}\",{Color})",
        "example": "Text(Obj1,\"Click!\",1)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Message", "type": "string", "default": "Click!"},
            {"name": "Color", "type": "number", "default": 1}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Text (Texto Animado)",
                "category": "Efeitos & Mensagens",
                "desc": "Exibe texto animado colorido flutuando sobre o alvo (ex: 'Hug me ^^', 'Click!').",
                "param_descs": ["Alvo do texto", "Mensagem a exibir", "Cor do texto (1=Branco, 2=Verde, etc.)"]
            },
            "en": {
                "name": "Text (Animated Floating Text)",
                "category": "Effects & Messages",
                "desc": "Displays colorful floating animated text over the target (e.g. 'Hug me ^^', 'Click!').",
                "param_descs": ["Target of text", "Message to display", "Text color (1=White, 2=Green, etc.)"]
            },
            "es": {
                "name": "Text (Texto Flotante Animado)",
                "category": "Efectos y Mensajes",
                "desc": "Muestra texto animado colorido flotando sobre el objetivo (ej: 'Hug me ^^', 'Click!').",
                "param_descs": ["Objetivo del texto", "Mensaje a mostrar", "Color del texto (1=Blanco, 2=Verde, etc.)"]
            }
        }
    },
    {
        "id": "MoveRel",
        "template": "MoveRel({User},{Target},{Offset})",
        "example": "MoveRel(User,Obj1,[0,0,0])",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "Offset", "type": "vector", "default": "[0,0,0]"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "MoveRel (Teleportar Relativamente)",
                "category": "Movimentação & Teleporte",
                "desc": "Teleporta o jogador ou objeto para uma posição com deslocamento [dx, dy, dz] relativo ao alvo.",
                "param_descs": ["Jogador a mover", "Objeto de referência", "Deslocamento [dx, dy, dz]"]
            },
            "en": {
                "name": "MoveRel (Relative Teleport)",
                "category": "Movement & Teleport",
                "desc": "Teleports the player or object with a relative offset [dx, dy, dz] from the target.",
                "param_descs": ["Player to move", "Reference object", "Offset [dx, dy, dz]"]
            },
            "es": {
                "name": "MoveRel (Teletransporte Relativo)",
                "category": "Movimiento y Teletransporte",
                "desc": "Teletransporta al jugador u objeto con un desplazamiento [dx, dy, dz] relativo al objetivo.",
                "param_descs": ["Jugador a mover", "Objeto de referencia", "Desplazamiento [dx, dy, dz]"]
            }
        }
    },
    {
        "id": "Move",
        "template": "Move({User},{Position})",
        "example": "Move(User,[32360,32199,7])",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Position", "type": "coord", "default": "[32000,32000,7]"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Move (Teleportar Absoluto)",
                "category": "Movimentação & Teleporte",
                "desc": "Move o jogador ou objeto para uma coordenada absoluta do mapa [X, Y, Z].",
                "param_descs": ["Jogador", "Destino [X, Y, Z]"]
            },
            "en": {
                "name": "Move (Absolute Teleport)",
                "category": "Movement & Teleport",
                "desc": "Moves the player or object to an absolute map coordinate [X, Y, Z].",
                "param_descs": ["Player", "Destination [X, Y, Z]"]
            },
            "es": {
                "name": "Move (Teletransporte Absoluto)",
                "category": "Movimiento y Teletransporte",
                "desc": "Mueve al jugador u objeto a una coordenada absoluta del mapa [X, Y, Z].",
                "param_descs": ["Jugador", "Destino [X, Y, Z]"]
            }
        }
    },
    {
        "id": "Monster",
        "template": "Monster({Target},\"{MonsterName}\")",
        "example": "Monster(Obj1,\"Demon\")",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "MonsterName", "type": "string", "default": "Demon"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Monster (Spawnar Monstro)",
                "category": "Transformação & Criação",
                "desc": "Invoca e spawna uma criatura na posição do alvo.",
                "param_descs": ["Alvo", "Nome do monstro (ex: Rat, Dragon, Demon)"]
            },
            "en": {
                "name": "Monster (Spawn Monster)",
                "category": "Transformation & Creation",
                "desc": "Summons and spawns a creature at the target's position.",
                "param_descs": ["Target", "Monster name (e.g. Rat, Dragon, Demon)"]
            },
            "es": {
                "name": "Monster (Generar Monstruo)",
                "category": "Transformación y Creación",
                "desc": "Invoca y genera una criatura en la posición del objetivo.",
                "param_descs": ["Objetivo", "Nombre del monstruo (ej: Rat, Dragon, Demon)"]
            }
        }
    },
    {
        "id": "SetQuestValue",
        "template": "SetQuestValue({User},{QuestNumber},{Value})",
        "example": "SetQuestValue(User,1000,1)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "QuestNumber", "type": "number", "default": 1000},
            {"name": "Value", "type": "number", "default": 1}
        ],
        "i18n": {
            "pt_BR": {
                "name": "SetQuestValue (Salvar Progresso de Quest)",
                "category": "Quests",
                "desc": "Grava um storage value / progresso de quest no jogador.",
                "param_descs": ["Jogador", "ID da Quest", "Novo valor"]
            },
            "en": {
                "name": "SetQuestValue (Save Quest Progress)",
                "category": "Quests",
                "desc": "Saves a storage value / quest progress on the player.",
                "param_descs": ["Player", "Quest ID", "New value"]
            },
            "es": {
                "name": "SetQuestValue (Guardar Progreso de Misión)",
                "category": "Quests",
                "desc": "Guarda un valor de storage / progreso de misión en el jugador.",
                "param_descs": ["Jugador", "ID de la Quest", "Nuevo valor"]
            }
        }
    },
    {
        "id": "Damage",
        "template": "Damage({Target},{DamageType},{MinDamage},{MaxDamage})",
        "example": "Damage(User,4,20,50)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "User"},
            {"name": "DamageType", "type": "number", "default": 4},
            {"name": "MinDamage", "type": "number", "default": 20},
            {"name": "MaxDamage", "type": "number", "default": 50}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Damage (Causar Dano)",
                "category": "Combate",
                "desc": "Causa dano direto ao jogador ou criatura (ex: armadilhas de fogo ou espinhos).",
                "param_descs": ["Alvo do dano", "Tipo (1=Physical, 2=Poison, 4=Fire, 8=Energy)", "Dano Mínimo", "Dano Máximo"]
            },
            "en": {
                "name": "Damage (Inflict Damage)",
                "category": "Combat",
                "desc": "Inflicts direct damage to the player or creature (e.g. fire fields or spike traps).",
                "param_descs": ["Target of damage", "Type (1=Physical, 2=Poison, 4=Fire, 8=Energy)", "Min Damage", "Max Damage"]
            },
            "es": {
                "name": "Damage (Infligir Daño)",
                "category": "Combate",
                "desc": "Inflige daño directo al jugador o criatura (ej: trampas de fuego o pinchos).",
                "param_descs": ["Objetivo del daño", "Tipo (1=Physical, 2=Poison, 4=Fire, 8=Energy)", "Daño Mínimo", "Daño Máximo"]
            }
        }
    },
    {
        "id": "WriteName",
        "template": "WriteName({User},\"{Format}\",{Target})",
        "example": "WriteName(User,\"%N\",Obj1)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"},
            {"name": "Format", "type": "string", "default": "%N"},
            {"name": "Target", "type": "target", "default": "Obj1"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "WriteName (Gravar Nome do Jogador)",
                "category": "Itens & Texto",
                "desc": "Grava o nome do jogador no item (ex: %N para camas).",
                "param_descs": ["Jogador", "Formato (%N = Nome)", "Objeto"]
            },
            "en": {
                "name": "WriteName (Write Player Name)",
                "category": "Items & Text",
                "desc": "Writes the player's name onto the item (e.g. %N for beds).",
                "param_descs": ["Player", "Format (%N = Name)", "Object"]
            },
            "es": {
                "name": "WriteName (Escribir Nombre del Jugador)",
                "category": "Objetos y Texto",
                "desc": "Escribe el nombre del jugador en el objeto (ej: %N para camas).",
                "param_descs": ["Jugador", "Formato (%N = Nombre)", "Objeto"]
            }
        }
    },
    {
        "id": "Logout",
        "template": "Logout({User})",
        "example": "Logout(User)",
        "params_meta": [
            {"name": "User", "type": "target", "default": "User"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Logout (Desconectar Jogador)",
                "category": "Jogador",
                "desc": "Executa o logout seguro do jogador (usado após dormir na cama).",
                "param_descs": ["Jogador"]
            },
            "en": {
                "name": "Logout (Player Logout)",
                "category": "Player",
                "desc": "Executes a safe player logout (used after sleeping in bed).",
                "param_descs": ["Player"]
            },
            "es": {
                "name": "Logout (Desconectar Jugador)",
                "category": "Jugador",
                "desc": "Ejecuta la desconexión segura del jugador (usado tras dormir en la cama).",
                "param_descs": ["Jugador"]
            }
        }
    },
    {
        "id": "Description",
        "template": "Description({Target},{User})",
        "example": "Description(Obj1,User)",
        "params_meta": [
            {"name": "Target", "type": "target", "default": "Obj1"},
            {"name": "User", "type": "target", "default": "User"}
        ],
        "i18n": {
            "pt_BR": {
                "name": "Description (Exibir Descrição)",
                "category": "Itens & Texto",
                "desc": "Mostra a descrição do item diretamente na tela do jogador (usado em portas de casas e placas).",
                "param_descs": ["Item", "Jogador"]
            },
            "en": {
                "name": "Description (Show Description)",
                "category": "Items & Text",
                "desc": "Displays the item description directly on the player's screen (used on house doors and signs).",
                "param_descs": ["Item", "Player"]
            },
            "es": {
                "name": "Description (Mostrar Descripción)",
                "category": "Objetos y Texto",
                "desc": "Muestra la descripción del objeto directamente en la pantalla del jugador (usado en puertas de casas y carteles).",
                "param_descs": ["Objeto", "Jugador"]
            }
        }
    }
]

def build_events_for_lang(lang: str = "pt_BR") -> List[Dict[str, Any]]:
    res = []
    for ev_id, ev_info in EVENTS_I18N.items():
        tr = ev_info.get(lang) or ev_info.get("pt_BR") or {}
        res.append({
            "id": ev_id,
            "name": tr.get("name", ev_id),
            "category": tr.get("category", "General"),
            "desc": tr.get("desc", ""),
            "example": tr.get("example", "")
        })
    return res

def build_conditions_for_lang(lang: str = "pt_BR") -> List[Dict[str, Any]]:
    res = []
    for item in CONDITIONS_RAW:
        tr = item.get("i18n", {}).get(lang) or item.get("i18n", {}).get("pt_BR") or {}
        param_descs = tr.get("param_descs", [])
        
        params = []
        for idx, pm in enumerate(item.get("params_meta", [])):
            desc_text = param_descs[idx] if idx < len(param_descs) else ""
            params.append({
                "name": pm["name"],
                "type": pm["type"],
                "default": pm["default"],
                "desc": desc_text
            })
            
        res.append({
            "id": item["id"],
            "name": tr.get("name", item["id"]),
            "category": tr.get("category", "General"),
            "desc": tr.get("desc", ""),
            "params": params,
            "template": item["template"],
            "example": item["example"]
        })
    return res

def build_actions_for_lang(lang: str = "pt_BR") -> List[Dict[str, Any]]:
    res = []
    for item in ACTIONS_RAW:
        tr = item.get("i18n", {}).get(lang) or item.get("i18n", {}).get("pt_BR") or {}
        param_descs = tr.get("param_descs", [])
        
        params = []
        for idx, pm in enumerate(item.get("params_meta", [])):
            desc_text = param_descs[idx] if idx < len(param_descs) else ""
            params.append({
                "name": pm["name"],
                "type": pm["type"],
                "default": pm["default"],
                "desc": desc_text
            })
            
        res.append({
            "id": item["id"],
            "name": tr.get("name", item["id"]),
            "category": tr.get("category", "General"),
            "desc": tr.get("desc", ""),
            "params": params,
            "template": item["template"],
            "example": item["example"]
        })
    return res

def get_full_catalog(lang: str = "pt_BR") -> Dict[str, Any]:
    if lang not in ["pt_BR", "en", "es"]:
        lang = "pt_BR"
    return {
        "lang": lang,
        "events": build_events_for_lang(lang),
        "conditions": build_conditions_for_lang(lang),
        "actions": build_actions_for_lang(lang)
    }

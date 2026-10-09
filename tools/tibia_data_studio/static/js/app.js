/**
 * Tibia Data Studio - Controlador Frontend SPA (Vanilla JavaScript)
 * Gerencia a lógica das abas de Itens, Monstros, NPCs, MoveUse, Sprites, Dicionário e Configurações.
 * Suporte completo a Internacionalização (i18n): Português do Brasil, English e Español.
 */

// =============================================================================
// DICIONÁRIO DE INTERNACIONALIZAÇÃO (i18n)
// =============================================================================
const I18N_DICT = {
  pt_BR: {
    // Sidebar
    nav_objects: "Itens (objects.srv)",
    nav_monsters: "Monstros (.mon)",
    nav_npcs: "NPCs (.npc)",
    nav_moveuse: "Ações (moveuse.dat)",
    nav_sprites: "Sprites (.spr / .dat)",
    nav_docs: "Dicionário de Funções",
    nav_settings: "Diretórios & Config",
    server_active: "Servidor Ativo",
    backup_active: "🛡️ Backups automáticos ativos",

    // Header
    search_placeholder: "Pesquisar por ID, Nome...",
    btn_reload: "🔄 Recarregar",
    btn_save_current: "💾 Salvar Atual",

    // Titles & Subtitles
    tab_objects_title: "Editor de Itens (objects.srv)",
    tab_objects_sub: "Gerenciamento completo de atributos, pesos, flags e sprites",
    tab_monsters_title: "Editor de Monstros (.mon)",
    tab_monsters_sub: "Edição de atributos, estatísticas, spells e tabela de loot",
    tab_npcs_title: "Editor de NPCs (.npc)",
    tab_npcs_sub: "Edição de aparência, posição home e diálogos",
    tab_moveuse_title: "Editor de Ações (moveuse.dat)",
    tab_moveuse_sub: "Regras de uso, portas, camas, transformações e interações",
    tab_sprites_title: "Navegador de Sprites (.spr / .dat)",
    tab_sprites_sub: "Inspeção e visualização de todos os gráficos do jogo",
    tab_docs_title: "Dicionário de Funções CipSoft",
    tab_docs_sub: "Guia completo de todos os Eventos, Condições e Ações da Move/Use Engine",
    tab_settings_title: "Diretórios & Configurações",
    tab_settings_sub: "Caminhos das pastas do servidor e do cliente Tibia",

    // Objects Tab
    objects_list_title: "Lista de Itens",
    btn_new_object: "+ Novo Item",
    all_flags: "Todas as Flags",
    obj_details_title: "Propriedades do Item",
    btn_save_object: "Salvar Item",
    btn_delete_object: "Excluir",
    btn_select_sprite: "Selecionar Sprite",
    lbl_obj_typeid: "Type ID (ID do Objeto)*",
    lbl_obj_name: "Nome do Item*",
    lbl_obj_desc: "Descrição (Look Description)",
    lbl_obj_comment: "Comentário Interno",
    lbl_numeric_attrs: "Atributos Comuns",
    lbl_weight: "Weight (oz * 100)",
    lbl_capacity: "Capacity (Slots)",
    lbl_attack: "Attack",
    lbl_defend: "Defense",
    lbl_armor: "Armor Value",
    lbl_nutrition: "Nutrition (Food)",
    lbl_waypoints: "Waypoints (Ground)",
    lbl_totalexpire: "Total Expire Time",
    lbl_raw_attrs: "Outros Atributos (JSON/Chave=Valor)",
    lbl_flags: "Flags do Item",
    btn_prev: "◀ Ant",
    btn_next: "Próx ▶",
    obj_title_edit: "Edição:",
    obj_title_create: "Criar Novo Item",
    obj_typeid_new: "Novo",
    obj_unnamed: "(Sem Nome)",
    lbl_items_count: "itens",
    lbl_page: "Página",

    // Monsters Tab
    monsters_list_title: "Monstros do Servidor",
    btn_new_monster: "+ Novo Monstro",
    btn_save_monster: "Salvar Monstro",
    btn_combat_guide: "💡 Guia de Cálculos & Dano",
    lbl_avg_melee: "⚔️ Melee Médio:",
    lbl_avg_armor: "🛡️ Absorção Armor:",
    lbl_flee_point: "🏃 Ponto de Fuga:",
    lbl_max_combo: "💥 Combo Máx Teórico:",
    lbl_mon_combat_sec: "⚔️ Combate & Atributos",
    lbl_mon_immunities_sec: "🛡️ Imunidades & Comportamento",
    lbl_mon_loot_sec: "💎 Tabela de Drop / Loot (Inventory)",
    lbl_mon_spells_sec: "✨ Magias e Ataques Mágicos (Spells)",
    lbl_mon_talk_sec: "💬 Falas e Sons em Combate (Talk)",
    lbl_mon_name: "Nome do Monstro*",
    lbl_mon_race: "RaceNumber*",
    lbl_mon_outfit: "Outfit LookType",
    lbl_mon_colors: "Cores (Head-Body-Legs-Feet)",
    lbl_mon_hp: "HitPoints (Vida)*",
    lbl_mon_exp: "Experience (XP)",
    lbl_mon_atk: "Attack (Dano Melee Máx)",
    lbl_mon_def: "Defend (Escudo/Defesa)",
    lbl_mon_armor: "Armor (Armadura)",
    lbl_mon_speed: "Speed (Velocidade)",
    lbl_mon_blood: "Blood (Tipo de Sangue)",
    lbl_mon_flee: "Flee Threshold (HP Fuga)",
    lbl_mon_corpse: "Corpse (ID do Corpo)",
    lbl_mon_poison: "Poison (Veneno Melee)",
    opt_blood_red: "Blood (Sangue Vermelho)",
    opt_blood_slime: "Slime (Gosma Verde)",
    opt_blood_undead: "Undead (Sem Sangue / Fumaça)",
    opt_blood_fire: "Fire (Fogo / Chamas)",
    opt_blood_energy: "Energy (Energia Azul)",
    btn_add_loot_item: "+ Adicionar Item de Loot",
    btn_add_monster_spell: "+ Nova Magia",
    btn_add_monster_talk: "+ Nova Fala",
    th_loot_item: "Item",
    th_loot_id: "ID",
    th_loot_count: "Qtd Máx",
    th_loot_chance: "Chance (Permilagem)",
    th_loot_percent: "Porcentagem",
    th_loot_action: "Ações",
    btn_remove: "Remover",
    th_spell_effect: "Efeito",
    th_spell_type: "Tipo / Forma",
    th_spell_details: "Detalhes",
    th_talk_mode: "Modo",
    th_talk_text: "Texto da fala",
    no_loot_registered: "Nenhum item dropado cadastrado",
    no_spells_registered: "Nenhuma magia cadastrada. Clique em '+ Nova Magia' para adicionar.",
    no_talk_registered: "Nenhuma fala cadastrada. Clique em '+ Nova Fala' para adicionar.",
    badge_yell: "📣 Grito (#Y)",
    badge_say: "💬 Fala (Say)",
    placeholder_talk: "Digite a fala do monstro...",
    mon_title_edit: "Edição:",
    mon_title_create: "Criar Novo Monstro",
    mon_race_new: "Novo",
    modal_pick_corpse: "Escolher Item de Corpo Morto (Corpse)",
    modal_pick_loot: "Adicionar Item ao Loot do Monstro",
    summary_combo_melee: "Melee",
    summary_combo_spells: "Magias",

    // Spell Types & Names
    dmg_name_physical: "Físico",
    dmg_name_fire: "Fogo",
    dmg_name_poison: "Veneno",
    dmg_name_energy: "Energia",
    dmg_name_death: "Morte/SD",
    dmg_name_manadrain: "Mana Drain",
    dmg_name_holy: "Holy",
    dmg_name_drown: "Afogamento",
    spell_lbl_damage: "Dano",
    spell_lbl_heal: "Auto-Cura",
    spell_lbl_summon: "Invocação: Raça",
    spell_lbl_minions: "lacaios",
    spell_lbl_turns: "Turnos",
    spell_lbl_to: "a",

    // Monster Spell Builder Modal
    modal_mon_spell_title: "✨ Criar / Editar Magia de Monstro",
    lbl_spell_impact: "Efeito da Magia (Impact)*",
    opt_impact_damage: "Damage (Dano Elementar / Físico / Morte / Mana)",
    opt_impact_healing: "Healing (Cura de Vida / Auto-Cura)",
    opt_impact_speed: "Speed (Paralyze / Haste)",
    opt_impact_summon: "Summon (Invocar Criaturas / Lacaios)",
    opt_impact_field: "Field (Criar Campo de Fogo/Veneno no Chão)",
    opt_impact_drunken: "Drunken (Embriaguez)",
    lbl_spell_shape: "Forma de Disparo / Alvo (Shape)*",
    opt_shape_origin: "Origin (Projétil em Linha Reta)",
    opt_shape_victim: "Victim (Alvo Travado na Vítima)",
    opt_shape_actor: "Actor (Em Si Mesmo / Self Buff)",
    opt_shape_destination: "Destination (Área no Chão / Bomba)",
    opt_shape_angle: "Angle (Wave / Cone Frontal)",
    lbl_spell_params_title: "🎯 Parâmetros de Dano e Efeito",
    lbl_spell_dmg_type: "Elemento / Tipo de Dano",
    opt_dmg_fire: "Fire (4 - Fogo / Chamas)",
    opt_dmg_energy: "Energy (16 - Energia / Raios)",
    opt_dmg_poison: "Poison (8 - Veneno / Terra)",
    opt_dmg_death: "Death / LifeDrain (32 - Morte / SD)",
    opt_dmg_physical: "Physical (1 - Físico / Impacto)",
    opt_dmg_manadrain: "ManaDrain (64 - Dreno de Mana)",
    opt_dmg_holy: "Holy (128 - Luz Sagrada)",
    opt_dmg_drown: "Drown (256 - Afogamento)",
    lbl_spell_dmg_min: "Dano Mínimo",
    lbl_spell_dmg_max: "Dano Máximo",
    lbl_spell_heal_min: "Cura Mínima",
    lbl_spell_heal_max: "Cura Máxima",
    lbl_spell_speed_val: "Alteração de Speed (ex: -50 ou +60)",
    lbl_spell_speed_dur: "Duração (Segundos)",
    lbl_spell_speed_eff: "Efeito de Speed (ID)",
    lbl_spell_summon_race: "RaceNumber do Monstro Invocado",
    lbl_spell_summon_count: "Quantidade Máxima Invocada",
    lbl_spell_field_item: "Item ID do Campo (ex: 1487 = Fire Field)",
    lbl_spell_drunk_int: "Intensidade (1 a 100)",
    lbl_spell_drunk_dur: "Duração (Segundos)",
    lbl_spell_drunk_eff: "Efeito (ID)",
    lbl_spell_visual_title: "🔮 Visual do Efeito, Projétil & Frequência",
    lbl_spell_magic_effect: "Efeito Mágico no Impacto",
    lbl_pick_spell_effect_btn: "✨ Escolher",
    lbl_spell_missile_effect: "Projétil / Míssil (Distance)",
    lbl_spell_delay: "Turnos / Frequência (Delay)*",
    lbl_mon_spell_code_title: "Linha Formatada do .mon:",
    btn_cancel_mon_spell: "Cancelar",
    btn_confirm_mon_spell: "Salvar Magia",

    // Monster Combat Guide & Simulator Modal
    combat_guide_title: "💡 Guia de Combate, Cálculos de Dano & Fórmulas (CipSoft 7.72)",
    sim_title_text: "🎯 Simulador Rápido de Dano Melee",
    sim_badge_text: "Baseado no C++",
    lbl_sim_mon_atk: "Ataque do Monstro (Attack):",
    lbl_sim_target_def: "Defesa do Alvo (Defend):",
    lbl_sim_target_armor: "Armadura do Alvo (Armor):",
    btn_run_damage_sim: "🎲 Simular 100 Ataques",
    sim_lbl_avg: "Dano Médio / Turno",
    sim_lbl_max: "Dano Máximo Causado",
    sim_lbl_block: "Taxa de Bloqueio (Poff)",
    sim_lbl_hits: "Golpes com Dano",
    btn_close_combat_guide_footer: "Fechar Guia",

    // NPCs Tab
    npcs_list_title: "NPCs do Servidor",
    btn_new_npc: "+ Novo NPC",
    btn_save_npc: "Salvar NPC",
    npc_details_title: "Edição de NPC",
    lbl_npc_name: "Nome*",
    lbl_npc_sex: "Sexo",
    opt_npc_sex_male: "Male (Masculino)",
    opt_npc_sex_female: "Female (Feminino)",
    lbl_npc_outfit: "LookType",
    lbl_npc_home: "Home Position [X, Y, Z]",
    lbl_npc_radius: "Raio (Radius)",
    lbl_npc_colors: "Cores (H-B-L-F)",
    lbl_npc_palette_sec: "🎨 Paleta de Cores do Outfit (Tibia Client)",
    tab_pal_head: "Head",
    tab_pal_primary: "Primary",
    tab_pal_secondary: "Secondary",
    tab_pal_detail: "Detail",
    btn_npc_random_colors: "🎲 Cores Aleatórias",
    btn_npc_reset_colors: "🔄 Reset (0-0-0-0)",
    btn_tab_npc_dialogues: "💬 Diálogos & Respostas",
    btn_tab_npc_ndb: "🛒 Comércio & Módulos (NDB)",
    btn_tab_npc_raw: "📝 Script Completo (.npc)",
    lbl_npc_greetings_title: "👋 Saudações, Foco & Despedida",
    lbl_npc_greeting: "Saudação (Player diz 'hi' / 'hello')",
    lbl_npc_farewell: "Despedida (Player diz 'bye' / 'farewell')",
    lbl_npc_busy: "Quando Ocupado com outro Jogador (BUSY)",
    lbl_npc_vanish: "Jogador Anda para Longe (VANISH)",
    lbl_npc_aggressive: "Fala Espontânea ao Ver Jogador (Sem Trigger / Agressivo)",
    lbl_npc_keywords_title: "💬 Palavras-Chave & Regras de Conversa",
    btn_add_npc_keyword: "+ Adicionar Regra",
    th_npc_keyword: "Palavras-Chave (Keywords)",
    th_npc_condition: "Condição / Topic",
    th_npc_response: "Resposta do NPC",
    th_npc_actions: "Ações / Efeitos",
    lbl_npc_ndb_modules: "📦 Módulos Oficiais CipSoft (.ndb)",
    lbl_npc_ndb_desc: "Marque os módulos pré-fabricados de comércio para incluir automaticamente no NPC:",
    lbl_npc_behaviour: "Código Completo de Behaviour ({ ... })",
    npc_title_edit: "Edição:",
    npc_title_create: "Criar Novo NPC",
    lbl_lines: "falas",

    // MoveUse Tab
    moveuse_sections_title: "Seções de Regras",
    moveuse_rules_title: "Regras da Seção",
    btn_add_rule: "+ Nova Regra",
    btn_save_moveuse: "Salvar moveuse.dat",
    moveuse_rules_of_section: "Regras da Seção:",
    lbl_rules: "regras",
    no_section_selected: "Nenhuma seção selecionada",

    // Sprites Tab
    sprites_panel_title: "Navegador de Sprites do Tibia.spr",
    sprites_search_placeholder: "Ir para Sprite ID...",
    btn_search: "Buscar",
    lbl_zoom_text: "Zoom:",
    btn_first: "Primeira",
    btn_prev_sprite: "Anterior",
    btn_next_sprite: "Próxima",
    btn_last: "Última",
    btn_go: "Ir",
    lbl_page_text: "Página",
    lbl_of: "de",
    lbl_showing_sprites: "Mostrando sprites",

    // CipSoft Function Dictionary Tab
    docs_panel_title: "📚 Dicionário & Guia de Funções CipSoft (Move/Use Engine)",
    docs_panel_subtitle: "Catálogo completo de todos os Eventos, Condições, Ações, Alvos e Parâmetros suportados pelo servidor.",
    doc_btn_all: "Todos",
    doc_btn_events: "Eventos (5)",
    doc_btn_conds: "Condições (19)",
    doc_btn_acts: "Ações / Efeitos (17)",
    docs_search_placeholder: "Pesquisar por nome de função, palavra-chave, exemplo (ex: Change, IsType, Level, Effect, Logout)...",
    badge_event_label: "Evento / Trigger",
    badge_condition_label: "Condição",
    badge_action_label: "Ação / Efeito",
    lbl_parameters: "Parâmetros:",
    lbl_example: "Exemplo:",

    // Rule Builder
    builder_modal_title: "⚡ Construtor de Evento & Regra CipSoft",
    lbl_builder_sec: "Seção no moveuse.dat (ex: Beds, Doors, Keys, Fun)*",
    lbl_builder_trigger: "Evento Disparador (Trigger)*",
    opt_trig_use: "Use (Clicar em 'Use' no item - Obj1)",
    opt_trig_multiuse: "MultiUse ('Use With' - Usar Obj1 em Obj2)",
    opt_trig_movement: "Movement (Arrastar item ou andar)",
    opt_trig_collision: "Collision (Pisar em cima de alavanca de piso/tile)",
    opt_trig_separation: "Separation (Sair de cima do tile)",
    lbl_builder_conds: "🔎 Condições Obrigatórias (Se acontecer isso...)",
    add_condition_opt: "+ Adicionar Condição...",
    lbl_builder_acts: "⚡ Ações e Efeitos Resultantes (Então faça isso...)",
    add_action_opt: "+ Adicionar Ação...",
    lbl_builder_raw: "Linha Formatada do moveuse.dat:",
    btn_cancel: "Cancelar",
    btn_confirm_rule: "Salvar Regra no moveuse.dat",
    choose_effect_btn: "✨ Escolher Efeito...",
    choose_item_btn: "📦 Escolher Item...",
    custom_chip: "Personalizado",
    no_cond_added: "Nenhuma condição adicionada (será executada sempre).",
    no_act_added: "Nenhuma ação adicionada.",
    modal_pick_cond_item: "Escolher Item para Condição",
    modal_pick_act_item: "Escolher Item para Ação",

    // Modals
    modal_picker_title: "Selecionar Item do objects.srv",
    modal_item_search: "Filtrar por nome ou ID...",
    modal_effect_picker_title: "✨ Selecionar Efeito Mágico CipSoft",
    modal_effect_search: "Filtrar por nome ou ID do efeito...",

    // Settings
    settings_panel_title: "Caminhos de Diretórios & Configuração",
    lbl_cfg_data_path: "Diretório de Dados do Servidor (tibia-game_data)",
    sub_cfg_data_path: "Contém as pastas <code>dat/</code>, <code>mon/</code>, <code>npc/</code>.",
    lbl_cfg_client_path: "Diretório do Cliente Tibia (com Tibia.spr e Tibia.dat)",
    sub_cfg_client_path: "Usado para extrair e renderizar todos os gráficos e sprites visuais.",
    lbl_cfg_status: "Status do Carregamento de Arquivos:",
    btn_save_config: "Salvar e Recarregar Dados",
    status_loaded: "Carregado",
    status_not_found: "Não encontrado",
    status_spr_missing: "Não carregado (Selecione a pasta do cliente)",
    status_types: "tipos",
    status_sections: "seções",
    status_creatures: "criaturas",
    status_npcs: "NPCs",
    status_sprites: "sprites",
    status_items: "itens",

    // Unsaved Changes (Dirty State)
    lbl_unsaved_text: "Edições pendentes",
    lbl_modal_unsaved_title: "⚠️ Alterações Não Salvas",
    lbl_modal_unsaved_msg: "Você possui edições pendentes que ainda não foram salvas. Deseja salvar as alterações agora antes de continuar ou descartá-las?",
    btn_unsaved_save: "💾 Salvar e Continuar",
    btn_unsaved_discard: "🗑️ Descartar",
    btn_unsaved_cancel: "Cancelar",

    // Toasts
    toast_reloading: "Recarregando dados...",
    toast_updated: "Dados atualizados com sucesso!",
    toast_item_saved: "Item salvo com sucesso!",
    toast_monster_saved: "Monstro salvo com sucesso!",
    toast_npc_saved: "NPC salvo com sucesso!",
    toast_moveuse_saved: "moveuse.dat salvo com sucesso!",
    toast_config_saved: "Configurações salvas e dados recarregados!",
    toast_sprite_copied: "Sprite ID copiado",
    toast_invalid_typeid: "Por favor, informe um TypeID válido!",
    toast_invalid_sprite_id: "Informe um Sprite ID válido!"
  },

  en: {
    // Sidebar
    nav_objects: "Items (objects.srv)",
    nav_monsters: "Monsters (.mon)",
    nav_npcs: "NPCs (.npc)",
    nav_moveuse: "Actions (moveuse.dat)",
    nav_sprites: "Sprites (.spr / .dat)",
    nav_docs: "Function Dictionary",
    nav_settings: "Directories & Config",
    server_active: "Server Active",
    backup_active: "🛡️ Automatic backups active",

    // Header
    search_placeholder: "Search by ID, Name...",
    btn_reload: "🔄 Reload",
    btn_save_current: "💾 Save Current",

    // Titles & Subtitles
    tab_objects_title: "Items Editor (objects.srv)",
    tab_objects_sub: "Complete management of attributes, weights, flags, and sprites",
    tab_monsters_title: "Monsters Editor (.mon)",
    tab_monsters_sub: "Edit attributes, stats, spells, and loot table",
    tab_npcs_title: "NPCs Editor (.npc)",
    tab_npcs_sub: "Edit appearance, home position, and dialogues",
    tab_moveuse_title: "Actions Editor (moveuse.dat)",
    tab_moveuse_sub: "Rules for use, doors, beds, transformations, and interactions",
    tab_sprites_title: "Sprites Browser (.spr / .dat)",
    tab_sprites_sub: "Inspection and preview of all game graphics",
    tab_docs_title: "CipSoft Function Dictionary",
    tab_docs_sub: "Complete guide to all Events, Conditions, and Actions of Move/Use Engine",
    tab_settings_title: "Directories & Settings",
    tab_settings_sub: "Paths to server and Tibia client folders",

    // Objects Tab
    objects_list_title: "Items List",
    btn_new_object: "+ New Item",
    all_flags: "All Flags",
    obj_details_title: "Item Properties",
    btn_save_object: "Save Item",
    btn_delete_object: "Delete",
    btn_select_sprite: "Select Sprite",
    lbl_obj_typeid: "Type ID (Object ID)*",
    lbl_obj_name: "Item Name*",
    lbl_obj_desc: "Description (Look Description)",
    lbl_obj_comment: "Internal Note / Comment",
    lbl_numeric_attrs: "Common Attributes",
    lbl_weight: "Weight (100 = 1.00 oz)",
    lbl_capacity: "Capacity (Slots)",
    lbl_attack: "Attack Value",
    lbl_defend: "Defense Value",
    lbl_armor: "Armor Value",
    lbl_nutrition: "Nutrition (Food)",
    lbl_waypoints: "Waypoints (Ground)",
    lbl_totalexpire: "Total Expire Time",
    lbl_raw_attrs: "Other Attributes (key=val, ...)",
    lbl_flags: "Item Flags",
    btn_prev: "◀ Prev",
    btn_next: "Next ▶",
    obj_title_edit: "Edit:",
    obj_title_create: "Create New Item",
    obj_typeid_new: "New",
    obj_unnamed: "(Unnamed)",
    lbl_items_count: "items",
    lbl_page: "Page",

    // Monsters Tab
    monsters_list_title: "Server Monsters",
    btn_new_monster: "+ New Monster",
    btn_save_monster: "Save Monster",
    btn_combat_guide: "💡 Combat & Damage Guide",
    lbl_avg_melee: "⚔️ Avg Melee:",
    lbl_avg_armor: "🛡️ Armor Absorption:",
    lbl_flee_point: "🏃 Flee Point:",
    lbl_max_combo: "💥 Max Combo Theory:",
    lbl_mon_combat_sec: "⚔️ Combat & Attributes",
    lbl_mon_immunities_sec: "🛡️ Immunities & Behavior",
    lbl_mon_loot_sec: "💎 Loot / Drop Table (Inventory)",
    lbl_mon_spells_sec: "✨ Spells & Magic Attacks",
    lbl_mon_talk_sec: "💬 Combat Talk & Sounds (Talk)",
    lbl_mon_name: "Monster Name*",
    lbl_mon_race: "RaceNumber*",
    lbl_mon_outfit: "Outfit LookType",
    lbl_mon_colors: "Colors (Head-Body-Legs-Feet)",
    lbl_mon_hp: "HitPoints (Health)*",
    lbl_mon_exp: "Experience (XP)",
    lbl_mon_atk: "Attack (Max Melee Damage)",
    lbl_mon_def: "Defend (Shield/Defense)",
    lbl_mon_armor: "Armor (Armor Value)",
    lbl_mon_speed: "Speed (Movement Speed)",
    lbl_mon_blood: "Blood (Blood Type)",
    lbl_mon_flee: "Flee Threshold (Flee HP)",
    lbl_mon_corpse: "Corpse (Corpse Item ID)",
    lbl_mon_poison: "Poison (Melee Poison)",
    opt_blood_red: "Blood (Red Blood)",
    opt_blood_slime: "Slime (Green Slime)",
    opt_blood_undead: "Undead (No Blood / Smoke)",
    opt_blood_fire: "Fire (Fire / Flames)",
    opt_blood_energy: "Energy (Blue Energy)",
    btn_add_loot_item: "+ Add Loot Item",
    btn_add_monster_spell: "+ New Spell",
    btn_add_monster_talk: "+ New Talk Line",
    th_loot_item: "Item",
    th_loot_id: "ID",
    th_loot_count: "Max Count",
    th_loot_chance: "Chance (‰)",
    th_loot_percent: "Percentage",
    th_loot_action: "Actions",
    btn_remove: "Remove",
    th_spell_effect: "Effect",
    th_spell_type: "Type / Shape",
    th_spell_details: "Details",
    th_talk_mode: "Mode",
    th_talk_text: "Speech text",
    no_loot_registered: "No dropped items registered",
    no_spells_registered: "No spells registered. Click '+ New Spell' to add one.",
    no_talk_registered: "No talk lines registered. Click '+ New Talk Line' to add one.",
    badge_yell: "📣 Yell (#Y)",
    badge_say: "💬 Talk (Say)",
    placeholder_talk: "Type monster talk line...",
    mon_title_edit: "Edit:",
    mon_title_create: "Create New Monster",
    mon_race_new: "New",
    modal_pick_corpse: "Select Corpse Item",
    modal_pick_loot: "Add Item to Monster Loot",
    summary_combo_melee: "Melee",
    summary_combo_spells: "Spells",

    // Spell Types & Names
    dmg_name_physical: "Physical",
    dmg_name_fire: "Fire",
    dmg_name_poison: "Poison",
    dmg_name_energy: "Energy",
    dmg_name_death: "Death/SD",
    dmg_name_manadrain: "Mana Drain",
    dmg_name_holy: "Holy",
    dmg_name_drown: "Drowning",
    spell_lbl_damage: "Damage",
    spell_lbl_heal: "Self-Heal",
    spell_lbl_summon: "Summon: Race",
    spell_lbl_minions: "minions",
    spell_lbl_turns: "Turns",
    spell_lbl_to: "to",

    // Monster Spell Builder Modal
    modal_mon_spell_title: "✨ Create / Edit Monster Spell",
    lbl_spell_impact: "Spell Effect (Impact)*",
    opt_impact_damage: "Damage (Elemental / Physical / Death / Mana)",
    opt_impact_healing: "Healing (Health Heal / Self-Heal)",
    opt_impact_speed: "Speed (Paralyze / Haste)",
    opt_impact_summon: "Summon (Summon Creatures / Minions)",
    opt_impact_field: "Field (Create Fire/Poison Field on Ground)",
    opt_impact_drunken: "Drunken (Drunkenness)",
    lbl_spell_shape: "Cast Shape / Target (Shape)*",
    opt_shape_origin: "Origin (Straight Line Projectile)",
    opt_shape_victim: "Victim (Locked on Target)",
    opt_shape_actor: "Actor (On Self / Self Buff)",
    opt_shape_destination: "Destination (Ground Area / Bomb)",
    opt_shape_angle: "Angle (Wave / Front Cone)",
    lbl_spell_params_title: "🎯 Damage and Effect Parameters",
    lbl_spell_dmg_type: "Damage Element / Type",
    opt_dmg_fire: "Fire (4 - Fire / Flames)",
    opt_dmg_energy: "Energy (16 - Energy / Lightning)",
    opt_dmg_poison: "Poison (8 - Poison / Earth)",
    opt_dmg_death: "Death / LifeDrain (32 - Death / SD)",
    opt_dmg_physical: "Physical (1 - Physical / Impact)",
    opt_dmg_manadrain: "ManaDrain (64 - Mana Drain)",
    opt_dmg_holy: "Holy (128 - Holy Light)",
    opt_dmg_drown: "Drown (256 - Drowning)",
    lbl_spell_dmg_min: "Min Damage",
    lbl_spell_dmg_max: "Max Damage",
    lbl_spell_heal_min: "Min Heal",
    lbl_spell_heal_max: "Max Heal",
    lbl_spell_speed_val: "Speed Change (e.g. -50 or +60)",
    lbl_spell_speed_dur: "Duration (Seconds)",
    lbl_spell_speed_eff: "Speed Effect (ID)",
    lbl_spell_summon_race: "Summoned Monster RaceNumber",
    lbl_spell_summon_count: "Max Summoned Count",
    lbl_spell_field_item: "Field Item ID (e.g. 1487 = Fire Field)",
    lbl_spell_drunk_int: "Intensity (1 to 100)",
    lbl_spell_drunk_dur: "Duration (Seconds)",
    lbl_spell_drunk_eff: "Effect (ID)",
    lbl_spell_visual_title: "🔮 Visual Effect, Projectile & Delay",
    lbl_spell_magic_effect: "Magic Effect on Impact",
    lbl_pick_spell_effect_btn: "✨ Choose",
    lbl_spell_missile_effect: "Distance Missile",
    lbl_spell_delay: "Delay / Turns Frequency*",
    lbl_mon_spell_code_title: "Formatted .mon line:",
    btn_cancel_mon_spell: "Cancel",
    btn_confirm_mon_spell: "Save Spell",

    // Monster Combat Guide & Simulator Modal
    combat_guide_title: "💡 Combat Guide, Damage Calculations & Formulas (CipSoft 7.72)",
    sim_title_text: "🎯 Quick Melee Damage Simulator",
    sim_badge_text: "Based on C++",
    lbl_sim_mon_atk: "Monster Attack (Attack):",
    lbl_sim_target_def: "Target Defense (Defend):",
    lbl_sim_target_armor: "Target Armor (Armor):",
    btn_run_damage_sim: "🎲 Simulate 100 Attacks",
    sim_lbl_avg: "Avg Damage / Turn",
    sim_lbl_max: "Max Damage Dealt",
    sim_lbl_block: "Block Rate (Poff)",
    sim_lbl_hits: "Damaging Hits",
    btn_close_combat_guide_footer: "Close Guide",

    // NPCs Tab
    npcs_list_title: "Server NPCs",
    btn_new_npc: "+ New NPC",
    btn_save_npc: "Save NPC",
    npc_details_title: "NPC Editor",
    lbl_npc_name: "Name*",
    lbl_npc_sex: "Sex",
    opt_npc_sex_male: "Male",
    opt_npc_sex_female: "Female",
    lbl_npc_outfit: "LookType",
    lbl_npc_home: "Home Position [X, Y, Z]",
    lbl_npc_radius: "Walk Radius",
    lbl_npc_colors: "Outfit Colors (H-B-L-F)",
    lbl_npc_palette_sec: "🎨 Outfit Color Palette (Tibia Client)",
    tab_pal_head: "Head",
    tab_pal_primary: "Primary",
    tab_pal_secondary: "Secondary",
    tab_pal_detail: "Detail",
    btn_npc_random_colors: "🎲 Random Colors",
    btn_npc_reset_colors: "🔄 Reset (0-0-0-0)",
    btn_tab_npc_dialogues: "💬 Dialogues & Responses",
    btn_tab_npc_ndb: "🛒 Shop & Modules (NDB)",
    btn_tab_npc_raw: "📝 Full Script (.npc)",
    lbl_npc_greetings_title: "👋 Greetings, Focus & Farewell",
    lbl_npc_greeting: "Greeting (Player says 'hi' / 'hello')",
    lbl_npc_farewell: "Farewell (Player says 'bye' / 'farewell')",
    lbl_npc_busy: "When Busy with another Player (BUSY)",
    lbl_npc_vanish: "Player Walks Away (VANISH)",
    lbl_npc_aggressive: "Auto On-Sight Talk (No Trigger / Aggressive)",
    lbl_npc_keywords_title: "💬 Custom Keywords & Dialogue Rules",
    btn_add_npc_keyword: "+ Add Rule",
    th_npc_keyword: "Keywords",
    th_npc_condition: "Condition / Topic",
    th_npc_response: "NPC Response",
    th_npc_actions: "Actions / Effects",
    lbl_npc_ndb_modules: "📦 Official CipSoft Modules (.ndb)",
    lbl_npc_ndb_desc: "Check the pre-built CipSoft trade modules to include automatically in this NPC:",
    lbl_npc_behaviour: "Full Behaviour Code ({ ... })",
    npc_title_edit: "Edit:",
    npc_title_create: "Create New NPC",
    lbl_lines: "lines",

    // MoveUse Tab
    moveuse_sections_title: "Rule Sections",
    moveuse_rules_title: "Section Rules",
    btn_add_rule: "+ New Rule",
    btn_save_moveuse: "Save moveuse.dat",
    moveuse_rules_of_section: "Rules of Section:",
    lbl_rules: "rules",
    no_section_selected: "No section selected",

    // Sprites Tab
    sprites_panel_title: "Tibia.spr Sprites Browser",
    sprites_search_placeholder: "Go to Sprite ID...",
    btn_search: "Search",
    lbl_zoom_text: "Zoom:",
    btn_first: "First",
    btn_prev_sprite: "Previous",
    btn_next_sprite: "Next",
    btn_last: "Last",
    btn_go: "Go",
    lbl_page_text: "Page",
    lbl_of: "of",
    lbl_showing_sprites: "Showing sprites",

    // CipSoft Function Dictionary Tab
    docs_panel_title: "📚 CipSoft Function Dictionary & Guide (Move/Use Engine)",
    docs_panel_subtitle: "Complete catalog of all Events, Conditions, Actions, Targets, and Parameters supported by the server.",
    doc_btn_all: "All",
    doc_btn_events: "Events (5)",
    doc_btn_conds: "Conditions (19)",
    doc_btn_acts: "Actions / Effects (17)",
    docs_search_placeholder: "Search by function name, keyword, example (e.g. Change, IsType, Level, Effect, Logout)...",
    badge_event_label: "Event / Trigger",
    badge_condition_label: "Condition",
    badge_action_label: "Action / Effect",
    lbl_parameters: "Parameters:",
    lbl_example: "Example:",

    // Rule Builder
    builder_modal_title: "⚡ CipSoft Event & Rule Builder",
    lbl_builder_sec: "Section in moveuse.dat (e.g., Beds, Doors, Keys, Fun)*",
    lbl_builder_trigger: "Trigger Event*",
    opt_trig_use: "Use (Click 'Use' on item - Obj1)",
    opt_trig_multiuse: "MultiUse ('Use With' - Use Obj1 on Obj2)",
    opt_trig_movement: "Movement (Drag item or walk on tile)",
    opt_trig_collision: "Collision (Step on floor lever/tile)",
    opt_trig_separation: "Separation (Step off tile)",
    lbl_builder_conds: "🔎 Required Conditions (If this happens...)",
    add_condition_opt: "+ Add Condition...",
    lbl_builder_acts: "⚡ Resulting Actions & Effects (Then do this...)",
    add_action_opt: "+ Add Action...",
    lbl_builder_raw: "Formatted moveuse.dat line:",
    btn_cancel: "Cancel",
    btn_confirm_rule: "Save Rule to moveuse.dat",
    choose_effect_btn: "✨ Choose Effect...",
    choose_item_btn: "📦 Choose Item...",
    custom_chip: "Custom",
    no_cond_added: "No conditions added (will always execute).",
    no_act_added: "No actions added.",
    modal_pick_cond_item: "Select Item for Condition",
    modal_pick_act_item: "Select Item for Action",

    // Modals
    modal_picker_title: "Select Item from objects.srv",
    modal_item_search: "Filter by name or ID...",
    modal_effect_picker_title: "✨ Select CipSoft Magic Effect",
    modal_effect_search: "Filter by effect name or ID...",

    // Settings
    settings_panel_title: "Directory Paths & Settings",
    lbl_cfg_data_path: "Server Data Directory (tibia-game_data)",
    sub_cfg_data_path: "Contains <code>dat/</code>, <code>mon/</code>, <code>npc/</code> folders.",
    lbl_cfg_client_path: "Tibia Client Directory (with Tibia.spr and Tibia.dat)",
    sub_cfg_client_path: "Used to extract and render all game graphics and sprites.",
    lbl_cfg_status: "File Loading Status:",
    btn_save_config: "Save & Reload Data",
    status_loaded: "Loaded",
    status_not_found: "Not found",
    status_spr_missing: "Not loaded (Select client folder)",
    status_types: "types",
    status_sections: "sections",
    status_creatures: "creatures",
    status_npcs: "NPCs",
    status_sprites: "sprites",
    status_items: "items",

    // Unsaved Changes (Dirty State)
    lbl_unsaved_text: "Unsaved changes",
    lbl_modal_unsaved_title: "⚠️ Unsaved Changes",
    lbl_modal_unsaved_msg: "You have unsaved edits. Would you like to save your changes before proceeding, or discard them?",
    btn_unsaved_save: "💾 Save & Proceed",
    btn_unsaved_discard: "🗑️ Discard",
    btn_unsaved_cancel: "Cancel",

    // Toasts
    toast_reloading: "Reloading data...",
    toast_updated: "Data updated successfully!",
    toast_item_saved: "Item saved successfully!",
    toast_monster_saved: "Monster saved successfully!",
    toast_npc_saved: "NPC saved successfully!",
    toast_moveuse_saved: "moveuse.dat saved successfully!",
    toast_config_saved: "Settings saved and data reloaded!",
    toast_sprite_copied: "Sprite ID copied",
    toast_invalid_typeid: "Please provide a valid TypeID!",
    toast_invalid_sprite_id: "Please provide a valid Sprite ID!"
  },

  es: {
    // Sidebar
    nav_objects: "Objetos (objects.srv)",
    nav_monsters: "Monstruos (.mon)",
    nav_npcs: "NPCs (.npc)",
    nav_moveuse: "Acciones (moveuse.dat)",
    nav_sprites: "Sprites (.spr / .dat)",
    nav_docs: "Diccionario de Funciones",
    nav_settings: "Directorios y Config",
    server_active: "Servidor Activo",
    backup_active: "🛡️ Copias de seguridad activas",

    // Header
    search_placeholder: "Buscar por ID, Nombre...",
    btn_reload: "🔄 Recargar",
    btn_save_current: "💾 Guardar Actual",

    // Titles & Subtitles
    tab_objects_title: "Editor de Objetos (objects.srv)",
    tab_objects_sub: "Gestión completa de atributos, pesos, flags y sprites",
    tab_monsters_title: "Editor de Monstruos (.mon)",
    tab_monsters_sub: "Edición de atributos, estadísticas, hechizos y tabla de botín",
    tab_npcs_title: "Editor de NPCs (.npc)",
    tab_npcs_sub: "Edición de apariencia, posición de origen y diálogos",
    tab_moveuse_title: "Editor de Acciones (moveuse.dat)",
    tab_moveuse_sub: "Reglas de uso, puertas, camas, transformaciones e interacciones",
    tab_sprites_title: "Navegador de Sprites (.spr / .dat)",
    tab_sprites_sub: "Inspección y previsualización de todos los gráficos del juego",
    tab_docs_title: "Diccionario de Funciones CipSoft",
    tab_docs_sub: "Guía completa de todos los Eventos, Condiciones y Acciones de Move/Use Engine",
    tab_settings_title: "Directorios y Configuración",
    tab_settings_sub: "Rutas de carpetas del servidor y del cliente de Tibia",

    // Objects Tab
    objects_list_title: "Lista de Objetos",
    btn_new_object: "+ Nuevo Objeto",
    all_flags: "Todas las Flags",
    obj_details_title: "Propiedades del Objeto",
    btn_save_object: "Guardar Objeto",
    btn_delete_object: "Eliminar",
    btn_select_sprite: "Seleccionar Sprite",
    lbl_obj_typeid: "Type ID (ID del Objeto)*",
    lbl_obj_name: "Nombre del Objeto*",
    lbl_obj_desc: "Descripción (Look Description)",
    lbl_obj_comment: "Comentario Interno / Nota",
    lbl_numeric_attrs: "Atributos Comunes",
    lbl_weight: "Peso (100 = 1.00 oz)",
    lbl_capacity: "Capacidad (Slots)",
    lbl_attack: "Valor de Ataque",
    lbl_defend: "Valor de Defensa",
    lbl_armor: "Valor de Armadura",
    lbl_nutrition: "Nutrición (Comida)",
    lbl_waypoints: "Waypoints (Suelo)",
    lbl_totalexpire: "Tiempo Total de Expiración",
    lbl_raw_attrs: "Otros Atributos (clave=valor, ...)",
    lbl_flags: "Flags del Objeto",
    btn_prev: "◀ Ant",
    btn_next: "Sig ▶",
    obj_title_edit: "Edición:",
    obj_title_create: "Crear Nuevo Objeto",
    obj_typeid_new: "Nuevo",
    obj_unnamed: "(Sin Nombre)",
    lbl_items_count: "objetos",
    lbl_page: "Página",

    // Monsters Tab
    monsters_list_title: "Monstruos del Servidor",
    btn_new_monster: "+ Nuevo Monstruo",
    btn_save_monster: "Guardar Monstruo",
    btn_combat_guide: "💡 Guía de Cálculos y Daño",
    lbl_avg_melee: "⚔️ Melee Medio:",
    lbl_avg_armor: "🛡️ Absorción Armor:",
    lbl_flee_point: "🏃 Punto de Huida:",
    lbl_max_combo: "💥 Combo Máx Teórico:",
    lbl_mon_combat_sec: "⚔️ Combate y Atributos",
    lbl_mon_immunities_sec: "🛡️ Inmunidades y Comportamiento",
    lbl_mon_loot_sec: "💎 Tabla de Drop / Botín (Inventory)",
    lbl_mon_spells_sec: "✨ Hechizos y Ataques Mágicos (Spells)",
    lbl_mon_talk_sec: "💬 Frases y Sonidos en Combate (Talk)",
    lbl_mon_name: "Nombre del Monstruo*",
    lbl_mon_race: "Número de Raza*",
    lbl_mon_outfit: "LookType del Outfit",
    lbl_mon_colors: "Colores (Head-Body-Legs-Feet)",
    lbl_mon_hp: "HitPoints (Vida)*",
    lbl_mon_exp: "Experiencia (XP)",
    lbl_mon_atk: "Ataque (Daño Melee Máx)",
    lbl_mon_def: "Defensa (Escudo/Defensa)",
    lbl_mon_armor: "Armadura (Valor de Armadura)",
    lbl_mon_speed: "Velocidad (Movimiento)",
    lbl_mon_blood: "Sangre (Tipo de Sangre)",
    lbl_mon_flee: "Umbral de Huida (HP Huida)",
    lbl_mon_corpse: "Cadáver (ID del Cuerpo)",
    lbl_mon_poison: "Veneno (Veneno Melee)",
    opt_blood_red: "Blood (Sangre Roja)",
    opt_blood_slime: "Slime (Baba Verde)",
    opt_blood_undead: "Undead (Sin Sangre / Humo)",
    opt_blood_fire: "Fire (Fuego / Llamas)",
    opt_blood_energy: "Energy (Energía Azul)",
    btn_add_loot_item: "+ Añadir Objeto al Botín",
    btn_add_monster_spell: "+ Nueva Magia",
    btn_add_monster_talk: "+ Nueva Frase",
    th_loot_item: "Objeto",
    th_loot_id: "ID",
    th_loot_count: "Cant. Máx",
    th_loot_chance: "Probabilidad (‰)",
    th_loot_percent: "Porcentaje",
    th_loot_action: "Acciones",
    btn_remove: "Eliminar",
    th_spell_effect: "Efecto",
    th_spell_type: "Tipo / Forma",
    th_spell_details: "Detalles",
    th_talk_mode: "Modo",
    th_talk_text: "Texto del habla",
    no_loot_registered: "Ningún objeto de loot registrado",
    no_spells_registered: "No hay magias registradas. Haz clic en '+ Nueva Magia' para añadir.",
    no_talk_registered: "No hay frases registradas. Haz clic en '+ Nueva Frase' para añadir.",
    badge_yell: "📣 Grito (#Y)",
    badge_say: "💬 Frase (Say)",
    placeholder_talk: "Escribe la frase del monstruo...",
    mon_title_edit: "Edición:",
    mon_title_create: "Crear Nuevo Monstruo",
    mon_race_new: "Nuevo",
    modal_pick_corpse: "Elegir Objeto de Cadáver (Corpse)",
    modal_pick_loot: "Añadir Objeto al Botín del Monstruo",
    summary_combo_melee: "Melee",
    summary_combo_spells: "Magias",

    // Spell Types & Names
    dmg_name_physical: "Físico",
    dmg_name_fire: "Fuego",
    dmg_name_poison: "Veneno",
    dmg_name_energy: "Energía",
    dmg_name_death: "Muerte/SD",
    dmg_name_manadrain: "Drenaje de Maná",
    dmg_name_holy: "Sagrado",
    dmg_name_drown: "Ahogamiento",
    spell_lbl_damage: "Daño",
    spell_lbl_heal: "Auto-Curación",
    spell_lbl_summon: "Invocación: Raza",
    spell_lbl_minions: "esbirros",
    spell_lbl_turns: "Turnos",
    spell_lbl_to: "a",

    // Monster Spell Builder Modal
    modal_mon_spell_title: "✨ Crear / Editar Magia de Monstruo",
    lbl_spell_impact: "Efecto de la Magia (Impact)*",
    opt_impact_damage: "Damage (Daño Elemental / Físico / Muerte / Maná)",
    opt_impact_healing: "Healing (Curación de Vida / Auto-Curación)",
    opt_impact_speed: "Speed (Parálisis / Celeridad)",
    opt_impact_summon: "Summon (Invocar Criaturas / Esbirros)",
    opt_impact_field: "Field (Crear Campo de Fuego/Veneno en el Suelo)",
    opt_impact_drunken: "Drunken (Embriaguez)",
    lbl_spell_shape: "Forma de Disparo / Objetivo (Shape)*",
    opt_shape_origin: "Origin (Proyectil en Línea Recta)",
    opt_shape_victim: "Victim (Fijado en la Víctima)",
    opt_shape_actor: "Actor (En Sí Mismo / Auto-Buff)",
    opt_shape_destination: "Destination (Área en Suelo / Bomba)",
    opt_shape_angle: "Angle (Wave / Cono Frontal)",
    lbl_spell_params_title: "🎯 Parâmetros de Daño y Efecto",
    lbl_spell_dmg_type: "Elemento / Tipo de Daño",
    opt_dmg_fire: "Fire (4 - Fuego / Llamas)",
    opt_dmg_energy: "Energy (16 - Energía / Rayos)",
    opt_dmg_poison: "Poison (8 - Veneno / Tierra)",
    opt_dmg_death: "Death / LifeDrain (32 - Muerte / SD)",
    opt_dmg_physical: "Physical (1 - Físico / Impacto)",
    opt_dmg_manadrain: "ManaDrain (64 - Drenaje de Maná)",
    opt_dmg_holy: "Holy (128 - Luz Sagrada)",
    opt_dmg_drown: "Drown (256 - Ahogamiento)",
    lbl_spell_dmg_min: "Daño Mínimo",
    lbl_spell_dmg_max: "Daño Máximo",
    lbl_spell_heal_min: "Curación Mínima",
    lbl_spell_heal_max: "Curación Máxima",
    lbl_spell_speed_val: "Cambio de Velocidad (ej: -50 o +60)",
    lbl_spell_speed_dur: "Duración (Segundos)",
    lbl_spell_speed_eff: "Efecto de Velocidad (ID)",
    lbl_spell_summon_race: "RaceNumber del Monstruo Invocado",
    lbl_spell_summon_count: "Cantidad Máxima Invocada",
    lbl_spell_field_item: "ID del Objeto de Campo (ej: 1487 = Fire Field)",
    lbl_spell_drunk_int: "Intensidad (1 a 100)",
    lbl_spell_drunk_dur: "Duración (Segundos)",
    lbl_spell_drunk_eff: "Efecto (ID)",
    lbl_spell_visual_title: "🔮 Efecto Visual, Proyectil y Frecuencia",
    lbl_spell_magic_effect: "Efecto Mágico en Impacto",
    lbl_pick_spell_effect_btn: "✨ Elegir",
    lbl_spell_missile_effect: "Proyectil / Misil (Distancia)",
    lbl_spell_delay: "Turnos / Frecuencia (Delay)*",
    lbl_mon_spell_code_title: "Línea formateada del .mon:",
    btn_cancel_mon_spell: "Cancelar",
    btn_confirm_mon_spell: "Guardar Magia",

    // Monster Combat Guide & Simulator Modal
    combat_guide_title: "💡 Guía de Combate, Cálculos de Daño y Fórmulas (CipSoft 7.72)",
    sim_title_text: "🎯 Simulador Rápido de Daño Melee",
    sim_badge_text: "Basado en C++",
    lbl_sim_mon_atk: "Ataque del Monstruo (Attack):",
    lbl_sim_target_def: "Defensa del Objetivo (Defend):",
    lbl_sim_target_armor: "Armadura del Objetivo (Armor):",
    btn_run_damage_sim: "🎲 Simular 100 Ataques",
    sim_lbl_avg: "Daño Medio / Turno",
    sim_lbl_max: "Daño Máximo Causado",
    sim_lbl_block: "Tasa de Bloqueo (Poff)",
    sim_lbl_hits: "Golpes con Daño",
    btn_close_combat_guide_footer: "Cerrar Guía",

    // NPCs Tab
    npcs_list_title: "NPCs del Servidor",
    btn_new_npc: "+ Nuevo NPC",
    btn_save_npc: "Guardar NPC",
    npc_details_title: "Edición de NPC",
    lbl_npc_name: "Nombre*",
    lbl_npc_sex: "Sexo",
    opt_npc_sex_male: "Male (Masculino)",
    opt_npc_sex_female: "Female (Femenino)",
    lbl_npc_outfit: "LookType",
    lbl_npc_home: "Posición Home [X, Y, Z]",
    lbl_npc_radius: "Radio de Movimiento",
    lbl_npc_colors: "Colores (H-B-L-F)",
    lbl_npc_palette_sec: "🎨 Paleta de Colores del Outfit (Tibia Client)",
    tab_pal_head: "Head",
    tab_pal_primary: "Primary",
    tab_pal_secondary: "Secondary",
    tab_pal_detail: "Detail",
    btn_npc_random_colors: "🎲 Colores Aleatorios",
    btn_npc_reset_colors: "🔄 Restablecer (0-0-0-0)",
    btn_tab_npc_dialogues: "💬 Diálogos y Respuestas",
    btn_tab_npc_ndb: "🛒 Comercio y Módulos (NDB)",
    btn_tab_npc_raw: "📝 Script Completo (.npc)",
    lbl_npc_greetings_title: "👋 Saludos, Enfoque y Despedida",
    lbl_npc_greeting: "Saludo (Jugador dice 'hi' / 'hello')",
    lbl_npc_farewell: "Despedida (Jugador dice 'bye' / 'farewell')",
    lbl_npc_busy: "Cuando Ocupado con otro Jugador (BUSY)",
    lbl_npc_vanish: "Jugador se Aleja (VANISH)",
    lbl_npc_aggressive: "Frase Espontánea al Ver Jugador (Sin Trigger / Agresivo)",
    lbl_npc_keywords_title: "💬 Palabras Clave y Regras de Conversación",
    btn_add_npc_keyword: "+ Añadir Regla",
    th_npc_keyword: "Palabras Clave (Keywords)",
    th_npc_condition: "Condición / Topic",
    th_npc_response: "Respuesta del NPC",
    th_npc_actions: "Acciones / Efectos",
    lbl_npc_ndb_modules: "📦 Módulos Oficiales CipSoft (.ndb)",
    lbl_npc_ndb_desc: "Marca los módulos prefabricados de comercio para incluir automáticamente en el NPC:",
    lbl_npc_behaviour: "Código Completo de Behaviour ({ ... })",
    npc_title_edit: "Edición:",
    npc_title_create: "Crear Nuevo NPC",
    lbl_lines: "líneas",

    // MoveUse Tab
    moveuse_sections_title: "Secciones de Reglas",
    moveuse_rules_title: "Reglas de la Sección",
    btn_add_rule: "+ Nueva Regla",
    btn_save_moveuse: "Guardar moveuse.dat",
    moveuse_rules_of_section: "Reglas de la Sección:",
    lbl_rules: "reglas",
    no_section_selected: "Ninguna sección seleccionada",

    // Sprites Tab
    sprites_panel_title: "Navegador de Sprites de Tibia.spr",
    sprites_search_placeholder: "Ir a Sprite ID...",
    btn_search: "Buscar",
    lbl_zoom_text: "Zoom:",
    btn_first: "Primera",
    btn_prev_sprite: "Anterior",
    btn_next_sprite: "Siguiente",
    btn_last: "Última",
    btn_go: "Ir",
    lbl_page_text: "Página",
    lbl_of: "de",
    lbl_showing_sprites: "Mostrando sprites",

    // CipSoft Function Dictionary Tab
    docs_panel_title: "📚 Diccionario y Guía de Funciones CipSoft (Move/Use Engine)",
    docs_panel_subtitle: "Catálogo completo de todos los Eventos, Condiciones, Acciones, Objetivos y Parámetros soportados por el servidor.",
    doc_btn_all: "Todos",
    doc_btn_events: "Eventos (5)",
    doc_btn_conds: "Condiciones (19)",
    doc_btn_acts: "Acciones / Efectos (17)",
    docs_search_placeholder: "Buscar por nombre de función, palabra clave, ejemplo (ej: Change, IsType, Level, Effect, Logout)...",
    badge_event_label: "Evento / Trigger",
    badge_condition_label: "Condición",
    badge_action_label: "Acción / Efecto",
    lbl_parameters: "Parámetros:",
    lbl_example: "Ejemplo:",

    // Rule Builder
    builder_modal_title: "⚡ Constructor de Eventos y Reglas CipSoft",
    lbl_builder_sec: "Sección en moveuse.dat (ej: Beds, Doors, Keys, Fun)*",
    lbl_builder_trigger: "Evento Disparador (Trigger)*",
    opt_trig_use: "Use (Clic en 'Use' en el objeto - Obj1)",
    opt_trig_multiuse: "MultiUse ('Use With' - Usar Obj1 en Obj2)",
    opt_trig_movement: "Movement (Arrastrar objeto o caminar)",
    opt_trig_collision: "Collision (Pisar sobre palanca de suelo/casilla)",
    opt_trig_separation: "Separation (Salir de encima de la casilla)",
    lbl_builder_conds: "🔎 Condiciones Obligatorias (Si ocurre esto...)",
    add_condition_opt: "+ Añadir Condición...",
    lbl_builder_acts: "⚡ Acciones y Efectos Resultantes (Entonces haz esto...)",
    add_action_opt: "+ Añadir Acción...",
    lbl_builder_raw: "Línea formateada de moveuse.dat:",
    btn_cancel: "Cancelar",
    btn_confirm_rule: "Guardar Regla en moveuse.dat",
    choose_effect_btn: "✨ Elegir Efecto...",
    choose_item_btn: "📦 Elegir Objeto...",
    custom_chip: "Personalizado",
    no_cond_added: "Ninguna condición añadida (se ejecutará siempre).",
    no_act_added: "Ninguna acción añadida.",
    modal_pick_cond_item: "Elegir Objeto para Condición",
    modal_pick_act_item: "Elegir Objeto para Acción",

    // Modals
    modal_picker_title: "Seleccionar Objeto de objects.srv",
    modal_item_search: "Filtrar por nombre o ID...",
    modal_effect_picker_title: "✨ Seleccionar Efecto Mágico CipSoft",
    modal_effect_search: "Filtrar por nombre o ID del efecto...",

    // Settings
    settings_panel_title: "Rutas de Directorios y Configuración",
    lbl_cfg_data_path: "Directorio de Datos del Servidor (tibia-game_data)",
    sub_cfg_data_path: "Contiene las carpetas <code>dat/</code>, <code>mon/</code>, <code>npc/</code>.",
    lbl_cfg_client_path: "Directorio del Cliente Tibia (con Tibia.spr y Tibia.dat)",
    sub_cfg_client_path: "Utilizado para extraer y renderizar todos los gráficos y sprites visuales.",
    lbl_cfg_status: "Estado de Carga de Archivos:",
    btn_save_config: "Guardar y Recargar Datos",
    status_loaded: "Cargado",
    status_not_found: "No encontrado",
    status_spr_missing: "No cargado (Seleccione la carpeta del cliente)",
    status_types: "tipos",
    status_sections: "secciones",
    status_creatures: "criaturas",
    status_npcs: "NPCs",
    status_sprites: "sprites",
    status_items: "objetos",

    // Unsaved Changes (Dirty State)
    lbl_unsaved_text: "Cambios sin guardar",
    lbl_modal_unsaved_title: "⚠️ Cambios sin Guardar",
    lbl_modal_unsaved_msg: "Tienes ediciones pendientes que aún no se han guardado. ¿Deseas guardar los cambios antes de continuar o descartarlos?",
    btn_unsaved_save: "💾 Guardar y Continuar",
    btn_unsaved_discard: "🗑️ Descartar",
    btn_unsaved_cancel: "Cancelar",

    // Toasts
    toast_reloading: "Recargando datos...",
    toast_updated: "¡Datos actualizados con éxito!",
    toast_item_saved: "¡Objeto guardado con éxito!",
    toast_monster_saved: "¡Monstruo guardado con éxito!",
    toast_npc_saved: "¡NPC guardado con éxito!",
    toast_moveuse_saved: "¡moveuse.dat guardado con éxito!",
    toast_config_saved: "¡Configuraciones guardadas y datos recargados!",
    toast_sprite_copied: "Sprite ID copiado",
    toast_invalid_typeid: "¡Por favor ingrese un TypeID válido!",
    toast_invalid_sprite_id: "¡Ingrese un Sprite ID válido!"
  }
};

// =============================================================================
// ESTADO GLOBAL DA APLICAÇÃO
// =============================================================================
const state = {
  lang: localStorage.getItem("tibia_studio_lang") || "pt_BR",
  currentTab: "tab-objects",
  isDirty: false,
  pendingNavigation: null,
  objects: {
    page: 1,
    limit: 40,
    search: "",
    flag: "",
    selectedTypeID: null,
    total: 0
  },
  monsters: {
    list: [],
    selectedFilename: null,
    search: ""
  },
  npcs: {
    list: [],
    selectedFilename: null,
    search: ""
  },
  moveuse: {
    sections: [],
    selectedSectionIndex: 0,
    search: ""
  },
  sprites: {
    page: 1,
    limit: 250,
    currentZoom: 1,
    total: 10962
  },
  docs: {
    catalog: { events: [], conditions: [], actions: [] },
    filter: "all",
    search: ""
  },
  builder: {
    editingSection: "",
    editingRuleIndex: null,
    eventType: "Use",
    conditions: [],
    actions: []
  },
  effects: [],
  config: {}
};

function t(key) {
  const dict = I18N_DICT[state.lang] || I18N_DICT["pt_BR"];
  return dict[key] || I18N_DICT["pt_BR"][key] || key;
}

function setDirty(isDirty) {
  state.isDirty = !!isDirty;
  const badge = document.getElementById("unsaved-changes-badge");
  if (badge) {
    badge.style.display = state.isDirty ? "inline-flex" : "none";
  }

  const baseTitle = "Tibia Data Studio";
  if (state.isDirty) {
    if (!document.title.startsWith("⚠️ * ")) {
      document.title = `⚠️ * ${document.title.replace(/^⚠️ \*\s*/, "") || baseTitle}`;
    }
    const saveBtn = document.getElementById("btn-global-save");
    if (saveBtn) saveBtn.classList.add("btn-dirty");
  } else {
    document.title = document.title.replace(/^⚠️ \*\s*/, "") || baseTitle;
    const saveBtn = document.getElementById("btn-global-save");
    if (saveBtn) saveBtn.classList.remove("btn-dirty");
  }
}

function confirmNavigation(onProceed) {
  if (!state.isDirty) {
    if (typeof onProceed === "function") onProceed();
    return;
  }
  state.pendingNavigation = onProceed;
  const modal = document.getElementById("unsaved-changes-modal");
  if (modal) modal.classList.add("active");
}

function setupUnsavedChangesModal() {
  const modal = document.getElementById("unsaved-changes-modal");
  const btnClose = document.getElementById("btn-close-unsaved-modal");
  const btnCancel = document.getElementById("btn-unsaved-cancel");
  const btnDiscard = document.getElementById("btn-unsaved-discard");
  const btnSave = document.getElementById("btn-unsaved-save");

  const closeModal = () => {
    if (modal) modal.classList.remove("active");
  };

  if (btnClose) {
    btnClose.addEventListener("click", () => {
      closeModal();
      state.pendingNavigation = null;
    });
  }

  if (btnCancel) {
    btnCancel.addEventListener("click", () => {
      closeModal();
      state.pendingNavigation = null;
    });
  }

  if (btnDiscard) {
    btnDiscard.addEventListener("click", () => {
      closeModal();
      setDirty(false);
      if (typeof state.pendingNavigation === "function") {
        const cb = state.pendingNavigation;
        state.pendingNavigation = null;
        cb();
      }
    });
  }

  if (btnSave) {
    btnSave.addEventListener("click", async () => {
      let success = false;
      if (state.currentTab === "tab-objects") {
        success = await saveCurrentObject();
      } else if (state.currentTab === "tab-monsters") {
        success = await saveCurrentMonster();
      } else if (state.currentTab === "tab-npcs") {
        success = await saveCurrentNpc();
      } else if (state.currentTab === "tab-moveuse") {
        success = await saveMoveUse();
      } else if (state.currentTab === "tab-settings") {
        const btnSaveCfg = document.getElementById("btn-save-config");
        if (btnSaveCfg) {
          btnSaveCfg.click();
          success = true;
        }
      } else {
        success = true;
      }

      if (success !== false) {
        closeModal();
        setDirty(false);
        if (typeof state.pendingNavigation === "function") {
          const cb = state.pendingNavigation;
          state.pendingNavigation = null;
          cb();
        }
      }
    });
  }

  window.addEventListener("beforeunload", (e) => {
    if (state.isDirty) {
      e.preventDefault();
      e.returnValue = "";
      return "";
    }
  });
}

function setupDirtyTracking() {
  const isExcluded = (target) => {
    if (!target) return true;
    const id = target.id || "";
    const excludedIds = [
      "global-search", "sprite-id-search", "docs-search-input",
      "modal-item-search", "modal-effect-search",
      "sim-target-def", "sim-target-armor", "sim-mon-atk",
      "app-language-select", "obj-flag-filter"
    ];
    if (excludedIds.includes(id)) return true;
    if (target.closest("#monster-combat-guide-modal")) return true;
    if (target.closest(".sidebar") || target.closest(".header")) return true;
    return false;
  };

  document.addEventListener("input", (e) => {
    if (isExcluded(e.target)) return;
    const inEditableTab = e.target.closest("#tab-objects, #tab-monsters, #tab-npcs, #tab-moveuse, #tab-settings, #rule-builder-modal, #monster-spell-modal");
    if (inEditableTab) {
      setDirty(true);
    }
  });

  document.addEventListener("change", (e) => {
    if (isExcluded(e.target)) return;
    const inEditableTab = e.target.closest("#tab-objects, #tab-monsters, #tab-npcs, #tab-moveuse, #tab-settings, #rule-builder-modal, #monster-spell-modal");
    if (inEditableTab) {
      setDirty(true);
    }
  });
}

async function setLanguage(lang) {
  if (!I18N_DICT[lang]) lang = "pt_BR";
  state.lang = lang;
  localStorage.setItem("tibia_studio_lang", lang);

  const langSelect = document.getElementById("app-language-select");
  if (langSelect) langSelect.value = lang;

  applyTranslationsToDOM();
  await loadDocs(); // Recarrega dicionário com o idioma selecionado
  
  if (state.currentTab === "tab-monsters") {
    if (state.monsters.selectedFilename) {
      const monName = document.getElementById("mon-name")?.value || "";
      setText("monster-title", `${t("mon_title_edit")} ${monName} (${state.monsters.selectedFilename})`);
    } else {
      setText("monster-title", t("mon_title_create"));
    }
    renderLootTable();
    renderMonsterSpellsList();
    renderMonsterTalkList();
    updateMonsterCombatSummary();
  }
  if (state.currentTab === "tab-settings") {
    renderConfigStatus();
  }
  if (state.currentTab === "tab-moveuse") {
    renderMoveUseSections();
    renderMoveUseRules();
  }
  if (state.currentTab === "tab-objects") {
    if (state.objects.selectedTypeID) {
      const objName = document.getElementById("obj-name")?.value || "";
      setText("lbl-obj-props-title", `${t("obj_title_edit")} ${objName} (ID: ${state.objects.selectedTypeID})`);
    } else {
      setText("lbl-obj-props-title", t("obj_title_create"));
    }
    loadObjects();
  }
  if (state.currentTab === "tab-npcs") {
    if (state.npcs.selectedFilename) {
      const npcName = document.getElementById("npc-name")?.value || "";
      setText("npc-title", `${t("npc_title_edit")} ${npcName} (${state.npcs.selectedFilename})`);
    } else {
      setText("npc-title", t("npc_title_create"));
    }
    renderNpcsList();
  }
  if (state.currentTab === "tab-sprites") {
    renderSpritesGrid();
  }
}

function escapeHtmlText(v) {
  return String(v == null ? "" : v)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");
}

function setText(id, text) {
  const el = document.getElementById(id);
  if (el) el.innerText = text;
}

function setHTML(id, html) {
  const el = document.getElementById(id);
  if (el) el.innerHTML = html;
}

function setPlaceholder(id, ph) {
  const el = document.getElementById(id);
  if (el) el.placeholder = ph;
}

function applyTranslationsToDOM() {
  const titles = {
    "tab-objects": { title: t("tab_objects_title"), sub: t("tab_objects_sub") },
    "tab-monsters": { title: t("tab_monsters_title"), sub: t("tab_monsters_sub") },
    "tab-npcs": { title: t("tab_npcs_title"), sub: t("tab_npcs_sub") },
    "tab-moveuse": { title: t("tab_moveuse_title"), sub: t("tab_moveuse_sub") },
    "tab-sprites": { title: t("tab_sprites_title"), sub: t("tab_sprites_sub") },
    "tab-docs": { title: t("tab_docs_title"), sub: t("tab_docs_sub") },
    "tab-settings": { title: t("tab_settings_title"), sub: t("tab_settings_sub") }
  };

  const curInfo = titles[state.currentTab] || { title: "Tibia Studio", sub: "" };
  setText("current-tab-title", curInfo.title);
  setText("current-tab-subtitle", curInfo.sub);

  // Header
  setPlaceholder("global-search", t("search_placeholder"));
  setText("btn-reload-data", t("btn_reload"));
  setText("btn-global-save", t("btn_save_current"));

  // Nav labels
  const navMap = [
    { tab: "tab-objects", key: "nav_objects" },
    { tab: "tab-monsters", key: "nav_monsters" },
    { tab: "tab-npcs", key: "nav_npcs" },
    { tab: "tab-moveuse", key: "nav_moveuse" },
    { tab: "tab-sprites", key: "nav_sprites" },
    { tab: "tab-docs", key: "nav_docs" },
    { tab: "tab-settings", key: "nav_settings" }
  ];
  navMap.forEach(({ tab, key }) => {
    const btn = document.querySelector(`.nav-item[data-tab='${tab}'] .nav-label`);
    if (btn) btn.innerText = t(key);
  });

  // Footer
  setText("server-status", t("server_active"));
  const bNote = document.querySelector(".backup-note");
  if (bNote) bNote.innerText = t("backup_active");

  // Objects Tab
  setText("lbl-objects-list-title", t("objects_list_title"));
  setText("btn-new-object", t("btn_new_object"));
  setText("opt-flag-all", t("all_flags"));
  setText("lbl-obj-props-title", t("obj_details_title"));
  setText("btn-delete-object", t("btn_delete_object"));
  setText("btn-save-object", t("btn_save_object"));
  setText("btn-open-sprite-picker", t("btn_select_sprite"));
  setText("lbl-obj-typeid", t("lbl_obj_typeid"));
  setText("lbl-obj-name", t("lbl_obj_name"));
  setText("lbl-obj-desc", t("lbl_obj_desc"));
  setText("lbl-obj-comment", t("lbl_obj_comment"));
  setText("lbl-numeric-attrs", t("lbl_numeric_attrs"));
  setText("lbl-weight", t("lbl_weight"));
  setText("lbl-capacity", t("lbl_capacity"));
  setText("lbl-attack", t("lbl_attack"));
  setText("lbl-defend", t("lbl_defend"));
  setText("lbl-armor", t("lbl_armor"));
  setText("lbl-nutrition", t("lbl_nutrition"));
  setText("lbl-waypoints", t("lbl_waypoints"));
  setText("lbl-totalexpire", t("lbl_totalexpire"));
  setText("lbl-raw-attrs", t("lbl_raw_attrs"));
  setText("lbl-flags", t("lbl_flags"));
  setText("btn-prev-page", t("btn_prev"));
  setText("btn-next-page", t("btn_next"));

  // Monsters Tab
  setText("lbl-monsters-list-title", t("monsters_list_title"));
  setText("btn-new-monster", t("btn_new_monster"));
  setText("btn-mon-combat-guide", t("btn_combat_guide"));
  setText("btn-save-monster", t("btn_save_monster"));
  setText("lbl-summary-avg-melee", t("lbl_avg_melee"));
  setText("lbl-summary-avg-armor", t("lbl_avg_armor"));
  setText("lbl-summary-flee-pct", t("lbl_flee_point"));
  setText("lbl-summary-max-combo", t("lbl_max_combo"));
  setText("lbl-mon-combat-sec", t("lbl_mon_combat_sec"));
  setText("lbl-mon-immunities-sec", t("lbl_mon_immunities_sec"));
  setText("lbl-mon-loot-sec", t("lbl_mon_loot_sec"));
  setText("lbl-mon-spells-sec", t("lbl_mon_spells_sec"));
  setText("lbl-mon-talk-sec", t("lbl_mon_talk_sec"));
  setText("th-spell-effect", t("th_spell_effect"));
  setText("th-spell-type", t("th_spell_type"));
  setText("th-spell-details", t("th_spell_details"));
  setText("th-spell-action", t("th_loot_action"));
  setText("th-talk-mode", t("th_talk_mode"));
  setText("th-talk-text", t("th_talk_text"));
  setText("th-talk-action", t("th_loot_action"));
  setText("lbl-mon-name", t("lbl_mon_name"));
  setText("lbl-mon-race", t("lbl_mon_race"));
  setText("lbl-mon-outfit", t("lbl_mon_outfit"));
  setText("lbl-mon-colors", t("lbl_mon_colors"));
  setText("lbl-mon-hp", t("lbl_mon_hp"));
  setText("lbl-mon-exp", t("lbl_mon_exp"));
  setText("lbl-mon-atk", t("lbl_mon_atk"));
  setText("lbl-mon-def", t("lbl_mon_def"));
  setText("lbl-mon-armor", t("lbl_mon_armor"));
  setText("lbl-mon-speed", t("lbl_mon_speed"));
  setText("lbl-mon-blood", t("lbl_mon_blood"));
  setText("lbl-mon-flee", t("lbl_mon_flee"));
  setText("lbl-mon-corpse", t("lbl_mon_corpse"));
  setText("lbl-mon-poison", t("lbl_mon_poison"));

  setText("opt-blood-red", t("opt_blood_red"));
  setText("opt-blood-slime", t("opt_blood_slime"));
  setText("opt-blood-undead", t("opt_blood_undead"));
  setText("opt-blood-fire", t("opt_blood_fire"));
  setText("opt-blood-energy", t("opt_blood_energy"));

  setText("th-loot-item", t("th_loot_item"));
  setText("th-loot-id", t("th_loot_id"));
  setText("th-loot-count", t("th_loot_count"));
  setText("th-loot-chance", t("th_loot_chance"));
  setText("th-loot-percent", t("th_loot_percent"));
  setText("th-loot-action", t("th_loot_action"));
  setText("btn-add-loot-item", t("btn_add_loot_item"));
  setText("btn-add-monster-spell", t("btn_add_monster_spell"));
  setText("btn-add-monster-talk", t("btn_add_monster_talk"));

  // Monster Spell Builder Modal
  setText("modal-mon-spell-title", t("modal_mon_spell_title"));
  setText("lbl-spell-impact", t("lbl_spell_impact"));
  setText("opt-impact-damage", t("opt_impact_damage"));
  setText("opt-impact-healing", t("opt_impact_healing"));
  setText("opt-impact-speed", t("opt_impact_speed"));
  setText("opt-impact-summon", t("opt_impact_summon"));
  setText("opt-impact-field", t("opt_impact_field"));
  setText("opt-impact-drunken", t("opt_impact_drunken"));
  setText("lbl-spell-shape", t("lbl_spell_shape"));
  setText("opt-shape-origin", t("opt_shape_origin"));
  setText("opt-shape-victim", t("opt_shape_victim"));
  setText("opt-shape-actor", t("opt_shape_actor"));
  setText("opt-shape-destination", t("opt_shape_destination"));
  setText("opt-shape-angle", t("opt_shape_angle"));
  setText("lbl-spell-params-title", t("lbl_spell_params_title"));
  setText("lbl-spell-dmg-type", t("lbl_spell_dmg_type"));
  setText("opt-dmg-fire", t("opt_dmg_fire"));
  setText("opt-dmg-energy", t("opt_dmg_energy"));
  setText("opt-dmg-poison", t("opt_dmg_poison"));
  setText("opt-dmg-death", t("opt_dmg_death"));
  setText("opt-dmg-physical", t("opt_dmg_physical"));
  setText("opt-dmg-manadrain", t("opt_dmg_manadrain"));
  setText("opt-dmg-holy", t("opt_dmg_holy"));
  setText("opt-dmg-drown", t("opt_dmg_drown"));
  setText("lbl-spell-dmg-min", t("lbl_spell_dmg_min"));
  setText("lbl-spell-dmg-max", t("lbl_spell_dmg_max"));
  setText("lbl-spell-heal-min", t("lbl_spell_heal_min"));
  setText("lbl-spell-heal-max", t("lbl_spell_heal_max"));
  setText("lbl-spell-speed-val", t("lbl_spell_speed_val"));
  setText("lbl-spell-speed-dur", t("lbl_spell_speed_dur"));
  setText("lbl-spell-speed-eff", t("lbl_spell_speed_eff"));
  setText("lbl-spell-summon-race", t("lbl_spell_summon_race"));
  setText("lbl-spell-summon-count", t("lbl_spell_summon_count"));
  setText("lbl-spell-field-item", t("lbl_spell_field_item"));
  setText("lbl-spell-drunk-int", t("lbl_spell_drunk_int"));
  setText("lbl-spell-drunk-dur", t("lbl_spell_drunk_dur"));
  setText("lbl-spell-drunk-eff", t("lbl_spell_drunk_eff"));
  setText("lbl-spell-visual-title", t("lbl_spell_visual_title"));
  setText("lbl-spell-magic-effect", t("lbl_spell_magic_effect"));
  setText("lbl-pick-spell-effect-btn", t("lbl_pick_spell_effect_btn"));
  setText("lbl-spell-missile-effect", t("lbl_spell_missile_effect"));
  setText("lbl-spell-delay", t("lbl_spell_delay"));
  setText("lbl-mon-spell-code-title", t("lbl_mon_spell_code_title"));
  setText("btn-cancel-mon-spell", t("btn_cancel_mon_spell"));
  setText("btn-confirm-mon-spell", t("btn_confirm_mon_spell"));

  // Combat Guide & Simulator Modal
  setText("combat-guide-title", t("combat_guide_title"));
  setText("sim-title-text", t("sim_title_text"));
  setText("sim-badge-text", t("sim_badge_text"));
  setText("lbl-sim-mon-atk", t("lbl_sim_mon_atk"));
  setText("lbl-sim-target-def", t("lbl_sim_target_def"));
  setText("lbl-sim-target-armor", t("lbl_sim_target_armor"));
  setText("btn-run-damage-sim", t("btn_run_damage_sim"));
  setText("sim-lbl-avg", t("sim_lbl_avg"));
  setText("sim-lbl-max", t("sim_lbl_max"));
  setText("sim-lbl-block", t("sim_lbl_block"));
  setText("sim-lbl-hits", t("sim_lbl_hits"));
  setText("btn-close-combat-guide-footer", t("btn_close_combat_guide_footer"));
  renderCombatGuideCards();

  // NPCs Tab
  setText("lbl-npcs-list-title", t("npcs_list_title"));
  setText("btn-new-npc", t("btn_new_npc"));
  setText("btn-save-npc", t("btn_save_npc"));
  setText("lbl-npc-name", t("lbl_npc_name"));
  setText("lbl-npc-sex", t("lbl_npc_sex"));
  setText("opt-npc-sex-male", t("opt_npc_sex_male"));
  setText("opt-npc-sex-female", t("opt_npc_sex_female"));
  setText("lbl-npc-outfit", t("lbl_npc_outfit"));
  setText("lbl-npc-home", t("lbl_npc_home"));
  setText("lbl-npc-radius", t("lbl_npc_radius"));
  setText("lbl-npc-colors", t("lbl_npc_colors"));
  setText("lbl-npc-palette-sec", t("lbl_npc_palette_sec"));
  setText("tab-pal-head", t("tab_pal_head"));
  setText("tab-pal-primary", t("tab_pal_primary"));
  setText("tab-pal-secondary", t("tab_pal_secondary"));
  setText("tab-pal-detail", t("tab_pal_detail"));
  setText("btn-npc-random-colors", t("btn_npc_random_colors"));
  setText("btn-npc-reset-colors", t("btn_npc_reset_colors"));
  setText("btn-tab-npc-dialogues", t("btn_tab_npc_dialogues"));
  setText("btn-tab-npc-ndb", t("btn_tab_npc_ndb"));
  setText("btn-tab-npc-raw", t("btn_tab_npc_raw"));
  setText("lbl-npc-greetings-title", t("lbl_npc_greetings_title"));
  setText("lbl-npc-greeting", t("lbl_npc_greeting"));
  setText("lbl-npc-farewell", t("lbl_npc_farewell"));
  setText("lbl-npc-busy", t("lbl_npc_busy"));
  setText("lbl-npc-vanish", t("lbl_npc_vanish"));
  setText("lbl-npc-aggressive", t("lbl_npc_aggressive"));
  setText("lbl-npc-keywords-title", t("lbl_npc_keywords_title"));
  setText("btn-add-npc-keyword", t("btn_add_npc_keyword"));
  setText("th-npc-keyword", t("th_npc_keyword"));
  setText("th-npc-condition", t("th_npc_condition"));
  setText("th-npc-response", t("th_npc_response"));
  setText("th-npc-actions", t("th_npc_actions"));
  setText("lbl-npc-ndb-modules", t("lbl_npc_ndb_modules"));
  setText("lbl-npc-ndb-desc", t("lbl_npc_ndb_desc"));
  setText("lbl-npc-behaviour", t("lbl_npc_behaviour"));

  // MoveUse Tab
  setText("lbl-moveuse-sec-title", t("moveuse_sections_title"));
  setText("moveuse-section-title", t("moveuse_rules_title"));
  setText("btn-add-moveuse-rule", t("btn_add_rule"));
  setText("btn-save-moveuse", t("btn_save_moveuse"));

  // Sprites Tab
  setText("sprites-panel-title", t("sprites_panel_title"));
  setPlaceholder("sprite-id-search", t("sprites_search_placeholder"));
  setText("btn-go-sprite", t("btn_search"));
  setText("lbl-zoom-text", t("lbl_zoom_text"));
  setHTML("btn-sprites-first", `⏮️ ${t("btn_first")}`);
  setHTML("btn-sprites-prev", `◀️ ${t("btn_prev_sprite")}`);
  setHTML("btn-sprites-next", `${t("btn_next_sprite")} ▶️`);
  setHTML("btn-sprites-last", `${t("btn_last")} ⏭️`);
  setText("lbl-page-text", t("lbl_page_text"));
  setText("btn-sprites-jump", t("btn_go"));

  // Function Dictionary Tab
  setText("docs-panel-title", t("docs_panel_title"));
  setText("docs-panel-subtitle", t("docs_panel_subtitle"));
  setText("doc-btn-all", t("doc_btn_all"));
  setText("doc-btn-events", t("doc_btn_events"));
  setText("doc-btn-conds", t("doc_btn_conds"));
  setText("doc-btn-acts", t("doc_btn_acts"));
  setPlaceholder("docs-search-input", t("docs_search_placeholder"));

  // Event Builder Modal
  setText("builder-modal-title", t("builder_modal_title"));
  setText("lbl-builder-sec", t("lbl_builder_sec"));
  setText("lbl-builder-trigger", t("lbl_builder_trigger"));
  setText("opt-trig-use", t("opt_trig_use"));
  setText("opt-trig-multiuse", t("opt_trig_multiuse"));
  setText("opt-trig-movement", t("opt_trig_movement"));
  setText("opt-trig-collision", t("opt_trig_collision"));
  setText("opt-trig-separation", t("opt_trig_separation"));
  setText("lbl-builder-conds", t("lbl_builder_conds"));
  setText("lbl-builder-acts", t("lbl_builder_acts"));
  setText("lbl-builder-raw", t("lbl_builder_raw"));
  setText("btn-cancel-builder", t("btn_cancel"));
  setText("btn-confirm-rule", t("btn_confirm_rule"));

  // Pickers
  setText("modal-picker-title", t("modal_picker_title"));
  setPlaceholder("modal-item-search", t("modal_item_search"));
  setText("modal-effect-picker-title", t("modal_effect_picker_title"));
  setPlaceholder("modal-effect-search", t("modal_effect_search"));

  // Settings
  setText("settings-panel-title", t("settings_panel_title"));
  setText("lbl-cfg-data-path", t("lbl_cfg_data_path"));
  setHTML("sub-cfg-data-path", t("sub_cfg_data_path"));
  setText("lbl-cfg-client-path", t("lbl_cfg_client_path"));
  setText("sub-cfg-client-path", t("sub_cfg_client_path"));
  setText("lbl-cfg-status", t("lbl_cfg_status"));
  setText("btn-save-config", t("btn_save_config"));

  // Dirty State / Modal Alterações Não Salvas
  setText("lbl-unsaved-text", t("lbl_unsaved_text"));
  setText("lbl-modal-unsaved-title", t("lbl_modal_unsaved_title"));
  setText("lbl-modal-unsaved-msg", t("lbl_modal_unsaved_msg"));
  setText("btn-unsaved-save", t("btn_unsaved_save"));
  setText("btn-unsaved-discard", t("btn_unsaved_discard"));
  setText("btn-unsaved-cancel", t("btn_unsaved_cancel"));

  // Re-render components with translated texts
  if (state.currentTab === "tab-sprites") renderSpritesGrid();
  if (state.currentTab === "tab-docs") renderDocsGrid();
}

function renderCombatGuideCards() {
  const container = document.getElementById("combat-guide-grid");
  if (!container) return;

  const lang = state.lang || "pt_BR";

  if (lang === "en") {
    container.innerHTML = `
      <div class="guide-topic-card">
        <h4>⚔️ 1. Melee Attack & Physical Damage</h4>
        <p>Every combat round (<strong>~2 seconds</strong>), the monster performs a physical attack against its target:</p>
        <div class="formula-box">
          <code>Raw_Damage = random(0, Attack)</code>
        </div>
        <p>Next, the target's <strong>Defense (Defend)</strong> value tries to block the blow:</p>
        <div class="formula-box">
          <code>Damage_After_Shield = max(0, Raw_Damage - Target_Defense)</code>
        </div>
        <p>If <code>Damage_After_Shield > 0</code>, the target's <strong>Armor</strong> absorbs part of the impact:</p>
        <div class="formula-box">
          <code>Armor_Absorption = (Armor / 2) + random(0, Armor / 2)</code><br>
          <code>Final_Damage = max(0, Damage_After_Shield - Armor_Absorption)</code>
        </div>
      </div>

      <div class="guide-topic-card">
        <h4>🛡️ 2. Defense & Resistances</h4>
        <ul>
          <li><strong>Defend:</strong> Acts as shield defense. Blocks physical damage from up to 2 attackers per round.</li>
          <li><strong>Armor:</strong> Absorbs residual damage. On average absorbs <strong>75%</strong> of Armor value on each physical hit.</li>
          <li><strong>Poison:</strong> If <code>Poison > 0</code>, when landing a physical hit the monster inflicts poison damage from <code>Poison/2</code> to <code>Poison</code> per second.</li>
          <li><strong>Blood:</strong> Type of blood emitted (Blood = Red, Slime = Green, Undead = Smoke, Fire = Flames, Energy = Blue).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>✨ 3. Spells & Ranged Magic Attacks</h4>
        <p>Spells formatted as <code>Shape -> Impact : Delay</code> are evaluated each turn:</p>
        <ul>
          <li><strong>Shape:</strong> How the spell is cast:
            <br>• <code>Origin(Missile, Effect)</code>: Straight line projectile.
            <br>• <code>Victim(Missile, Range, Effect)</code>: Locked-on target projectile.
            <br>• <code>Actor(Effect)</code>: Centered on monster (Self Buff / Heal).
            <br>• <code>Destination(...)</code>: Ground area / Bomb.
            <br>• <code>Angle(...)</code>: Frontal cone / wave (e.g. Dragon Wave).
          </li>
          <li><strong>Impact:</strong> The resulting effect (<code>Damage(Type, Max, Min)</code>, <code>Healing</code>, <code>Speed/Paralyze</code>, <code>Summon</code>, <code>Field</code>).</li>
          <li><strong>Delay (Turns):</strong> Spell frequency. <code>: 2</code> = 50% chance/turn (~every 4s), <code>: 6</code> = ~16% chance/turn (~every 12s).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>💬 4. Monster Talk Lines & Yells</h4>
        <ul>
          <li><strong>Regular Talk (Say):</strong> Appears in yellow over the creature within normal screen vision.</li>
          <li><strong>Yell (#Y):</strong> Starts with <code>#Y </code>. Appears in <strong>red</strong> to all players across the full game screen (e.g. <code>#Y GROOAAARRR!</code>).</li>
        </ul>
      </div>
    `;
  } else if (lang === "es") {
    container.innerHTML = `
      <div class="guide-topic-card">
        <h4>⚔️ 1. Ataque Melee y Daño Físico</h4>
        <p>En cada ronda de combate (<strong>~2 segundos</strong>), el monstruo realiza un ataque físico contra su objetivo:</p>
        <div class="formula-box">
          <code>Daño_Bruto = random(0, Attack)</code>
        </div>
        <p>A continuación, el valor de <strong>Defensa (Defend)</strong> del objetivo intenta bloquear el golpe:</p>
        <div class="formula-box">
          <code>Daño_Tras_Escudo = max(0, Daño_Bruto - Defensa_Objetivo)</code>
        </div>
        <p>Si <code>Daño_Tras_Escudo > 0</code>, la <strong>Armadura (Armor)</strong> del objetivo absorbe parte del impacto:</p>
        <div class="formula-box">
          <code>Absorcion_Armor = (Armor / 2) + random(0, Armor / 2)</code><br>
          <code>Daño_Final = max(0, Daño_Tras_Escudo - Absorcion_Armor)</code>
        </div>
      </div>

      <div class="guide-topic-card">
        <h4>🛡️ 2. Defensa y Resistencias</h4>
        <ul>
          <li><strong>Defend:</strong> Actúa como escudo. Bloquea el daño físico de hasta 2 atacantes por turno.</li>
          <li><strong>Armor:</strong> Absorbe daño residual. En promedio absorbe el <strong>75%</strong> del valor de Armor en cada golpe físico.</li>
          <li><strong>Poison:</strong> Si <code>Poison > 0</code>, al acertar un golpe físico el monstruo aplica veneno de <code>Poison/2</code> a <code>Poison</code> por segundo.</li>
          <li><strong>Blood:</strong> Tipo de sangre emitida (Blood = Roja, Slime = Verde, Undead = Humo, Fire = Fuego, Energy = Azul).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>✨ 3. Hechizos y Ataques a Distancia (Spells)</h4>
        <p>Los hechizos en formato <code>Shape -> Impact : Delay</code> se evalúan en cada turno:</p>
        <ul>
          <li><strong>Shape:</strong> Cómo se dispara el hechizo:
            <br>• <code>Origin(Missile, Effect)</code>: Proyectil en línea recta.
            <br>• <code>Victim(Missile, Range, Effect)</code>: Guiado hacia la víctima.
            <br>• <code>Actor(Effect)</code>: Centrado en el monstruo (Auto-Buff / Curación).
            <br>• <code>Destination(...)</code>: Área / Bomba en el suelo.
            <br>• <code>Angle(...)</code>: Cono / Wave frontal (ej: Dragon Wave).
          </li>
          <li><strong>Impact:</strong> Efecto causado (<code>Damage(Tipo, Max, Min)</code>, <code>Healing</code>, <code>Speed/Parálisis</code>, <code>Summon</code>, <code>Field</code>).</li>
          <li><strong>Delay (Turnos):</strong> Frecuencia del hechizo. <code>: 2</code> = 50% probabilidad/turno (~cada 4s), <code>: 6</code> = ~16% probabilidad/turno (~cada 12s).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>💬 4. Frases del Monstruo (Talk) y Gritos</h4>
        <ul>
          <li><strong>Frase Común (Say):</strong> Aparece en amarillo sobre la criatura en rango normal de visión.</li>
          <li><strong>Grito (#Y):</strong> Comienza con <code>#Y </code>. Aparece en <strong>rojo</strong> para todos los jugadores en toda la pantalla (ej: <code>#Y GROOAAARRR!</code>).</li>
        </ul>
      </div>
    `;
  } else {
    container.innerHTML = `
      <div class="guide-topic-card">
        <h4>⚔️ 1. Ataque Melee & Dano Físico</h4>
        <p>A cada rodada de combate (<strong>~2 segundos</strong>), o monstro executa um ataque físico contra seu alvo:</p>
        <div class="formula-box">
          <code>Dano_Bruto = random(0, Attack)</code>
        </div>
        <p>Em seguida, o valor de <strong>Defesa (Defend)</strong> do alvo tenta bloquear o golpe:</p>
        <div class="formula-box">
          <code>Dano_Apos_Escudo = max(0, Dano_Bruto - Defesa_Alvo)</code>
        </div>
        <p>Se <code>Dano_Apos_Escudo > 0</code>, a <strong>Armadura (Armor)</strong> do alvo absorve parte do impacto:</p>
        <div class="formula-box">
          <code>Absorcao_Armor = (Armor / 2) + random(0, Armor / 2)</code><br>
          <code>Dano_Final = max(0, Dano_Apos_Escudo - Absorcao_Armor)</code>
        </div>
      </div>

      <div class="guide-topic-card">
        <h4>🛡️ 2. Defesa & Resistência do Monstro</h4>
        <ul>
          <li><strong>Defend:</strong> Age como escudo. Bloqueia o dano físico de até 2 atacantes por rodada.</li>
          <li><strong>Armor:</strong> Absorve dano residual. Em média absorve <strong>75%</strong> do valor de Armor em cada golpe físico recebido.</li>
          <li><strong>Poison:</strong> Se <code>Poison > 0</code>, ao acertar um ataque físico o monstro aplica veneno periódico de <code>Poison/2</code> a <code>Poison</code> por segundo.</li>
          <li><strong>Blood:</strong> Tipo de sangue emitido (Blood = Vermelho, Slime = Verde, Undead = Fumaça, Fire = Fogo, Energy = Azul).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>✨ 3. Magias e Ataques à Distância (Spells)</h4>
        <p>As magias no formato <code>Shape -> Impact : Delay</code> são processadas a cada turno:</p>
        <ul>
          <li><strong>Shape:</strong> Como a magia é disparada:
            <br>• <code>Origin(Missile, Effect)</code>: Projétil em linha reta.
            <br>• <code>Victim(Missile, Range, Effect)</code>: Teleguiado até o alvo.
            <br>• <code>Actor(Effect)</code>: Centrado no monstro (Self Buff / Cura).
            <br>• <code>Destination(...)</code>: Área / Bomba no chão.
            <br>• <code>Angle(...)</code>: Cone / Wave frontal (ex: Dragon Wave).
          </li>
          <li><strong>Impact:</strong> O efeito causado (<code>Damage(Tipo, Max, Min)</code>, <code>Healing</code>, <code>Speed/Paralyze</code>, <code>Summon</code>, <code>Field</code>).</li>
          <li><strong>Delay (Turnos):</strong> Frequência da magia. <code>: 2</code> = 50% de chance/turno (~1 a cada 4s), <code>: 6</code> = ~16% de chance/turno (~1 a cada 12s).</li>
        </ul>
      </div>

      <div class="guide-topic-card">
        <h4>💬 4. Falas do Monstro (Talk) & Gritos</h4>
        <ul>
          <li><strong>Fala Comum (Say):</strong> Aparece em amarelo sobre a criatura no alcance normal de visão.</li>
          <li><strong>Grito (#Y):</strong> Começa com <code>#Y </code>. Aparece em <strong>vermelho</strong> para todos os jogadores na tela inteira (ex: <code>#Y GROOAAARRR!</code>).</li>
        </ul>
      </div>
    `;
  }
}

// Flags conhecidas do objects.srv
const KNOWN_FLAGS = [
  "Take", "Container", "Chest", "Cumulative", "Bank", "Clip", "Bottom", "Top",
  "Unmove", "Unpass", "Unthrow", "Unlay", "Avoid", "Weapon", "Armor", "Shield",
  "Food", "Rune", "Information", "Text", "Write", "WriteOnce", "LiquidContainer",
  "LiquidSource", "LiquidPool", "TeleportAbsolute", "TeleportRelative", "Key",
  "KeyDoor", "NameDoor", "LevelDoor", "QuestDoor", "Bed", "Disguise", "Height",
  "Light", "Corpse", "Expire", "WearOut", "Bow", "Throw", "Wand", "Ammo"
];

// Flags conhecidas de Monstros
const MONSTER_FLAGS = [
  "KickBoxes", "KickCreatures", "SeeInvisible", "Unpushable", "DistanceFighting",
  "NoSummon", "NoConvince", "NoIllusion", "NoBurning", "NoPoison", "NoEnergy",
  "NoParalyze"
];

// Inicialização ao carregar a página
document.addEventListener("DOMContentLoaded", async () => {
  setupLanguageSelector();
  setupNavigation();
  setupGlobalSearch();
  setupUnsavedChangesModal();
  setupDirtyTracking();
  setupObjectsTab();
  setupMonstersTab();
  setupNpcsTab();
  setupMoveUseTab();
  setupSpritesTab();
  setupDocsTab();
  setupRuleBuilder();
  setupSettingsTab();
  setupModals();

  setLanguage(state.lang);

  await loadInitialConfig();
  await loadDocs();
  await loadObjects();
  await loadMonsters();
  await loadNpcs();
  await loadMoveUse();
  await loadEffects();
});

function setupLanguageSelector() {
  const select = document.getElementById("app-language-select");
  if (select) {
    select.value = state.lang;
    select.addEventListener("change", (e) => {
      setLanguage(e.target.value);
    });
  }
}

// =============================================================================
// NAVEGAÇÃO & ABAS
// =============================================================================
function setupNavigation() {
  const navButtons = document.querySelectorAll(".nav-item");
  const tabPanes = document.querySelectorAll(".tab-pane");

  navButtons.forEach(btn => {
    btn.addEventListener("click", () => {
      const targetTab = btn.getAttribute("data-tab");
      if (state.currentTab === targetTab) return;

      confirmNavigation(() => {
        state.currentTab = targetTab;

        navButtons.forEach(b => b.classList.remove("active"));
        tabPanes.forEach(p => p.classList.remove("active"));

        btn.classList.add("active");
        const activePane = document.getElementById(targetTab);
        if (activePane) activePane.classList.add("active");

        applyTranslationsToDOM();

        if (targetTab === "tab-sprites" && state.sprites.total > 0) {
          renderSpritesGrid();
        }
        if (targetTab === "tab-docs") {
          renderDocsGrid();
        }
      });
    });
  });
}

function setupGlobalSearch() {
  const searchInput = document.getElementById("global-search");
  let searchTimeout = null;

  searchInput.addEventListener("input", (e) => {
    clearTimeout(searchTimeout);
    searchTimeout = setTimeout(() => {
      const q = e.target.value.trim();
      if (state.currentTab === "tab-objects") {
        state.objects.search = q;
        state.objects.page = 1;
        loadObjects();
      } else if (state.currentTab === "tab-monsters") {
        state.monsters.search = q;
        renderMonstersList();
      } else if (state.currentTab === "tab-npcs") {
        state.npcs.search = q;
        renderNpcsList();
      } else if (state.currentTab === "tab-moveuse") {
        state.moveuse.search = q;
        loadMoveUse();
      } else if (state.currentTab === "tab-docs") {
        state.docs.search = q;
        renderDocsGrid();
      }
    }, 250);
  });

  document.getElementById("btn-reload-data").addEventListener("click", () => {
    confirmNavigation(async () => {
      showToast(t("toast_reloading"), "info");
      await loadInitialConfig();
      await loadObjects();
      await loadMonsters();
      await loadNpcs();
      await loadMoveUse();
      await loadEffects();
      setDirty(false);
      showToast(t("toast_updated"), "success");
    });
  });

  document.getElementById("btn-global-save").addEventListener("click", () => {
    if (state.currentTab === "tab-objects") {
      document.getElementById("btn-save-object").click();
    } else if (state.currentTab === "tab-monsters") {
      document.getElementById("btn-save-monster").click();
    } else if (state.currentTab === "tab-npcs") {
      document.getElementById("btn-save-npc").click();
    } else if (state.currentTab === "tab-moveuse") {
      document.getElementById("btn-save-moveuse").click();
    }
  });
}

// =============================================================================
// CONFIGURAÇÃO & STATUS INICIAL
// =============================================================================
function renderConfigStatus() {
  const data = state.config || {};
  const statusUl = document.getElementById("config-status-list");
  if (!statusUl) return;

  statusUl.innerHTML = `
    <li><strong>objects.srv:</strong> ${data.objects_loaded ? `✅ ${t("status_loaded")} (${data.objects_count} ${t("status_types")})` : `❌ ${t("status_not_found")}`}</li>
    <li><strong>moveuse.dat:</strong> ${data.moveuse_loaded ? `✅ ${t("status_loaded")} (${data.moveuse_sections} ${t("status_sections")})` : `❌ ${t("status_not_found")}`}</li>
    <li><strong>${t("nav_monsters")}:</strong> ${data.monsters_loaded ? `✅ ${t("status_loaded")} (${data.monsters_count} ${t("status_creatures")})` : `❌ ${t("status_not_found")}`}</li>
    <li><strong>${t("nav_npcs")}:</strong> ${data.npcs_loaded ? `✅ ${t("status_loaded")} (${data.npcs_count} ${t("status_npcs")})` : `❌ ${t("status_not_found")}`}</li>
    <li><strong>Tibia.spr:</strong> ${data.spr_loaded ? `✅ ${t("status_loaded")} (${data.spr_count} ${t("status_sprites")})` : `⚠️ ${t("status_spr_missing")}`}</li>
    <li><strong>Tibia.dat:</strong> ${data.dat_loaded ? `✅ ${t("status_loaded")} (${data.dat_items} ${t("status_items")}, ${data.dat_creatures} ${t("status_creatures")})` : `⚠️ ${t("status_spr_missing")}`}</li>
  `;
}

async function loadInitialConfig() {
  try {
    const res = await fetch("/api/config");
    const data = await res.json();
    state.config = data;

    document.getElementById("cfg-data-path").value = data.data_path || "";
    document.getElementById("cfg-client-path").value = data.client_path || "";

    // Atualiza Badges
    document.getElementById("badge-objects").innerText = data.objects_count || 0;
    document.getElementById("badge-monsters").innerText = data.monsters_count || 0;
    document.getElementById("badge-npcs").innerText = data.npcs_count || 0;
    document.getElementById("badge-moveuse").innerText = data.moveuse_sections || 0;
    document.getElementById("badge-sprites").innerText = data.spr_count || 0;

    state.sprites.total = data.spr_count || 0;

    renderConfigStatus();
  } catch (err) {
    console.error("Erro ao carregar configurações:", err);
  }
}

// =============================================================================
// ABA 1: ITENS (OBJECTS.SRV)
// =============================================================================
function setupObjectsTab() {
  const flagsContainer = document.getElementById("flags-container");
  flagsContainer.innerHTML = KNOWN_FLAGS.map(flag => `
    <label class="flag-checkbox-label">
      <input type="checkbox" name="obj-flag" value="${flag}">
      ${flag}
    </label>
  `).join("");

  document.getElementById("obj-flag-filter").addEventListener("change", (e) => {
    const val = e.target.value;
    confirmNavigation(() => {
      state.objects.flag = val;
      state.objects.page = 1;
      loadObjects();
    });
  });

  document.getElementById("btn-prev-page").addEventListener("click", () => {
    if (state.objects.page > 1) {
      confirmNavigation(() => {
        state.objects.page--;
        loadObjects();
      });
    }
  });

  document.getElementById("btn-next-page").addEventListener("click", () => {
    confirmNavigation(() => {
      state.objects.page++;
      loadObjects();
    });
  });

  document.getElementById("btn-new-object").addEventListener("click", () => {
    confirmNavigation(() => {
      clearObjectForm();
      setDirty(false);
      showToast(t("obj_title_create"), "info");
    });
  });

  document.getElementById("btn-save-object").addEventListener("click", saveCurrentObject);
  document.getElementById("btn-delete-object").addEventListener("click", deleteCurrentObject);

  document.getElementById("obj-type-id").addEventListener("input", (e) => {
    const tid = parseInt(e.target.value) || 0;
    document.getElementById("obj-typeid-badge").innerText = `TypeID: ${tid}`;
    document.getElementById("obj-sprite-img").src = `/api/item_sprite/${tid}?zoom=3&t=${Date.now()}`;
  });
}

async function loadObjects() {
  try {
    const { page, limit, search, flag } = state.objects;
    const url = `/api/objects?page=${page}&limit=${limit}&q=${encodeURIComponent(search)}&flag=${encodeURIComponent(flag)}`;
    const res = await fetch(url);
    const data = await res.json();

    state.objects.total = data.total;
    document.getElementById("obj-total-counter").innerText = `${data.total} ${t("lbl_items_count")}`;
    document.getElementById("page-indicator").innerText = `${t("lbl_page")} ${data.page} / ${data.pages || 1}`;

    const listContainer = document.getElementById("objects-list");
    listContainer.innerHTML = data.items.map(item => `
      <div class="list-item-card ${state.objects.selectedTypeID === item.type_id ? 'selected' : ''}" data-typeid="${item.type_id}">
        <div class="item-thumb">
          <img src="/api/item_sprite/${item.type_id}?zoom=1" alt="${item.name}" loading="lazy">
        </div>
        <div class="item-info">
          <div class="item-name">${item.name || t("obj_unnamed")}</div>
          <div class="item-meta">
            <span>ID: ${item.type_id}</span>
            <span>${item.flags.slice(0, 2).join(", ")}</span>
          </div>
        </div>
      </div>
    `).join("");

    listContainer.querySelectorAll(".list-item-card").forEach(el => {
      el.addEventListener("click", () => {
        const typeId = parseInt(el.getAttribute("data-typeid"));
        if (state.objects.selectedTypeID === typeId) return;
        confirmNavigation(() => selectObject(typeId));
      });
    });

    if (data.items.length > 0 && state.objects.selectedTypeID === null) {
      selectObject(data.items[0].type_id);
    }
  } catch (err) {
    console.error("Erro ao carregar lista de itens:", err);
  }
}

async function selectObject(typeId) {
  state.objects.selectedTypeID = typeId;
  document.querySelectorAll("#objects-list .list-item-card").forEach(c => {
    c.classList.toggle("selected", parseInt(c.getAttribute("data-typeid")) === typeId);
  });

  try {
    const res = await fetch(`/api/objects/${typeId}`);
    if (!res.ok) return;
    const item = await res.json();

    document.getElementById("obj-type-id").value = item.type_id;
    document.getElementById("obj-typeid-badge").innerText = `TypeID: ${item.type_id}`;
    document.getElementById("obj-name").value = item.name || "";
    document.getElementById("obj-description").value = item.description || "";
    document.getElementById("obj-comment").value = item.comment || "";

    document.getElementById("obj-sprite-img").src = `/api/item_sprite/${item.type_id}?zoom=3&t=${Date.now()}`;

    // Atributos numéricos
    const attrs = item.attributes || {};
    document.getElementById("attr-weight").value = attrs.Weight !== undefined ? attrs.Weight : "";
    document.getElementById("attr-capacity").value = attrs.Capacity !== undefined ? attrs.Capacity : "";
    document.getElementById("attr-attack").value = attrs.WeaponAttackValue !== undefined ? attrs.WeaponAttackValue : "";
    document.getElementById("attr-defend").value = attrs.WeaponDefendValue !== undefined ? attrs.WeaponDefendValue : "";
    document.getElementById("attr-armor").value = attrs.ArmorValue !== undefined ? attrs.ArmorValue : "";
    document.getElementById("attr-nutrition").value = attrs.Nutrition !== undefined ? attrs.Nutrition : "";
    document.getElementById("attr-waypoints").value = attrs.Waypoints !== undefined ? attrs.Waypoints : "";
    document.getElementById("attr-totalexpire").value = attrs.TotalExpireTime !== undefined ? attrs.TotalExpireTime : "";

    const commonKeys = ["Weight", "Capacity", "WeaponAttackValue", "WeaponDefendValue", "ArmorValue", "Nutrition", "Waypoints", "TotalExpireTime"];
    const otherAttrs = [];
    for (const [k, v] of Object.entries(attrs)) {
      if (!commonKeys.includes(k)) {
        otherAttrs.push(`${k}=${v}`);
      }
    }
    document.getElementById("obj-raw-attrs").value = otherAttrs.join(", ");

    const flagsSet = new Set(item.flags || []);
    document.querySelectorAll("input[name='obj-flag']").forEach(chk => {
      chk.checked = flagsSet.has(chk.value);
    });

    setDirty(false);
  } catch (err) {
    console.error("Erro ao selecionar objeto:", err);
  }
}

function clearObjectForm() {
  state.objects.selectedTypeID = null;
  document.querySelectorAll("#objects-list .list-item-card").forEach(c => c.classList.remove("selected"));

  document.getElementById("obj-type-id").value = "";
  document.getElementById("obj-typeid-badge").innerText = `TypeID: ${t("obj_typeid_new")}`;
  document.getElementById("obj-name").value = "";
  document.getElementById("obj-description").value = "";
  document.getElementById("obj-comment").value = "";
  document.getElementById("obj-sprite-img").src = `/api/item_sprite/0?zoom=3`;

  document.getElementById("attr-weight").value = "";
  document.getElementById("attr-capacity").value = "";
  document.getElementById("attr-attack").value = "";
  document.getElementById("attr-defend").value = "";
  document.getElementById("attr-armor").value = "";
  document.getElementById("attr-nutrition").value = "";
  document.getElementById("attr-waypoints").value = "";
  document.getElementById("attr-totalexpire").value = "";
  document.getElementById("obj-raw-attrs").value = "";

  document.querySelectorAll("input[name='obj-flag']").forEach(chk => chk.checked = false);
}

async function saveCurrentObject() {
  const typeId = parseInt(document.getElementById("obj-type-id").value);
  if (isNaN(typeId) || typeId < 0) {
    showToast(t("toast_invalid_typeid"), "error");
    return false;
  }

  const flags = [];
  document.querySelectorAll("input[name='obj-flag']:checked").forEach(chk => flags.push(chk.value));

  const attributes = {};
  const weight = document.getElementById("attr-weight").value.trim();
  if (weight) attributes["Weight"] = parseInt(weight);

  const cap = document.getElementById("attr-capacity").value.trim();
  if (cap) attributes["Capacity"] = parseInt(cap);

  const atk = document.getElementById("attr-attack").value.trim();
  if (atk) attributes["WeaponAttackValue"] = parseInt(atk);

  const def = document.getElementById("attr-defend").value.trim();
  if (def) attributes["WeaponDefendValue"] = parseInt(def);

  const arm = document.getElementById("attr-armor").value.trim();
  if (arm) attributes["ArmorValue"] = parseInt(arm);

  const nut = document.getElementById("attr-nutrition").value.trim();
  if (nut) attributes["Nutrition"] = parseInt(nut);

  const way = document.getElementById("attr-waypoints").value.trim();
  if (way) attributes["Waypoints"] = parseInt(way);

  const exp = document.getElementById("attr-totalexpire").value.trim();
  if (exp) attributes["TotalExpireTime"] = parseInt(exp);

  const rawOther = document.getElementById("obj-raw-attrs").value.trim();
  if (rawOther) {
    rawOther.split(",").forEach(pair => {
      if (pair.includes("=")) {
        const [k, v] = pair.split("=");
        const key = k.trim();
        const val = v.trim();
        attributes[key] = isNaN(val) ? val : Number(val);
      }
    });
  }

  const payload = {
    type_id: typeId,
    name: document.getElementById("obj-name").value.trim(),
    description: document.getElementById("obj-description").value.trim(),
    comment: document.getElementById("obj-comment").value.trim(),
    flags: flags,
    attributes: attributes
  };

  try {
    const res = await fetch("/api/objects/save", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const result = await res.json();
    if (result.success) {
      showToast(`${t("toast_item_saved")} (ID: ${typeId})`, "success");
      setDirty(false);
      await loadObjects();
      selectObject(typeId);
      return true;
    } else {
      showToast(result.error || "Erro ao salvar item", "error");
      return false;
    }
  } catch (err) {
    showToast("Erro de rede ao salvar item", "error");
    return false;
  }
}

async function deleteCurrentObject() {
  const typeId = state.objects.selectedTypeID;
  if (!typeId) return;

  if (!confirm(`${t("confirm_delete_item")} ${typeId}?`)) return;

  try {
    const res = await fetch("/api/objects/delete", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ type_id: typeId })
    });
    const result = await res.json();
    if (result.success) {
      showToast(`Item ${typeId} excluído`, "success");
      clearObjectForm();
      loadObjects();
    }
  } catch (err) {
    showToast("Erro ao excluir item", "error");
  }
}

// =============================================================================
// ABA 2: MONSTROS (.MON) COM EDITOR VISUAL DE MAGIAS E FALAS
// =============================================================================
let currentMonsterLoot = [];
let currentMonsterSpells = [];
let currentMonsterTalk = [];
let editingSpellIndex = null;

function setupMonstersTab() {
  const flagsContainer = document.getElementById("mon-flags-container");
  flagsContainer.innerHTML = MONSTER_FLAGS.map(flag => `
    <label class="flag-checkbox-label">
      <input type="checkbox" name="mon-flag" value="${flag}">
      ${flag}
    </label>
  `).join("");

  document.getElementById("btn-add-loot-item").addEventListener("click", () => {
    openItemPickerModal(t("modal_pick_loot"), (item) => {
      currentMonsterLoot.push({
        item_id: item.type_id,
        count: 1,
        chance: 100,
        percentage: 10.0
      });
      setDirty(true);
      renderLootTable();
    });
  });

  document.getElementById("btn-save-monster").addEventListener("click", saveCurrentMonster);
  document.getElementById("btn-new-monster").addEventListener("click", () => {
    confirmNavigation(() => {
      clearMonsterForm();
      setDirty(false);
    });
  });

  const updateMonsterPreview = () => {
    const outfit = parseInt(document.getElementById("mon-outfit").value) || 0;
    const colors = document.getElementById("mon-colors").value.trim() || "0-0-0-0";
    document.getElementById("mon-sprite-img").src = `/api/outfit_sprite/${outfit}?zoom=3&colors=${encodeURIComponent(colors)}&t=${Date.now()}`;
  };

  document.getElementById("mon-outfit").addEventListener("input", updateMonsterPreview);
  document.getElementById("mon-colors").addEventListener("input", updateMonsterPreview);

  // Listeners para Métricas de Combate em Tempo Real
  ["mon-hp", "mon-atk", "mon-armor", "mon-flee"].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.addEventListener("input", updateMonsterCombatSummary);
  });

  // Corpse Picker
  const corpseInput = document.getElementById("mon-corpse");
  const corpseImg = document.getElementById("mon-corpse-img");
  const updateCorpseImg = () => {
    const cId = parseInt(corpseInput.value) || 0;
    if (cId > 0 && corpseImg) {
      corpseImg.src = `/api/item_sprite/${cId}?zoom=1&t=${Date.now()}`;
    }
  };
  corpseInput.addEventListener("input", updateCorpseImg);

  const btnPickCorpse = document.getElementById("btn-pick-corpse");
  if (btnPickCorpse) {
    btnPickCorpse.addEventListener("click", () => {
      openItemPickerModal(t("modal_pick_corpse"), (item) => {
        corpseInput.value = item.type_id;
        updateCorpseImg();
        setDirty(true);
      }, "Corpse,Container");
    });
  }

  // Spells & Talk Buttons
  const btnAddSpell = document.getElementById("btn-add-monster-spell");
  if (btnAddSpell) {
    btnAddSpell.addEventListener("click", () => openMonsterSpellModal(null));
  }

  const btnAddTalk = document.getElementById("btn-add-monster-talk");
  if (btnAddTalk) {
    btnAddTalk.addEventListener("click", () => {
      currentMonsterTalk.push("GROOAAARRR!");
      setDirty(true);
      renderMonsterTalkList();
    });
  }

  setupMonsterSpellModal();
  setupCombatGuideModal();
}

function updateMonsterCombatSummary() {
  const hp = parseInt(document.getElementById("mon-hp").value) || 0;
  const atk = parseInt(document.getElementById("mon-atk").value) || 0;
  const armor = parseInt(document.getElementById("mon-armor").value) || 0;
  const flee = parseInt(document.getElementById("mon-flee").value) || 0;

  const avgMelee = (atk / 2).toFixed(1);
  const avgArmor = (armor * 0.75).toFixed(1);
  const fleePct = hp > 0 ? Math.min(100, Math.round((flee / hp) * 100)) : 0;

  let maxSpellsDmg = 0;
  currentMonsterSpells.forEach(s => {
    const sp = parseMonsterSpell(s);
    if (sp.impact === "Damage") {
      maxSpellsDmg += (sp.impactParam2 || 0);
    }
  });

  const totalCombo = atk + maxSpellsDmg;

  const avgMeleeEl = document.getElementById("summary-avg-melee");
  if (avgMeleeEl) avgMeleeEl.innerText = avgMelee;

  const avgArmorEl = document.getElementById("summary-avg-armor");
  if (avgArmorEl) avgArmorEl.innerText = avgArmor;

  const fleeEl = document.getElementById("summary-flee-pct");
  if (fleeEl) fleeEl.innerText = `${fleePct}% (${flee} HP)`;

  const comboEl = document.getElementById("summary-max-combo");
  if (comboEl) comboEl.innerText = `${totalCombo} (${t("summary_combo_melee")}: ${atk} + ${t("summary_combo_spells")}: ${maxSpellsDmg})`;
}

function setupCombatGuideModal() {
  const modal = document.getElementById("monster-combat-guide-modal");
  const btnOpen = document.getElementById("btn-mon-combat-guide");
  const btnClose = document.getElementById("btn-close-combat-guide");
  const btnCloseFooter = document.getElementById("btn-close-combat-guide-footer");

  if (btnOpen) {
    btnOpen.addEventListener("click", () => {
      const curAtk = parseInt(document.getElementById("mon-atk").value) || 50;
      document.getElementById("sim-mon-atk").value = curAtk;
      modal.classList.add("active");
    });
  }

  if (btnClose) btnClose.addEventListener("click", () => modal.classList.remove("active"));
  if (btnCloseFooter) btnCloseFooter.addEventListener("click", () => modal.classList.remove("active"));

  const btnSim = document.getElementById("btn-run-damage-sim");
  if (btnSim) {
    btnSim.addEventListener("click", () => {
      const atk = parseInt(document.getElementById("sim-mon-atk").value) || 0;
      const def = parseInt(document.getElementById("sim-target-def").value) || 0;
      const armor = parseInt(document.getElementById("sim-target-armor").value) || 0;

      let totalDmg = 0;
      let maxDmg = 0;
      let blocks = 0;
      const rounds = 100;

      for (let i = 0; i < rounds; i++) {
        const rawAttack = Math.floor(Math.random() * (atk + 1));
        const shieldDef = Math.floor(Math.random() * (def + 1));
        const afterShield = Math.max(0, rawAttack - shieldDef);

        if (afterShield <= 0) {
          blocks++;
          continue;
        }

        const halfArmor = Math.floor(armor / 2);
        const armorAbsorb = halfArmor + Math.floor(Math.random() * (halfArmor + 1));
        const finalDmg = Math.max(0, afterShield - armorAbsorb);

        if (finalDmg <= 0) {
          blocks++;
        } else {
          totalDmg += finalDmg;
          if (finalDmg > maxDmg) maxDmg = finalDmg;
        }
      }

      const hits = rounds - blocks;
      const avgDmg = (totalDmg / rounds).toFixed(1);
      const blockRate = Math.round((blocks / rounds) * 100);
      const hitRate = Math.round((hits / rounds) * 100);

      document.getElementById("sim-avg-dmg").innerText = avgDmg;
      document.getElementById("sim-max-dmg").innerText = maxDmg;
      document.getElementById("sim-block-rate").innerText = `${blockRate}%`;
      document.getElementById("sim-hit-rate").innerText = `${hitRate}%`;
      document.getElementById("sim-results-output").style.display = "block";
    });
  }
}

async function loadMonsters() {
  try {
    const res = await fetch("/api/monsters");
    const data = await res.json();
    state.monsters.list = data.monsters;
    renderMonstersList();
  } catch (err) {
    console.error("Erro ao carregar monstros:", err);
  }
}

function renderMonstersList() {
  const listContainer = document.getElementById("monsters-list");
  const q = state.monsters.search.toLowerCase();

  const filtered = state.monsters.list.filter(m => {
    if (!q) return true;
    return m.name.toLowerCase().includes(q) || m.filename.toLowerCase().includes(q) || String(m.race_number).startsWith(q);
  });

  listContainer.innerHTML = filtered.map(m => `
    <div class="list-item-card ${state.monsters.selectedFilename === m.filename ? 'selected' : ''}" data-file="${m.filename}">
      <div class="item-thumb">
        <img src="/api/outfit_sprite/${m.outfit_type}?zoom=1&colors=${encodeURIComponent(m.outfit_colors || '0-0-0-0')}" alt="${m.name}" loading="lazy">
      </div>
      <div class="item-info">
        <div class="item-name">${m.name}</div>
        <div class="item-meta">
          <span>HP: ${m.hitpoints}</span>
          <span>XP: ${m.experience}</span>
        </div>
      </div>
    </div>
  `).join("");

  listContainer.querySelectorAll(".list-item-card").forEach(el => {
    el.addEventListener("click", () => {
      const filename = el.getAttribute("data-file");
      if (state.monsters.selectedFilename === filename) return;
      confirmNavigation(() => selectMonster(filename));
    });
  });

  if (filtered.length > 0 && state.monsters.selectedFilename === null) {
    selectMonster(filtered[0].filename);
  }
}

async function selectMonster(filename) {
  state.monsters.selectedFilename = filename;
  document.querySelectorAll("#monsters-list .list-item-card").forEach(c => {
    c.classList.toggle("selected", c.getAttribute("data-file") === filename);
  });

  try {
    const res = await fetch(`/api/monsters/${encodeURIComponent(filename)}`);
    if (!res.ok) return;
    const m = await res.json();

    document.getElementById("monster-title").innerText = `${t("mon_title_edit")} ${m.name} (${m.filename})`;
    document.getElementById("mon-name").value = m.name;
    document.getElementById("mon-race").value = m.race_number;
    document.getElementById("mon-race-badge").innerText = `Race: ${m.race_number}`;
    document.getElementById("mon-outfit").value = m.outfit_type;
    const colors = m.outfit_colors || "0-0-0-0";
    document.getElementById("mon-colors").value = colors;

    document.getElementById("mon-hp").value = m.hitpoints;
    document.getElementById("mon-exp").value = m.experience;
    document.getElementById("mon-atk").value = m.attack;
    document.getElementById("mon-def").value = m.defend;
    document.getElementById("mon-armor").value = m.armor;
    document.getElementById("mon-speed").value = m.speed;
    document.getElementById("mon-blood").value = m.blood || "Blood";
    document.getElementById("mon-flee").value = m.flee_threshold;
    document.getElementById("mon-poison").value = m.poison || 0;

    const corpseId = m.corpse || 0;
    document.getElementById("mon-corpse").value = corpseId;
    const corpseImg = document.getElementById("mon-corpse-img");
    if (corpseImg) {
      corpseImg.src = `/api/item_sprite/${corpseId || 4025}?zoom=1&t=${Date.now()}`;
    }

    document.getElementById("mon-sprite-img").src = `/api/outfit_sprite/${m.outfit_type}?zoom=3&colors=${encodeURIComponent(colors)}&t=${Date.now()}`;

    const flagsSet = new Set(m.flags || []);
    document.querySelectorAll("input[name='mon-flag']").forEach(chk => {
      chk.checked = flagsSet.has(chk.value);
    });

    currentMonsterSpells = m.spells_raw || [];
    currentMonsterTalk = m.talk || [];
    currentMonsterLoot = m.inventory || [];

    renderLootTable();
    renderMonsterSpellsList();
    renderMonsterTalkList();
    updateMonsterCombatSummary();
    setDirty(false);
  } catch (err) {
    console.error("Erro ao carregar detalhes do monstro:", err);
  }
}

// ---- Tabela de Loot ----
function renderLootTable() {
  const tbody = document.getElementById("loot-table-body");
  if (currentMonsterLoot.length === 0) {
    tbody.innerHTML = `<tr><td colspan="6" style="text-align: center; color: var(--text-muted); padding: 1rem;">${t("no_loot_registered")}</td></tr>`;
    return;
  }

  tbody.innerHTML = currentMonsterLoot.map((item, idx) => `
    <tr>
      <td>
        <div class="loot-item-cell">
          <div class="loot-item-thumb">
            <img src="/api/item_sprite/${item.item_id}?zoom=1" alt="item">
          </div>
          <span>Item #${item.item_id}</span>
        </div>
      </td>
      <td>
        <input type="number" style="width: 80px;" value="${item.item_id}" onchange="updateLootItem(${idx}, 'item_id', this.value)">
      </td>
      <td>
        <input type="number" style="width: 70px;" value="${item.count}" min="1" max="100" onchange="updateLootItem(${idx}, 'count', this.value)">
      </td>
      <td>
        <input type="number" style="width: 90px;" value="${item.chance}" min="1" max="1000" onchange="updateLootItem(${idx}, 'chance', this.value)">
      </td>
      <td>
        <strong>${(item.chance / 10).toFixed(1)}%</strong>
      </td>
      <td>
        <button class="btn btn-xs btn-danger" onclick="removeLootItem(${idx})">${t("btn_remove")}</button>
      </td>
    </tr>
  `).join("");
}

window.updateLootItem = function(idx, field, val) {
  if (currentMonsterLoot[idx]) {
    currentMonsterLoot[idx][field] = parseInt(val) || 0;
    setDirty(true);
    renderLootTable();
  }
};

window.removeLootItem = function(idx) {
  currentMonsterLoot.splice(idx, 1);
  setDirty(true);
  renderLootTable();
};

// ---- Editor de Magias de Monstros ----
function parseMonsterSpell(spellStr) {
  const clean = spellStr.trim();
  const res = {
    raw: clean,
    shape: "Origin",
    shapeParam1: 0,
    shapeParam2: 21,
    impact: "Damage",
    impactParam1: 4,
    impactParam2: 450,
    impactParam3: 50,
    delay: 6,
    magicEffect: 21
  };

  const delayM = clean.match(/:\s*(\d+)\s*$/);
  if (delayM) res.delay = parseInt(delayM[1]);

  const shapeM = clean.match(/^([A-Za-z]+)\s*\(([^)]+)\)/);
  if (shapeM) {
    res.shape = shapeM[1];
    const sParts = shapeM[2].split(",").map(s => parseInt(s.trim()) || 0);
    res.shapeParam1 = sParts[0] || 0;
    res.shapeParam2 = sParts[1] || 0;
    if (res.shape === "Actor") res.magicEffect = sParts[0] || 1;
    else if (res.shape === "Origin") res.magicEffect = sParts[1] || 1;
    else if (res.shape === "Victim") res.magicEffect = sParts[2] || sParts[1] || 1;
  }

  const impactM = clean.match(/->\s*([A-Za-z]+)\s*\(([^)]+)\)/);
  if (impactM) {
    res.impact = impactM[1];
    const iParts = impactM[2].split(",").map(s => parseInt(s.trim()) || 0);
    res.impactParam1 = iParts[0] || 0;
    res.impactParam2 = iParts[1] || 0;
    res.impactParam3 = iParts[2] || 0;
  }

  return res;
}

function renderMonsterSpellsList() {
  const container = document.getElementById("mon-spells-container");
  if (!container) return;

  if (currentMonsterSpells.length === 0) {
    container.innerHTML = `<tr><td colspan="4" style="color:var(--text-muted);font-size:0.8rem;padding:12px;text-align:center;">${t("no_spells_registered")}</td></tr>`;
    return;
  }

  container.innerHTML = currentMonsterSpells.map((sStr, idx) => {
    const sp = parseMonsterSpell(sStr);
    let typeClass = "damage";
    let descText = "";

    const dmgNames = {
      1: t("dmg_name_physical"),
      4: t("dmg_name_fire"),
      8: t("dmg_name_poison"),
      16: t("dmg_name_energy"),
      32: t("dmg_name_death"),
      64: t("dmg_name_manadrain"),
      128: t("dmg_name_holy"),
      256: t("dmg_name_drown")
    };

    if (sp.impact === "Damage") {
      typeClass = "damage";
      const dmgName = dmgNames[sp.impactParam1] || `Tipo ${sp.impactParam1}`;
      descText = `${t("spell_lbl_damage")} ${dmgName}: ${sp.impactParam3} ${t("spell_lbl_to")} ${sp.impactParam2} • ${t("spell_lbl_turns")}: ${sp.delay} (~${Math.round(100 / (sp.delay || 1))}%)`;
    } else if (sp.impact === "Healing") {
      typeClass = "healing";
      descText = `${t("spell_lbl_heal")}: +${sp.impactParam2} ${t("spell_lbl_to")} +${sp.impactParam1} HP • ${t("spell_lbl_turns")}: ${sp.delay}`;
    } else if (sp.impact === "Speed") {
      typeClass = "speed";
      const mode = sp.impactParam1 < 0 ? "Paralyze" : "Haste";
      descText = `${mode}: ${sp.impactParam1} (${sp.impactParam2}s) • ${t("spell_lbl_turns")}: ${sp.delay}`;
    } else if (sp.impact === "Summon") {
      typeClass = "summon";
      descText = `${t("spell_lbl_summon")} #${sp.impactParam1} (${t("spell_lbl_to")} ${sp.impactParam2} ${t("spell_lbl_minions")}) • ${t("spell_lbl_turns")}: ${sp.delay}`;
    } else {
      typeClass = "damage";
      descText = `${sp.impact} • ${t("spell_lbl_turns")}: ${sp.delay}`;
    }

    return `
      <tr>
        <td style="text-align:center;width:56px;">
          <div class="spell-thumb-box" title="Effect #${Number(sp.magicEffect) || 0}" style="margin:0 auto;">
            <img src="/api/effect_sprite/${Number(sp.magicEffect) || 1}?zoom=1" alt="eff_${Number(sp.magicEffect) || 0}">
          </div>
        </td>
        <td>
          <span class="spell-type-badge ${typeClass}">${escapeHtmlText(sp.impact)}</span>
          <div style="margin-top:4px;font-size:0.8rem;">${escapeHtmlText(sp.shape)}</div>
        </td>
        <td class="spell-params-summary">${escapeHtmlText(descText)}</td>
        <td style="white-space:nowrap;text-align:center;">
          <button class="btn btn-xs btn-secondary" onclick="openMonsterSpellModal(${idx})">✏️</button>
          <button class="btn btn-xs btn-danger" onclick="removeMonsterSpell(${idx})">✕</button>
        </td>
      </tr>
    `;
  }).join("");
}

window.removeMonsterSpell = function(idx) {
  currentMonsterSpells.splice(idx, 1);
  setDirty(true);
  renderMonsterSpellsList();
  updateMonsterCombatSummary();
};

// ---- Editor de Falas (Talk) com Toggle Grito / Fala Normal ----
function renderMonsterTalkList() {
  const container = document.getElementById("mon-talk-container");
  if (!container) return;

  if (currentMonsterTalk.length === 0) {
    container.innerHTML = `<tr><td colspan="3" style="color:var(--text-muted);font-size:0.8rem;padding:12px;text-align:center;">${t("no_talk_registered")}</td></tr>`;
    return;
  }

  container.innerHTML = currentMonsterTalk.map((line, idx) => {
    const isYell = line.startsWith("#Y ") || line.startsWith("#y ");
    const cleanText = isYell ? line.replace(/^#[Yy]\s*/, "") : line;

    return `
      <tr>
        <td style="width:90px;text-align:center;">
          <button class="btn-yell-toggle ${isYell ? 'is-yell' : ''}" onclick="toggleMonsterTalkYell(${idx})" title="${t("badge_yell")} / ${t("badge_say")}">
            ${isYell ? t("badge_yell") : t("badge_say")}
          </button>
        </td>
        <td>
          <input type="text" class="talk-input-text" value="${escapeHtmlText(cleanText).replace(/"/g, '&quot;')}" oninput="updateMonsterTalkText(${idx}, this.value)" placeholder="${t("placeholder_talk")}">
        </td>
        <td style="text-align:center;width:50px;">
          <button class="btn btn-xs btn-danger" onclick="removeMonsterTalk(${idx})">✕</button>
        </td>
      </tr>
    `;
  }).join("");
}

window.toggleMonsterTalkYell = function(idx) {
  const cur = currentMonsterTalk[idx] || "";
  const isYell = cur.startsWith("#Y ") || cur.startsWith("#y ");
  const clean = isYell ? cur.replace(/^#[Yy]\s*/, "") : cur;
  currentMonsterTalk[idx] = isYell ? clean : `#Y ${clean}`;
  setDirty(true);
  renderMonsterTalkList();
};

window.updateMonsterTalkText = function(idx, text) {
  const cur = currentMonsterTalk[idx] || "";
  const isYell = cur.startsWith("#Y ") || cur.startsWith("#y ");
  currentMonsterTalk[idx] = isYell ? `#Y ${text.trim()}` : text.trim();
  setDirty(true);
};

window.removeMonsterTalk = function(idx) {
  currentMonsterTalk.splice(idx, 1);
  setDirty(true);
  renderMonsterTalkList();
};

// ---- Modal Construtor de Magia de Monstro ----
function setupMonsterSpellModal() {
  const modal = document.getElementById("monster-spell-modal");
  document.getElementById("btn-close-mon-spell").addEventListener("click", () => modal.classList.remove("active"));
  document.getElementById("btn-cancel-mon-spell").addEventListener("click", () => modal.classList.remove("active"));

  const impactSelect = document.getElementById("spell-impact-type");
  const shapeSelect = document.getElementById("spell-shape-type");

  const updateSpellFieldsVisibility = () => {
    const imp = impactSelect.value;
    document.getElementById("spell-damage-fields").style.display = imp === "Damage" ? "grid" : "none";
    document.getElementById("spell-healing-fields").style.display = imp === "Healing" ? "grid" : "none";
    document.getElementById("spell-speed-fields").style.display = imp === "Speed" ? "grid" : "none";
    document.getElementById("spell-summon-fields").style.display = imp === "Summon" ? "grid" : "none";
    document.getElementById("spell-field-fields").style.display = imp === "Field" ? "grid" : "none";
    document.getElementById("spell-drunken-fields").style.display = imp === "Drunken" ? "grid" : "none";
    updateMonsterSpellCodePreview();
  };

  impactSelect.addEventListener("change", updateSpellFieldsVisibility);
  shapeSelect.addEventListener("change", updateMonsterSpellCodePreview);

  // Inputs listeners
  [
    "spell-damage-type", "spell-damage-min", "spell-damage-max",
    "spell-heal-min", "spell-heal-max",
    "spell-speed-val", "spell-speed-dur", "spell-speed-eff",
    "spell-summon-race", "spell-summon-count",
    "spell-field-item", "spell-drunk-intensity", "spell-drunk-duration", "spell-drunk-effect",
    "spell-magic-effect", "spell-missile-effect", "spell-delay"
  ].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.addEventListener("input", updateMonsterSpellCodePreview);
  });

  // Pick spell effect
  const btnPickEff = document.getElementById("btn-pick-spell-effect");
  const effInput = document.getElementById("spell-magic-effect");
  const effPreviewImg = document.getElementById("spell-effect-preview-img");

  if (btnPickEff) {
    btnPickEff.addEventListener("click", () => {
      openEffectPickerModal((eff) => {
        effInput.value = eff.id;
        effPreviewImg.src = `/api/effect_sprite/${eff.id}?zoom=1&t=${Date.now()}`;
        updateMonsterSpellCodePreview();
      });
    });
  }

  effInput.addEventListener("input", () => {
    const val = parseInt(effInput.value) || 1;
    effPreviewImg.src = `/api/effect_sprite/${val}?zoom=1&t=${Date.now()}`;
  });

  document.getElementById("btn-confirm-mon-spell").addEventListener("click", saveMonsterSpellFromModal);
}

function updateMonsterSpellCodePreview() {
  const shape = document.getElementById("spell-shape-type").value;
  const impact = document.getElementById("spell-impact-type").value;
  const magicEff = parseInt(document.getElementById("spell-magic-effect").value) || 21;
  const missile = parseInt(document.getElementById("spell-missile-effect").value) || 0;
  const delay = parseInt(document.getElementById("spell-delay").value) || 6;

  let shapeCode = "";
  if (shape === "Actor") {
    shapeCode = `Actor (${magicEff})`;
  } else if (shape === "Origin") {
    shapeCode = `Origin (${missile}, ${magicEff})`;
  } else if (shape === "Victim") {
    shapeCode = `Victim (${missile}, 7, ${magicEff})`;
  } else if (shape === "Destination") {
    shapeCode = `Destination (0, ${missile}, 7, ${magicEff})`;
  } else if (shape === "Angle") {
    shapeCode = `Angle (3, 7, ${magicEff})`;
  }

  let impactCode = "";
  if (impact === "Damage") {
    const dmgType = parseInt(document.getElementById("spell-damage-type").value) || 4;
    const minDmg = parseInt(document.getElementById("spell-damage-min").value) || 0;
    const maxDmg = parseInt(document.getElementById("spell-damage-max").value) || 100;
    impactCode = `Damage (${dmgType}, ${maxDmg}, ${minDmg})`;
  } else if (impact === "Healing") {
    const minHeal = parseInt(document.getElementById("spell-heal-min").value) || 10;
    const maxHeal = parseInt(document.getElementById("spell-heal-max").value) || 50;
    impactCode = `Healing (${maxHeal}, ${minHeal})`;
  } else if (impact === "Speed") {
    const spdVal = parseInt(document.getElementById("spell-speed-val").value) || -50;
    const spdDur = parseInt(document.getElementById("spell-speed-dur").value) || 10;
    const spdEff = parseInt(document.getElementById("spell-speed-eff").value) || 25;
    impactCode = `Speed (${spdVal}, ${spdDur}, ${spdEff})`;
  } else if (impact === "Summon") {
    const sRace = parseInt(document.getElementById("spell-summon-race").value) || 82;
    const sCount = parseInt(document.getElementById("spell-summon-count").value) || 3;
    impactCode = `Summon (${sRace}, ${sCount})`;
  } else if (impact === "Field") {
    const fItem = parseInt(document.getElementById("spell-field-item").value) || 1487;
    impactCode = `Field (${fItem})`;
  } else if (impact === "Drunken") {
    const dInt = parseInt(document.getElementById("spell-drunk-intensity").value) || 50;
    const dDur = parseInt(document.getElementById("spell-drunk-duration").value) || 15;
    const dEff = parseInt(document.getElementById("spell-drunk-effect").value) || 11;
    impactCode = `Drunken (${dInt}, ${dDur}, ${dEff})`;
  }

  const fullCode = `${shapeCode} -> ${impactCode} : ${delay}`;
  document.getElementById("mon-spell-code-output").innerText = fullCode;
  return fullCode;
}

window.openMonsterSpellModal = function(editIdx) {
  editingSpellIndex = editIdx;
  const modal = document.getElementById("monster-spell-modal");
  modal.classList.add("active");

  if (editIdx !== null && currentMonsterSpells[editIdx]) {
    const sp = parseMonsterSpell(currentMonsterSpells[editIdx]);
    document.getElementById("spell-shape-type").value = sp.shape || "Origin";
    document.getElementById("spell-impact-type").value = sp.impact || "Damage";
    document.getElementById("spell-magic-effect").value = sp.magicEffect || 21;
    document.getElementById("spell-missile-effect").value = sp.shapeParam1 || 0;
    document.getElementById("spell-delay").value = sp.delay || 6;

    if (sp.impact === "Damage") {
      document.getElementById("spell-damage-type").value = sp.impactParam1 || 4;
      document.getElementById("spell-damage-max").value = sp.impactParam2 || 100;
      document.getElementById("spell-damage-min").value = sp.impactParam3 || 0;
    } else if (sp.impact === "Healing") {
      document.getElementById("spell-heal-max").value = sp.impactParam1 || 50;
      document.getElementById("spell-heal-min").value = sp.impactParam2 || 10;
    } else if (sp.impact === "Speed") {
      document.getElementById("spell-speed-val").value = sp.impactParam1 || -50;
      document.getElementById("spell-speed-dur").value = sp.impactParam2 || 10;
      document.getElementById("spell-speed-eff").value = sp.impactParam3 || 25;
    } else if (sp.impact === "Summon") {
      document.getElementById("spell-summon-race").value = sp.impactParam1 || 82;
      document.getElementById("spell-summon-count").value = sp.impactParam2 || 3;
    }
  } else {
    document.getElementById("spell-shape-type").value = "Origin";
    document.getElementById("spell-impact-type").value = "Damage";
    document.getElementById("spell-damage-type").value = "4";
    document.getElementById("spell-damage-min").value = "50";
    document.getElementById("spell-damage-max").value = "450";
    document.getElementById("spell-magic-effect").value = "21";
    document.getElementById("spell-missile-effect").value = "4";
    document.getElementById("spell-delay").value = "6";
  }

  document.getElementById("spell-effect-preview-img").src = `/api/effect_sprite/${document.getElementById("spell-magic-effect").value || 21}?zoom=1`;
  document.getElementById("spell-impact-type").dispatchEvent(new Event("change"));
};

function saveMonsterSpellFromModal() {
  const generatedCode = updateMonsterSpellCodePreview();
  if (editingSpellIndex !== null) {
    currentMonsterSpells[editingSpellIndex] = generatedCode;
  } else {
    currentMonsterSpells.push(generatedCode);
  }

  document.getElementById("monster-spell-modal").classList.remove("active");
  setDirty(true);
  renderMonsterSpellsList();
  updateMonsterCombatSummary();
}

function clearMonsterForm() {
  state.monsters.selectedFilename = null;
  document.querySelectorAll("#monsters-list .list-item-card").forEach(c => c.classList.remove("selected"));

  document.getElementById("monster-title").innerText = t("mon_title_create");
  document.getElementById("mon-name").value = "";
  document.getElementById("mon-race").value = "0";
  document.getElementById("mon-race-badge").innerText = `Race: ${t("mon_race_new")}`;
  document.getElementById("mon-outfit").value = "0";
  document.getElementById("mon-colors").value = "0-0-0-0";
  document.getElementById("mon-hp").value = "100";
  document.getElementById("mon-exp").value = "50";
  document.getElementById("mon-atk").value = "10";
  document.getElementById("mon-def").value = "10";
  document.getElementById("mon-armor").value = "5";
  document.getElementById("mon-speed").value = "50";
  document.getElementById("mon-blood").value = "Blood";
  document.getElementById("mon-corpse").value = "4025";
  document.getElementById("mon-flee").value = "0";
  document.getElementById("mon-poison").value = "0";

  document.querySelectorAll("input[name='mon-flag']").forEach(chk => chk.checked = false);
  currentMonsterLoot = [];
  currentMonsterSpells = [];
  currentMonsterTalk = [];

  renderLootTable();
  renderMonsterSpellsList();
  renderMonsterTalkList();
  updateMonsterCombatSummary();
}

async function saveCurrentMonster() {
  const name = document.getElementById("mon-name").value.trim();
  if (!name) {
    showToast("Informe o nome do monstro", "error");
    return false;
  }

  const filename = state.monsters.selectedFilename || `${name.toLowerCase().replace(/[^a-z0-9]/g, '')}.mon`;

  const flags = [];
  document.querySelectorAll("input[name='mon-flag']:checked").forEach(chk => flags.push(chk.value));

  const payload = {
    filename: filename,
    name: name,
    race_number: parseInt(document.getElementById("mon-race").value) || 0,
    outfit_type: parseInt(document.getElementById("mon-outfit").value) || 0,
    outfit_colors: document.getElementById("mon-colors").value.trim() || "0-0-0-0",
    hitpoints: parseInt(document.getElementById("mon-hp").value) || 100,
    experience: parseInt(document.getElementById("mon-exp").value) || 0,
    attack: parseInt(document.getElementById("mon-atk").value) || 0,
    defend: parseInt(document.getElementById("mon-def").value) || 0,
    armor: parseInt(document.getElementById("mon-armor").value) || 0,
    speed: parseInt(document.getElementById("mon-speed").value) || 50,
    blood: document.getElementById("mon-blood").value || "Blood",
    corpse: parseInt(document.getElementById("mon-corpse").value) || 0,
    flee_threshold: parseInt(document.getElementById("mon-flee").value) || 0,
    poison: parseInt(document.getElementById("mon-poison").value) || 0,
    flags: flags,
    spells_raw: currentMonsterSpells,
    talk: currentMonsterTalk,
    inventory: currentMonsterLoot
  };

  try {
    const res = await fetch("/api/monsters/save", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const result = await res.json();
    if (result.success) {
      showToast(`${t("toast_monster_saved")} (${name})`, "success");
      setDirty(false);
      await loadMonsters();
      selectMonster(filename);
      return true;
    } else {
      showToast(result.error || "Erro ao salvar monstro", "error");
      return false;
    }
  } catch (err) {
    showToast("Erro ao salvar monstro", "error");
    return false;
  }
}

// =============================================================================
// ABA 3: NPCS (.NPC) - PALETA DE CORES TIBIA E CONSTRUTOR DE DIÁLOGOS
// =============================================================================
const TIBIA_PALETTE_HEX = [
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
];

const KNOWN_NDBS = [
  { file: "gen-bank.ndb", name: "🏦 Banco & Depósitos (gen-bank.ndb)" },
  { file: "gen-post.ndb", name: "📫 Correio & Parcels (gen-post.ndb)" },
  { file: "gen-t-runes-free-s.ndb", name: "📜 Runas Free (gen-t-runes-free-s.ndb)" },
  { file: "gen-t-runes-prem-s.ndb", name: "✨ Runas Premmy (gen-t-runes-prem-s.ndb)" },
  { file: "gen-t-wands-free-s.ndb", name: "🪄 Wands & Rods Free (gen-t-wands-free-s.ndb)" },
  { file: "gen-t-wands-prem-s.ndb", name: "🌟 Wands & Rods Premmy (gen-t-wands-prem-s.ndb)" },
  { file: "gen-t-weapon-s.ndb", name: "⚔️ Armas Gerais (gen-t-weapon-s.ndb)" },
  { file: "gen-t-armor-s.ndb", name: "🛡️ Armaduras (gen-t-armor-s.ndb)" },
  { file: "gen-t-helm-s.ndb", name: "🪖 Capacetes (gen-t-helm-s.ndb)" },
  { file: "gen-t-shield-s.ndb", name: "🛡️ Escudos (gen-t-shield-s.ndb)" },
  { file: "gen-t-legs-s.ndb", name: "👖 Calças / Legs (gen-t-legs-s.ndb)" },
  { file: "gen-t-fruit-s.ndb", name: "🍎 Frutas & Comida (gen-t-fruit-s.ndb)" },
  { file: "gen-t-furniture-chairs-s.ndb", name: "🪑 Móveis & Cadeiras (gen-t-furniture-chairs-s.ndb)" },
  { file: "gen-t-distance-s.ndb", name: "🏹 Arcos & Flechas (gen-t-distance-s.ndb)" },
  { file: "gen-t-gems-s.ndb", name: "💎 Gemas & Joias (gen-t-gems-s.ndb)" }
];

let activeNpcPalettePart = "head"; // "head" | "body" | "legs" | "feet"
let currentNpcKeywords = [];
let currentNpcNdbs = new Set();
let currentNpcActiveSecTab = "dialogues";

function parseOutfitColors(colorsStr) {
  if (!colorsStr) return { head: 0, body: 0, legs: 0, feet: 0 };
  const parts = String(colorsStr).match(/\d+/g) || [];
  return {
    head: parseInt(parts[0]) || 0,
    body: parseInt(parts[1]) || 0,
    legs: parseInt(parts[2]) || 0,
    feet: parseInt(parts[3]) || 0
  };
}

function formatOutfitColors(obj) {
  return `${obj.head}-${obj.body}-${obj.legs}-${obj.feet}`;
}

function initNpcColorPalette() {
  const container = document.getElementById("npc-palette-swatches");
  if (!container) return;

  container.innerHTML = TIBIA_PALETTE_HEX.map((hex, idx) => `
    <div class="palette-swatch" data-color-idx="${idx}" style="background-color: #${hex};" title="Cor #${idx}"></div>
  `).join("");

  container.querySelectorAll(".palette-swatch").forEach(el => {
    el.addEventListener("click", () => {
      const idx = parseInt(el.getAttribute("data-color-idx"));
      const curColors = parseOutfitColors(document.getElementById("npc-colors").value);
      
      curColors[activeNpcPalettePart] = idx;
      const newStr = formatOutfitColors(curColors);
      document.getElementById("npc-colors").value = newStr;

      setDirty(true);
      updateNpcSpritePreview();
      syncNpcPaletteSelection();
    });
  });

  const partTabs = [
    { id: "tab-pal-head", part: "head", label: "Head" },
    { id: "tab-pal-primary", part: "body", label: "Primary" },
    { id: "tab-pal-secondary", part: "legs", label: "Secondary" },
    { id: "tab-pal-detail", part: "feet", label: "Detail" }
  ];

  partTabs.forEach(({ id, part, label }) => {
    const btn = document.getElementById(id);
    if (btn) {
      btn.addEventListener("click", () => {
        activeNpcPalettePart = part;
        document.querySelectorAll(".tibia-palette-tabs .palette-tab").forEach(b => b.classList.remove("active"));
        btn.classList.add("active");
        
        const partLbl = document.getElementById("lbl-palette-active-part");
        if (partLbl) partLbl.innerText = label;

        syncNpcPaletteSelection();
      });
    }
  });

  const btnRandom = document.getElementById("btn-npc-random-colors");
  if (btnRandom) {
    btnRandom.addEventListener("click", () => {
      const rH = Math.floor(Math.random() * 133);
      const rB = Math.floor(Math.random() * 133);
      const rL = Math.floor(Math.random() * 133);
      const rF = Math.floor(Math.random() * 133);
      document.getElementById("npc-colors").value = `${rH}-${rB}-${rL}-${rF}`;
      setDirty(true);
      updateNpcSpritePreview();
      syncNpcPaletteSelection();
    });
  }

  const btnReset = document.getElementById("btn-npc-reset-colors");
  if (btnReset) {
    btnReset.addEventListener("click", () => {
      document.getElementById("npc-colors").value = "0-0-0-0";
      setDirty(true);
      updateNpcSpritePreview();
      syncNpcPaletteSelection();
    });
  }
}

function syncNpcPaletteSelection() {
  const colors = parseOutfitColors(document.getElementById("npc-colors").value);
  const activeVal = colors[activeNpcPalettePart] || 0;

  const valBadge = document.getElementById("lbl-palette-color-val");
  if (valBadge) valBadge.innerText = `#${activeVal}`;

  document.querySelectorAll("#npc-palette-swatches .palette-swatch").forEach(el => {
    const idx = parseInt(el.getAttribute("data-color-idx"));
    el.classList.toggle("active", idx === activeVal);
  });
}

function updateNpcSpritePreview() {
  const outfit = parseInt(document.getElementById("npc-outfit").value) || 128;
  const colors = document.getElementById("npc-colors").value.trim() || "0-0-0-0";
  const img = document.getElementById("npc-sprite-img");
  if (img) {
    img.src = `/api/outfit_sprite/${outfit}?zoom=3&colors=${encodeURIComponent(colors)}&t=${Date.now()}`;
  }
}

// ---- Diálogos & Comportamento de NPCs ----
function initNpcBehaviourTabs() {
  const secTabs = [
    { btnId: "btn-tab-npc-dialogues", paneId: "npc-pane-dialogues", key: "dialogues" },
    { btnId: "btn-tab-npc-ndb", paneId: "npc-pane-ndb", key: "ndb" },
    { btnId: "btn-tab-npc-raw", paneId: "npc-pane-raw", key: "raw" }
  ];

  secTabs.forEach(({ btnId, paneId, key }) => {
    const btn = document.getElementById(btnId);
    if (btn) {
      btn.addEventListener("click", () => {
        // Se estava saindo de visual, compila para raw
        if (currentNpcActiveSecTab !== "raw" && key === "raw") {
          generateNpcBehaviourFromUI();
        } else if (currentNpcActiveSecTab === "raw" && key !== "raw") {
          const rawLines = document.getElementById("npc-behaviour").value.split("\n").map(s => s.trim()).filter(Boolean);
          parseNpcBehaviourToUI(rawLines);
        }

        currentNpcActiveSecTab = key;
        document.querySelectorAll(".section-tabs-bar .sec-tab-btn").forEach(b => b.classList.remove("active"));
        btn.classList.add("active");

        document.querySelectorAll(".npc-behaviour-section .npc-sec-pane").forEach(p => p.style.display = "none");
        const pane = document.getElementById(paneId);
        if (pane) pane.style.display = "block";
      });
    }
  });

  // Checkboxes NDB
  const ndbContainer = document.getElementById("npc-ndb-checkboxes");
  if (ndbContainer) {
    ndbContainer.innerHTML = KNOWN_NDBS.map(mod => `
      <label class="ndb-module-chip">
        <input type="checkbox" data-ndb="${mod.file}" onchange="toggleNpcNdb('${mod.file}', this.checked)">
        <span>${mod.name}</span>
      </label>
    `).join("");
  }

  // Botão Adicionar Regra
  const btnAddKw = document.getElementById("btn-add-npc-keyword");
  if (btnAddKw) {
    btnAddKw.addEventListener("click", () => {
      addNpcKeywordRule({
        keywords: `"job"`,
        condition: "",
        response: "I am a helpful NPC.",
        actions: ""
      });
    });
  }
}

window.toggleNpcNdb = function(ndbFile, isChecked) {
  if (isChecked) {
    currentNpcNdbs.add(ndbFile);
  } else {
    currentNpcNdbs.delete(ndbFile);
  }
  setDirty(true);
  generateNpcBehaviourFromUI();
};

function renderNpcKeywordRulesTable() {
  const tbody = document.getElementById("npc-rules-tbody");
  if (!tbody) return;

  if (currentNpcKeywords.length === 0) {
    tbody.innerHTML = `<tr><td colspan="5" style="text-align:center;color:var(--text-muted);padding:1rem;">Nenhuma palavra-chave cadastrada. Clique em '+ Adicionar Regra' para criar respostas a palavras como 'job', 'name', 'offer', etc.</td></tr>`;
    return;
  }

  tbody.innerHTML = currentNpcKeywords.map((rule, idx) => `
    <tr>
      <td>
        <input type="text" value="${(rule.keywords || '').replace(/"/g, '&quot;')}" placeholder='"job" ou "life","fluid"' oninput="updateNpcKeywordRule(${idx}, 'keywords', this.value)">
      </td>
      <td>
        <input type="text" value="${(rule.condition || '').replace(/"/g, '&quot;')}" placeholder='Topic=1 ou CountMoney>=Price' oninput="updateNpcKeywordRule(${idx}, 'condition', this.value)">
      </td>
      <td>
        <input type="text" value="${(rule.response || '').replace(/"/g, '&quot;')}" placeholder='Texto falado pelo NPC' oninput="updateNpcKeywordRule(${idx}, 'response', this.value)">
      </td>
      <td>
        <input type="text" value="${(rule.actions || '').replace(/"/g, '&quot;')}" placeholder='Topic=1, DeleteMoney, etc.' oninput="updateNpcKeywordRule(${idx}, 'actions', this.value)">
      </td>
      <td style="text-align:center;">
        <button type="button" class="btn btn-xs btn-danger" onclick="removeNpcKeywordRule(${idx})">✕</button>
      </td>
    </tr>
  `).join("");
}

window.addNpcKeywordRule = function(rule) {
  currentNpcKeywords.push(rule);
  setDirty(true);
  renderNpcKeywordRulesTable();
  generateNpcBehaviourFromUI();
};

window.removeNpcKeywordRule = function(idx) {
  currentNpcKeywords.splice(idx, 1);
  setDirty(true);
  renderNpcKeywordRulesTable();
  generateNpcBehaviourFromUI();
};

window.updateNpcKeywordRule = function(idx, field, val) {
  if (currentNpcKeywords[idx]) {
    currentNpcKeywords[idx][field] = val;
    setDirty(true);
    generateNpcBehaviourFromUI();
  }
};

function parseNpcBehaviourToUI(lines) {
  let greeting = "";
  let farewell = "";
  let busy = "";
  let vanish = "";
  let aggressive = "";
  currentNpcKeywords = [];
  currentNpcNdbs = new Set();

  lines.forEach(line => {
    const s = line.trim();
    if (!s || s.startsWith("#")) return;

    if (s.startsWith("@")) {
      const match = s.match(/@"([^"]+)"/);
      if (match) currentNpcNdbs.add(match[1]);
      return;
    }

    if (s.startsWith("ADDRESS,") || s.startsWith("ADDRESS ")) {
      if (s.includes('"hello$"') || s.includes('"hi$"')) {
        const m = s.match(/->\s*"([^"]*)"/);
        if (m) greeting = m[1];
      }
      return;
    }

    if (s.startsWith("BUSY,") || s.startsWith("BUSY ")) {
      if (s.includes('"hello$"') || s.includes('"hi$"')) {
        const m = s.match(/->\s*"([^"]*)"/);
        if (m) busy = m[1];
      }
      return;
    }

    if (s.startsWith("VANISH,")) {
      const m = s.match(/->\s*"([^"]*)"/);
      if (m) vanish = m[1];
      return;
    }

    if (s.startsWith('"bye"') || s.startsWith('"farewell"')) {
      const m = s.match(/->\s*"([^"]*)"/);
      if (m) farewell = m[1];
      return;
    }

    if (s.startsWith("->")) {
      aggressive = s.replace(/^->\s*/, "");
      return;
    }

    // Regra de diálogo geral: Trigger -> Resposta / Ações
    if (s.includes("->")) {
      const [left, right] = s.split("->").map(p => p.trim());
      
      let condition = "";
      let keywords = left;
      
      // Se tiver Topic ou Condição no lado esquerdo
      if (left.includes("Topic=") || left.includes("Count") || left.includes("Price") || left.includes("Quest")) {
        const parts = left.split(",");
        const kwParts = [];
        const condParts = [];
        parts.forEach(p => {
          const pt = p.trim();
          if (pt.startsWith('"') || pt.startsWith("%")) {
            kwParts.push(pt);
          } else {
            condParts.push(pt);
          }
        });
        keywords = kwParts.join(", ");
        condition = condParts.join(", ");
      }

      // Extrai resposta entre aspas e ações no lado direito
      let responseText = "";
      let actionsText = "";
      
      const respMatch = right.match(/"([^"]*)"/);
      if (respMatch) {
        responseText = respMatch[1];
        actionsText = right.replace(`"${responseText}"`, "").replace(/^,\s*|,\s*$/g, "").trim();
      } else {
        actionsText = right;
      }

      currentNpcKeywords.push({
        keywords: keywords,
        condition: condition,
        response: responseText,
        actions: actionsText
      });
    }
  });

  // Atualiza campos do formulário
  const elGreet = document.getElementById("npc-greet-input");
  if (elGreet) elGreet.value = greeting;

  const elFarewell = document.getElementById("npc-farewell-input");
  if (elFarewell) elFarewell.value = farewell;

  const elBusy = document.getElementById("npc-busy-input");
  if (elBusy) elBusy.value = busy;

  const elVanish = document.getElementById("npc-vanish-input");
  if (elVanish) elVanish.value = vanish;

  const elAggressive = document.getElementById("npc-aggressive-input");
  if (elAggressive) elAggressive.value = aggressive;

  // Atualiza checkboxes NDB
  document.querySelectorAll("#npc-ndb-checkboxes input[type='checkbox']").forEach(chk => {
    const f = chk.getAttribute("data-ndb");
    chk.checked = currentNpcNdbs.has(f);
  });

  renderNpcKeywordRulesTable();
}

function generateNpcBehaviourFromUI() {
  const greeting = (document.getElementById("npc-greet-input")?.value || "").trim();
  const farewell = (document.getElementById("npc-farewell-input")?.value || "").trim();
  const busy = (document.getElementById("npc-busy-input")?.value || "").trim();
  const vanish = (document.getElementById("npc-vanish-input")?.value || "").trim();
  const aggressive = (document.getElementById("npc-aggressive-input")?.value || "").trim();

  const lines = [];

  // Fala agressiva / sem trigger
  if (aggressive) {
    lines.push(`-> ${aggressive}`);
  }

  // Saudações padrão GIMUD
  if (greeting) {
    lines.push(`ADDRESS,"hello$",! -> "${greeting}"`);
    lines.push(`ADDRESS,"hi$",!    -> *`);
    lines.push(`ADDRESS,!          -> Idle`);
  }

  if (busy) {
    lines.push(`BUSY,"hello$",!    -> "${busy}", Queue`);
    lines.push(`BUSY,"hi$",!       -> *`);
    lines.push(`BUSY,!             -> NOP`);
  }

  if (farewell) {
    lines.push(`"bye"      -> "${farewell}", Idle`);
    lines.push(`"farewell" -> *`);
  }

  // Regras customizadas de palavras-chave
  currentNpcKeywords.forEach(r => {
    const kw = (r.keywords || "").trim();
    const cond = (r.condition || "").trim();
    const resp = (r.response || "").trim();
    const act = (r.actions || "").trim();

    if (!kw && !cond && !resp && !act) return;

    let left = "";
    if (cond && kw) left = `${cond},${kw}`;
    else if (cond) left = cond;
    else left = kw;

    let rightParts = [];
    if (resp) rightParts.push(`"${resp}"`);
    if (act) rightParts.push(act);

    const right = rightParts.join(", ");
    if (left && right) {
      lines.push(`${left} -> ${right}`);
    }
  });

  // Módulos NDB incluídos
  currentNpcNdbs.forEach(ndb => {
    lines.push(`@"${ndb}"`);
  });

  const fullScript = lines.join("\n");
  const behTextarea = document.getElementById("npc-behaviour");
  if (behTextarea) {
    behTextarea.value = fullScript;
  }
  return fullScript;
}

function setupNpcsTab() {
  document.getElementById("btn-save-npc").addEventListener("click", saveCurrentNpc);
  document.getElementById("btn-new-npc").addEventListener("click", () => {
    confirmNavigation(() => {
      clearNpcForm();
      setDirty(false);
    });
  });

  document.getElementById("npc-outfit").addEventListener("input", updateNpcSpritePreview);
  document.getElementById("npc-colors").addEventListener("input", () => {
    updateNpcSpritePreview();
    syncNpcPaletteSelection();
  });

  // Inputs de diálogos disparam sincronização
  ["npc-greet-input", "npc-farewell-input", "npc-busy-input", "npc-vanish-input", "npc-aggressive-input"].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.addEventListener("input", generateNpcBehaviourFromUI);
  });

  initNpcColorPalette();
  initNpcBehaviourTabs();
}

async function loadNpcs() {
  try {
    const res = await fetch("/api/npcs");
    const data = await res.json();
    state.npcs.list = data.npcs;
    renderNpcsList();
  } catch (err) {
    console.error("Erro ao carregar NPCs:", err);
  }
}

function renderNpcsList() {
  const listContainer = document.getElementById("npcs-list");
  const q = state.npcs.search.toLowerCase();

  const filtered = state.npcs.list.filter(n => {
    if (!q) return true;
    return n.name.toLowerCase().includes(q) || n.filename.toLowerCase().includes(q);
  });

  listContainer.innerHTML = filtered.map(n => `
    <div class="list-item-card ${state.npcs.selectedFilename === n.filename ? 'selected' : ''}" data-file="${n.filename}">
      <div class="item-thumb">
        <img src="/api/outfit_sprite/${n.outfit_type}?zoom=1&colors=${encodeURIComponent(n.outfit_colors || '0-0-0-0')}" alt="${n.name}" loading="lazy">
      </div>
      <div class="item-info">
        <div class="item-name">${n.name}</div>
        <div class="item-meta">
          <span>${n.home_pos}</span>
          <span>${n.behaviour_count} ${t("lbl_lines")}</span>
        </div>
      </div>
    </div>
  `).join("");

  listContainer.querySelectorAll(".list-item-card").forEach(el => {
    el.addEventListener("click", () => {
      const filename = el.getAttribute("data-file");
      if (state.npcs.selectedFilename === filename) return;
      confirmNavigation(() => selectNpc(filename));
    });
  });

  if (filtered.length > 0 && state.npcs.selectedFilename === null) {
    selectNpc(filtered[0].filename);
  }
}

async function selectNpc(filename) {
  state.npcs.selectedFilename = filename;
  document.querySelectorAll("#npcs-list .list-item-card").forEach(c => {
    c.classList.toggle("selected", c.getAttribute("data-file") === filename);
  });

  try {
    const res = await fetch(`/api/npcs/${encodeURIComponent(filename)}`);
    if (!res.ok) return;
    const n = await res.json();

    document.getElementById("npc-title").innerText = `${t("npc_title_edit")} ${n.name} (${n.filename})`;
    document.getElementById("npc-name").value = n.name;
    document.getElementById("npc-sex").value = n.sex;
    document.getElementById("npc-outfit").value = n.outfit_type;
    document.getElementById("npc-home").value = n.home_pos;
    document.getElementById("npc-radius").value = n.radius;
    const colors = n.outfit_colors || "0-0-0-0";
    document.getElementById("npc-colors").value = colors;

    document.getElementById("npc-sprite-img").src = `/api/outfit_sprite/${n.outfit_type}?zoom=3&colors=${encodeURIComponent(colors)}&t=${Date.now()}`;
    document.getElementById("npc-behaviour").value = (n.behaviour_lines || []).join("\n");

    syncNpcPaletteSelection();
    parseNpcBehaviourToUI(n.behaviour_lines || []);
    setDirty(false);
  } catch (err) {
    console.error("Erro ao carregar detalhes do NPC:", err);
  }
}

function clearNpcForm() {
  state.npcs.selectedFilename = null;
  document.getElementById("npc-title").innerText = t("npc_title_create");
  document.getElementById("npc-name").value = "";
  document.getElementById("npc-sex").value = "male";
  document.getElementById("npc-outfit").value = "128";
  document.getElementById("npc-home").value = "[32000,32000,7]";
  document.getElementById("npc-radius").value = "4";
  document.getElementById("npc-colors").value = "0-0-0-0";
  
  const defaultLines = [
    `ADDRESS,"hello$",! -> "Hello adventurer %N!"`,
    `ADDRESS,"hi$",!    -> *`,
    `ADDRESS,!          -> Idle`,
    `"job" -> "I am an NPC."`,
    `"bye" -> "Good bye.", Idle`
  ];
  document.getElementById("npc-behaviour").value = defaultLines.join("\n");

  syncNpcPaletteSelection();
  parseNpcBehaviourToUI(defaultLines);
}

async function saveCurrentNpc() {
  const name = document.getElementById("npc-name").value.trim();
  if (!name) {
    showToast("Informe o nome do NPC", "error");
    return false;
  }

  // Se estiver em modo visual, compila para o textarea antes de salvar
  if (currentNpcActiveSecTab !== "raw") {
    generateNpcBehaviourFromUI();
  }

  const filename = state.npcs.selectedFilename || `${name.toLowerCase().replace(/[^a-z0-9]/g, '')}.npc`;
  const behLines = document.getElementById("npc-behaviour").value.split("\n").map(s => s.trim()).filter(Boolean);

  const payload = {
    filename: filename,
    name: name,
    sex: document.getElementById("npc-sex").value,
    outfit_type: parseInt(document.getElementById("npc-outfit").value) || 128,
    home_pos: document.getElementById("npc-home").value.trim(),
    radius: parseInt(document.getElementById("npc-radius").value) || 2,
    outfit_colors: document.getElementById("npc-colors").value.trim(),
    behaviour_lines: behLines
  };

  try {
    const res = await fetch("/api/npcs/save", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });
    const result = await res.json();
    if (result.success) {
      showToast(`${t("toast_npc_saved")} (${name})`, "success");
      setDirty(false);
      await loadNpcs();
      selectNpc(filename);
      return true;
    } else {
      showToast(result.error || "Erro ao salvar NPC", "error");
      return false;
    }
  } catch (err) {
    showToast("Erro ao salvar NPC", "error");
    return false;
  }
}

// =============================================================================
// ABA 4: MOVEUSE (MOVEUSE.DAT)
// =============================================================================
function setupMoveUseTab() {
  document.getElementById("btn-save-moveuse").addEventListener("click", saveMoveUse);

  document.getElementById("btn-add-moveuse-rule").addEventListener("click", () => {
    const sec = state.moveuse.sections[state.moveuse.selectedSectionIndex];
    const secName = sec ? sec.name : "Geral";
    openRuleBuilderForNew(secName);
  });
}

async function loadMoveUse() {
  try {
    const q = encodeURIComponent(state.moveuse.search);
    const res = await fetch(`/api/moveuse?q=${q}`);
    const data = await res.json();
    state.moveuse.sections = data.sections;
    renderMoveUseSections();
  } catch (err) {
    console.error("Erro ao carregar moveuse:", err);
  }
}

function renderMoveUseSections() {
  const container = document.getElementById("moveuse-sections-list");
  container.innerHTML = state.moveuse.sections.map((sec, idx) => `
    <div class="list-item-card ${state.moveuse.selectedSectionIndex === idx ? 'selected' : ''}" data-idx="${idx}">
      <div class="item-info">
        <div class="item-name">${sec.name}</div>
        <div class="item-meta">
          <span>${sec.count} ${t("lbl_rules")}</span>
        </div>
      </div>
    </div>
  `).join("");

  container.querySelectorAll(".list-item-card").forEach(el => {
    el.addEventListener("click", () => {
      const idx = parseInt(el.getAttribute("data-idx"));
      if (state.moveuse.selectedSectionIndex === idx) return;
      confirmNavigation(() => {
        state.moveuse.selectedSectionIndex = idx;
        document.querySelectorAll("#moveuse-sections-list .list-item-card").forEach(c => c.classList.remove("selected"));
        el.classList.add("selected");
        renderMoveUseRules();
      });
    });
  });

  renderMoveUseRules();
}

function renderMoveUseRules() {
  const container = document.getElementById("moveuse-rules-list");
  const sec = state.moveuse.sections[state.moveuse.selectedSectionIndex];

  if (!sec) {
    container.innerHTML = `<p style='color: var(--text-muted);'>${t("no_section_selected")}</p>`;
    return;
  }

  document.getElementById("moveuse-section-title").innerText = `${t("moveuse_rules_of_section")} ${sec.name}`;

  container.innerHTML = sec.rules.map((r, rIdx) => `
    <div class="rule-card" onclick="openRuleBuilderForEdit(${rIdx})">
      <div class="rule-visual">
        <span class="rule-badge">${r.trigger || "Regra"}</span>
        <div class="rule-code">${r.raw}</div>
      </div>
      <div class="rule-item-thumbs">
        ${(r.item_ids || []).slice(0, 3).map(id => `
          <img src="/api/item_sprite/${id}?zoom=1" style="width:24px;height:24px;image-rendering:pixelated;" title="Item #${id}">
        `).join("")}
      </div>
      <button class="btn btn-xs btn-danger" onclick="event.stopPropagation(); deleteMoveUseRule(${rIdx})">✕</button>
    </div>
  `).join("");
}

window.deleteMoveUseRule = function(rIdx) {
  const sec = state.moveuse.sections[state.moveuse.selectedSectionIndex];
  if (sec) {
    sec.rules.splice(rIdx, 1);
    sec.count = sec.rules.length;
    setDirty(true);
    renderMoveUseRules();
  }
};

async function saveMoveUse() {
  try {
    const res = await fetch("/api/moveuse/save", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ sections: state.moveuse.sections })
    });
    const result = await res.json();
    if (result.success) {
      showToast(t("toast_moveuse_saved"), "success");
      setDirty(false);
      return true;
    } else {
      showToast("Erro ao salvar moveuse.dat", "error");
      return false;
    }
  } catch (err) {
    showToast("Erro ao salvar moveuse.dat", "error");
    return false;
  }
}

// =============================================================================
// CONSTRUTOR VISUAL DE REGRAS CIPSOFT (MOVE/USE BUILDER)
// =============================================================================
function setupRuleBuilder() {
  const modal = document.getElementById("rule-builder-modal");
  document.getElementById("btn-close-builder").addEventListener("click", () => modal.classList.remove("active"));
  document.getElementById("btn-cancel-builder").addEventListener("click", () => modal.classList.remove("active"));

  document.getElementById("builder-event-type").addEventListener("change", (e) => {
    state.builder.eventType = e.target.value;
    updateBuilderPreview();
  });

  // Dropdown de Condições
  const condSelect = document.getElementById("select-add-condition");
  condSelect.addEventListener("change", (e) => {
    const condId = e.target.value;
    if (!condId) return;
    const def = state.docs.catalog.conditions.find(c => c.id === condId);
    if (def) {
      const initialParams = {};
      (def.params || []).forEach(p => initialParams[p.name] = p.default);
      state.builder.conditions.push({
        id: def.id,
        name: def.name,
        params: initialParams,
        template: def.template,
        invert: false
      });
      renderBuilderConditions();
      updateBuilderPreview();
    }
    condSelect.value = "";
  });

  // Dropdown de Ações
  const actSelect = document.getElementById("select-add-action");
  actSelect.addEventListener("change", (e) => {
    const actId = e.target.value;
    if (!actId) return;
    const def = state.docs.catalog.actions.find(a => a.id === actId);
    if (def) {
      const initialParams = {};
      (def.params || []).forEach(p => initialParams[p.name] = p.default);
      state.builder.actions.push({
        id: def.id,
        name: def.name,
        params: initialParams,
        template: def.template
      });
      renderBuilderActions();
      updateBuilderPreview();
    }
    actSelect.value = "";
  });

  document.getElementById("btn-confirm-rule").addEventListener("click", saveBuilderRule);
}

function populateSectionsDatalist() {
  const datalist = document.getElementById("builder-sections-datalist");
  if (datalist) {
    const existing = state.moveuse.sections.map(s => s.name);
    datalist.innerHTML = existing.map(name => `<option value="${name}"></option>`).join("");
  }
}

function openRuleBuilderForNew(sectionName) {
  state.builder.editingSection = sectionName;
  state.builder.editingRuleIndex = null;
  state.builder.eventType = "Use";
  state.builder.conditions = [
    {
      id: "IsType",
      name: "IsType",
      params: { Target: "Obj1", TypeID: 100 },
      template: "IsType ({Target},{TypeID})",
      invert: false
    }
  ];
  state.builder.actions = [
    {
      id: "Effect",
      name: "Effect",
      params: { Target: "Obj1", EffectID: 3 },
      template: "Effect({Target},{EffectID})"
    }
  ];

  document.getElementById("builder-modal-title").innerText = `⚡ ${t("builder_modal_title")} [${sectionName}]`;
  document.getElementById("builder-section-name").value = sectionName;
  document.getElementById("builder-event-type").value = "Use";

  populateSectionsDatalist();
  renderBuilderConditions();
  renderBuilderActions();
  updateBuilderPreview();

  document.getElementById("rule-builder-modal").classList.add("active");
}

function openRuleBuilderForEdit(rIdx) {
  const sec = state.moveuse.sections[state.moveuse.selectedSectionIndex];
  if (!sec || !sec.rules[rIdx]) return;
  const r = sec.rules[rIdx];

  state.builder.editingSection = sec.name;
  state.builder.editingRuleIndex = rIdx;
  state.builder.eventType = r.trigger || "Use";

  document.getElementById("builder-modal-title").innerText = `⚡ ${t("builder_modal_title")} [${sec.name}]`;
  document.getElementById("builder-section-name").value = sec.name;
  document.getElementById("builder-event-type").value = r.trigger || "Use";

  populateSectionsDatalist();

  // Parse conditions
  state.builder.conditions = [];
  if (r.conditions) {
    const rawConds = r.conditions.split(/,\s*(?=[A-Za-z])/);
    rawConds.forEach(cStr => {
      state.builder.conditions.push({
        id: "Custom",
        name: "Condição",
        raw: cStr.trim(),
        template: cStr.trim()
      });
    });
  }

  // Parse actions
  state.builder.actions = [];
  if (r.actions) {
    const rawActs = r.actions.split(/,\s*(?=[A-Za-z])/);
    rawActs.forEach(aStr => {
      state.builder.actions.push({
        id: "Custom",
        name: "Ação",
        raw: aStr.trim(),
        template: aStr.trim()
      });
    });
  }

  renderBuilderConditions();
  renderBuilderActions();
  updateBuilderPreview();

  document.getElementById("rule-builder-modal").classList.add("active");
}

function renderBuilderConditions() {
  const container = document.getElementById("builder-conditions-container");
  if (state.builder.conditions.length === 0) {
    container.innerHTML = `<div style="color:var(--text-muted);font-size:0.8rem;padding:6px;">${t("no_cond_added")}</div>`;
    return;
  }

  container.innerHTML = state.builder.conditions.map((c, idx) => {
    if (c.raw) {
      return `
        <div class="builder-chip-row">
          <span class="builder-chip-name">${t("custom_chip")}</span>
          <div class="builder-chip-inputs">
            <input type="text" value="${c.raw}" oninput="state.builder.conditions[${idx}].raw = this.value; updateBuilderPreview();" style="width:100%;">
          </div>
          <button class="btn btn-xs btn-danger" onclick="removeBuilderCondition(${idx})">✕</button>
        </div>
      `;
    }

    const def = state.docs.catalog.conditions.find(cd => cd.id === c.id);
    const inputsHtml = (def ? def.params : []).map(p => {
      const val = c.params[p.name] !== undefined ? c.params[p.name] : p.default;
      if (p.type === "item_id" || p.name === "TypeID" || p.name === "ObjType") {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <input type="number" style="width:80px;" value="${val}" oninput="state.builder.conditions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
            <button class="btn-builder-picker" title="${t("choose_item_btn")}" onclick="openItemPickerModal('${t("modal_pick_cond_item")}', (item) => { state.builder.conditions[${idx}].params['${p.name}'] = item.type_id; renderBuilderConditions(); updateBuilderPreview(); })">
              <img src="/api/item_sprite/${val || 100}?zoom=1" style="width:18px;height:18px;image-rendering:pixelated;">
            </button>
          </div>
        `;
      } else if (p.type === "target") {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <select onchange="state.builder.conditions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
              <option value="Obj1" ${val === 'Obj1' ? 'selected' : ''}>Obj1 (Item)</option>
              <option value="Obj2" ${val === 'Obj2' ? 'selected' : ''}>Obj2 (Alvo)</option>
              <option value="User" ${val === 'User' ? 'selected' : ''}>User (Player)</option>
            </select>
          </div>
        `;
      } else {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <input type="text" value="${val}" oninput="state.builder.conditions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
          </div>
        `;
      }
    }).join("");

    return `
      <div class="builder-chip-row">
        <span class="builder-chip-name">${c.id}</span>
        <div class="builder-chip-inputs">
          ${inputsHtml}
        </div>
        <button class="btn btn-xs btn-danger" onclick="removeBuilderCondition(${idx})">✕</button>
      </div>
    `;
  }).join("");
}

function renderBuilderActions() {
  const container = document.getElementById("builder-actions-container");
  if (state.builder.actions.length === 0) {
    container.innerHTML = `<div style="color:var(--text-muted);font-size:0.8rem;padding:6px;">${t("no_act_added")}</div>`;
    return;
  }

  container.innerHTML = state.builder.actions.map((a, idx) => {
    if (a.raw) {
      return `
        <div class="builder-chip-row">
          <span class="builder-chip-name">${t("custom_chip")}</span>
          <div class="builder-chip-inputs">
            <input type="text" value="${a.raw}" oninput="state.builder.actions[${idx}].raw = this.value; updateBuilderPreview();" style="width:100%;">
          </div>
          <button class="btn btn-xs btn-danger" onclick="removeBuilderAction(${idx})">✕</button>
        </div>
      `;
    }

    const def = state.docs.catalog.actions.find(ad => ad.id === a.id);
    const inputsHtml = (def ? def.params : []).map(p => {
      const val = a.params[p.name] !== undefined ? a.params[p.name] : p.default;

      // Seletor de Efeitos Mágicos
      if (p.name === "EffectID" || p.name === "Effect" || a.id === "Effect") {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <input type="number" style="width:70px;" value="${val}" min="1" max="25" oninput="state.builder.actions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
            <button class="btn-builder-picker" title="${t("choose_effect_btn")}" onclick="openEffectPickerModal((eff) => { state.builder.actions[${idx}].params['${p.name}'] = eff.id; renderBuilderActions(); updateBuilderPreview(); })">
              <img src="/api/effect_sprite/${val || 1}?zoom=1" style="width:18px;height:18px;image-rendering:pixelated;">
              <span>✨</span>
            </button>
          </div>
        `;
      }
      // Seletor de Itens
      else if (p.type === "item_id" || p.name === "TypeID" || p.name === "NewTypeID" || p.name === "ObjType") {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <input type="number" style="width:80px;" value="${val}" oninput="state.builder.actions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
            <button class="btn-builder-picker" title="${t("choose_item_btn")}" onclick="openItemPickerModal('${t("modal_pick_act_item")}', (item) => { state.builder.actions[${idx}].params['${p.name}'] = item.type_id; renderBuilderActions(); updateBuilderPreview(); })">
              <img src="/api/item_sprite/${val || 100}?zoom=1" style="width:18px;height:18px;image-rendering:pixelated;">
            </button>
          </div>
        `;
      } else if (p.type === "target") {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <select onchange="state.builder.actions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
              <option value="Obj1" ${val === 'Obj1' ? 'selected' : ''}>Obj1 (Item)</option>
              <option value="Obj2" ${val === 'Obj2' ? 'selected' : ''}>Obj2 (Alvo)</option>
              <option value="User" ${val === 'User' ? 'selected' : ''}>User (Player)</option>
            </select>
          </div>
        `;
      } else {
        return `
          <div style="display:flex;align-items:center;gap:4px;">
            <span>${p.name}:</span>
            <input type="text" value="${val}" oninput="state.builder.actions[${idx}].params['${p.name}'] = this.value; updateBuilderPreview();">
          </div>
        `;
      }
    }).join("");

    return `
      <div class="builder-chip-row">
        <span class="builder-chip-name">${a.id}</span>
        <div class="builder-chip-inputs">
          ${inputsHtml}
        </div>
        <button class="btn btn-xs btn-danger" onclick="removeBuilderAction(${idx})">✕</button>
      </div>
    `;
  }).join("");
}

window.removeBuilderCondition = function(idx) {
  state.builder.conditions.splice(idx, 1);
  renderBuilderConditions();
  updateBuilderPreview();
};

window.removeBuilderAction = function(idx) {
  state.builder.actions.splice(idx, 1);
  renderBuilderActions();
  updateBuilderPreview();
};

function generateRuleCode() {
  const trigger = state.builder.eventType;

  const condStrings = state.builder.conditions.map(c => {
    if (c.raw) return c.raw;
    let str = c.template || "";
    for (const [k, v] of Object.entries(c.params || {})) {
      str = str.replace(`{${k}}`, v);
    }
    return str;
  });

  const actStrings = state.builder.actions.map(a => {
    if (a.raw) return a.raw;
    let str = a.template || "";
    for (const [k, v] of Object.entries(a.params || {})) {
      str = str.replace(`{${k}}`, v);
    }
    return str;
  });

  const condPart = condStrings.length > 0 ? `, ${condStrings.join(", ")}` : "";
  const actPart = actStrings.length > 0 ? actStrings.join(", ") : "NOP";

  return `${trigger}${condPart} -> ${actPart}`;
}

function updateBuilderPreview() {
  const code = generateRuleCode();
  const out = document.getElementById("builder-code-output");
  if (out) out.innerText = code;
}

async function saveBuilderRule() {
  const secName = document.getElementById("builder-section-name").value.trim() || "Geral";
  const rawCode = generateRuleCode();

  // Encontra ou cria a seção
  let targetSec = state.moveuse.sections.find(s => s.name.toLowerCase() === secName.toLowerCase());
  if (!targetSec) {
    targetSec = { name: secName, count: 0, rules: [] };
    state.moveuse.sections.push(targetSec);
  }

  // Extrai IDs de itens da linha para visualização
  const foundIds = (rawCode.match(/\b\d{3,5}\b/g) || []).map(Number);
  const uniqueIds = [...new Set(foundIds)];

  const ruleObj = {
    id: Date.now(),
    section: targetSec.name,
    trigger: state.builder.eventType,
    conditions: "",
    actions: "",
    raw: rawCode,
    item_ids: uniqueIds
  };

  if (state.builder.editingRuleIndex !== null && state.builder.editingSection === targetSec.name) {
    targetSec.rules[state.builder.editingRuleIndex] = ruleObj;
  } else {
    targetSec.rules.unshift(ruleObj);
  }

  targetSec.count = targetSec.rules.length;

  document.getElementById("rule-builder-modal").classList.remove("active");
  renderMoveUseSections();
  await saveMoveUse();
  showToast(t("toast_moveuse_saved"), "success");
}

// =============================================================================
// ABA 6: DICIONÁRIO DE FUNÇÕES CIPSOFT (DOCUMENTATION)
// =============================================================================
function setupDocsTab() {
  const filterBtns = document.querySelectorAll("[data-doc-filter]");
  filterBtns.forEach(btn => {
    btn.addEventListener("click", () => {
      filterBtns.forEach(b => b.classList.remove("active"));
      btn.classList.add("active");
      state.docs.filter = btn.getAttribute("data-doc-filter");
      renderDocsGrid();
    });
  });

  const searchInp = document.getElementById("docs-search-input");
  searchInp.addEventListener("input", (e) => {
    state.docs.search = e.target.value.trim().toLowerCase();
    renderDocsGrid();
  });
}

async function loadDocs() {
  try {
    const lang = state.lang || "pt_BR";
    const res = await fetch(`/api/moveuse/docs?lang=${lang}`);
    const data = await res.json();
    state.docs.catalog = data;

    // Popula os selects do Builder
    const condSelect = document.getElementById("select-add-condition");
    if (condSelect) {
      condSelect.innerHTML = `<option value="">${t("add_condition_opt")}</option>` +
        (data.conditions || []).map(c => `<option value="${c.id}">${c.name}</option>`).join("");
    }

    const actSelect = document.getElementById("select-add-action");
    if (actSelect) {
      actSelect.innerHTML = `<option value="">${t("add_action_opt")}</option>` +
        (data.actions || []).map(a => `<option value="${a.id}">${a.name}</option>`).join("");
    }

    renderDocsGrid();
  } catch (err) {
    console.error("Erro ao carregar dicionário de funções:", err);
  }
}

function renderDocsGrid() {
  const container = document.getElementById("docs-grid-container");
  if (!container) return;

  const { catalog, filter, search } = state.docs;
  let items = [];

  if (filter === "all" || filter === "events") {
    items.push(...(catalog.events || []).map(e => ({ ...e, kind: "event", kindLabel: t("badge_event_label") })));
  }
  if (filter === "all" || filter === "conditions") {
    items.push(...(catalog.conditions || []).map(c => ({ ...c, kind: "condition", kindLabel: t("badge_condition_label") })));
  }
  if (filter === "all" || filter === "actions") {
    items.push(...(catalog.actions || []).map(a => ({ ...a, kind: "action", kindLabel: t("badge_action_label") })));
  }

  if (search) {
    items = items.filter(it =>
      it.name.toLowerCase().includes(search) ||
      (it.id && it.id.toLowerCase().includes(search)) ||
      (it.desc && it.desc.toLowerCase().includes(search)) ||
      (it.example && it.example.toLowerCase().includes(search))
    );
  }

  container.innerHTML = items.map(it => {
    const paramsListHtml = (it.params || []).map(p => `
      <dt>${p.name} <span style="font-weight:normal;color:var(--text-muted);">(${p.type})</span></dt>
      <dd>${p.desc}</dd>
    `).join("");

    return `
      <div class="doc-card">
        <div class="doc-card-header">
          <span class="doc-card-title">${it.name}</span>
          <span class="doc-category-badge ${it.kind}">${it.kindLabel} • ${it.category}</span>
        </div>
        <p class="doc-desc">${it.desc}</p>
        ${it.params && it.params.length > 0 ? `
          <div class="doc-params-list">
            <div style="font-weight:700;color:var(--text-accent);margin-bottom:4px;">${t("lbl_parameters")}</div>
            <dl>${paramsListHtml}</dl>
          </div>
        ` : ''}
        ${it.example ? `
          <div class="doc-example-box">
            <strong>${t("lbl_example")}</strong> <code>${it.example}</code>
          </div>
        ` : ''}
      </div>
    `;
  }).join("");
}

// =============================================================================
// ABA 5: SPRITES BROWSER (PAGINADO COM 250 POR PÁGINA)
// =============================================================================
function setupSpritesTab() {
  document.querySelectorAll(".btn-zoom").forEach(btn => {
    btn.addEventListener("click", () => {
      document.querySelectorAll(".btn-zoom").forEach(b => b.classList.remove("active"));
      btn.classList.add("active");
      state.sprites.currentZoom = parseInt(btn.getAttribute("data-zoom")) || 1;
      renderSpritesGrid();
    });
  });

  document.getElementById("btn-go-sprite").addEventListener("click", () => {
    const searchId = parseInt(document.getElementById("sprite-id-search").value);
    if (!isNaN(searchId) && searchId > 0 && searchId <= state.sprites.total) {
      state.sprites.page = Math.ceil(searchId / state.sprites.limit) || 1;
      renderSpritesGrid();
      setTimeout(() => {
        const el = document.getElementById(`sprite-card-${searchId}`);
        if (el) {
          el.scrollIntoView({ behavior: "smooth", block: "center" });
          el.classList.add("sprite-highlight");
          setTimeout(() => el.classList.remove("sprite-highlight"), 2500);
        }
      }, 100);
    } else {
      showToast(t("toast_invalid_sprite_id"), "error");
    }
  });

  // Botões de Paginação
  document.getElementById("btn-sprites-first").addEventListener("click", () => {
    if (state.sprites.page > 1) {
      state.sprites.page = 1;
      renderSpritesGrid();
    }
  });

  document.getElementById("btn-sprites-prev").addEventListener("click", () => {
    if (state.sprites.page > 1) {
      state.sprites.page--;
      renderSpritesGrid();
    }
  });

  document.getElementById("btn-sprites-next").addEventListener("click", () => {
    const totalPages = Math.ceil(state.sprites.total / state.sprites.limit) || 1;
    if (state.sprites.page < totalPages) {
      state.sprites.page++;
      renderSpritesGrid();
    }
  });

  document.getElementById("btn-sprites-last").addEventListener("click", () => {
    const totalPages = Math.ceil(state.sprites.total / state.sprites.limit) || 1;
    if (state.sprites.page < totalPages) {
      state.sprites.page = totalPages;
      renderSpritesGrid();
    }
  });

  document.getElementById("btn-sprites-jump").addEventListener("click", () => {
    const p = parseInt(document.getElementById("sprites-page-input").value);
    const totalPages = Math.ceil(state.sprites.total / state.sprites.limit) || 1;
    if (!isNaN(p) && p >= 1 && p <= totalPages) {
      state.sprites.page = p;
      renderSpritesGrid();
    }
  });

  document.getElementById("sprites-page-input").addEventListener("keydown", (e) => {
    if (e.key === "Enter") {
      document.getElementById("btn-sprites-jump").click();
    }
  });
}

function renderSpritesGrid() {
  const container = document.getElementById("sprites-browser-grid");
  const total = state.sprites.total || 10962;
  const limit = state.sprites.limit || 250;
  const totalPages = Math.ceil(total / limit) || 1;
  const page = Math.max(1, Math.min(state.sprites.page || 1, totalPages));
  state.sprites.page = page;

  const startId = (page - 1) * limit + 1;
  const endId = Math.min(total, startId + limit - 1);

  // Atualiza controles da barra
  document.getElementById("sprites-page-input").value = page;
  document.getElementById("sprites-page-total").innerText = `${t("lbl_of")} ${totalPages}`;
  document.getElementById("sprites-range-info").innerText = `${t("lbl_showing_sprites")} ${startId} - ${endId} ${t("lbl_of")} ${total.toLocaleString()}`;

  document.getElementById("btn-sprites-first").disabled = (page === 1);
  document.getElementById("btn-sprites-prev").disabled = (page === 1);
  document.getElementById("btn-sprites-next").disabled = (page === totalPages);
  document.getElementById("btn-sprites-last").disabled = (page === totalPages);

  const zoom = state.sprites.currentZoom || 1;
  const cellSizes = { 1: 88, 2: 112, 4: 168 };
  container.style.setProperty("--sprite-cell", `${cellSizes[zoom] || 88}px`);
  const cards = [];

  for (let id = startId; id <= endId; id++) {
    cards.push(`
      <div class="sprite-grid-item" id="sprite-card-${id}" onclick="copySpriteId(${id})" title="Sprite #${id}">
        <div class="sprite-thumb">
          <img src="/api/sprite/${id}?zoom=${zoom}" alt="#${id}" loading="lazy">
        </div>
        <span class="sprite-id-tag">#${id}</span>
      </div>
    `);
  }

  container.innerHTML = cards.join("");
  container.scrollTop = 0;
}

window.copySpriteId = function(id) {
  navigator.clipboard.writeText(String(id));
  showToast(`${t("toast_sprite_copied")}: #${id}`, "info");
};

// =============================================================================
// ABA 7: CONFIGURAÇÕES
// =============================================================================
async function saveConfig() {
  const dataPath = document.getElementById("cfg-data-path").value.trim();
  const clientPath = document.getElementById("cfg-client-path").value.trim();

  try {
    const res = await fetch("/api/config/load", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ data_path: dataPath, client_path: clientPath })
    });
    const data = await res.json();
    showToast(t("toast_config_saved"), "success");
    setDirty(false);
    await loadInitialConfig();
    await loadObjects();
    await loadMonsters();
    await loadNpcs();
    await loadMoveUse();
    await loadEffects();
    return true;
  } catch (err) {
    showToast("Erro ao carregar diretórios", "error");
    return false;
  }
}

function setupSettingsTab() {
  document.getElementById("btn-save-config").addEventListener("click", saveConfig);
}

// =============================================================================
// MODAIS: SELETOR DE ITENS E SELETOR DE EFEITOS MÁGICOS
// =============================================================================
let activePickerCallback = null;
let activeEffectCallback = null;
let cachedEffectsList = [];

function setupModals() {
  // Modal de Itens
  const itemModal = document.getElementById("item-picker-modal");
  document.getElementById("btn-close-picker").addEventListener("click", () => {
    itemModal.classList.remove("active");
  });

  const searchItemInput = document.getElementById("modal-item-search");
  searchItemInput.addEventListener("input", (e) => {
    loadModalItems(e.target.value.trim());
  });

  // Modal de Efeitos Mágicos
  const effectModal = document.getElementById("effect-picker-modal");
  document.getElementById("btn-close-effect-picker").addEventListener("click", () => {
    effectModal.classList.remove("active");
  });

  const searchEffectInput = document.getElementById("modal-effect-search");
  searchEffectInput.addEventListener("input", (e) => {
    renderEffectsPickerGrid(e.target.value.trim());
  });
}

// ---- Seletor de Itens ----
let activePickerFlags = "";

function openItemPickerModal(title, callback, flags) {
  activePickerCallback = callback;
  activePickerFlags = flags || "";
  document.getElementById("modal-picker-title").innerText = title;
  document.getElementById("item-picker-modal").classList.add("active");
  document.getElementById("modal-item-search").value = "";
  loadModalItems("");
}

async function loadModalItems(query) {
  try {
    const res = await fetch(`/api/objects?limit=60&q=${encodeURIComponent(query)}&flag=${encodeURIComponent(activePickerFlags)}`);
    const data = await res.json();
    const container = document.getElementById("modal-items-container");

    container.innerHTML = data.items.map(item => `
      <button class="modal-item-btn" onclick="selectModalItem(${item.type_id}, '${(item.name || '').replace(/'/g, "\\'")}')">
        <img src="/api/item_sprite/${item.type_id}?zoom=1" style="width:28px;height:28px;image-rendering:pixelated;">
        <div>
          <div style="font-weight:600;font-size:0.8rem;">${item.name || t("obj_unnamed")}</div>
          <div style="font-size:0.7rem;color:var(--text-muted);">#${item.type_id}</div>
        </div>
      </button>
    `).join("");
  } catch (err) {
    console.error("Erro ao carregar itens no modal:", err);
  }
}

window.selectModalItem = function(typeId, name) {
  if (activePickerCallback) {
    activePickerCallback({ type_id: typeId, name: name });
  }
  document.getElementById("item-picker-modal").classList.remove("active");
};

// ---- Seletor de Efeitos Mágicos ----
async function loadEffects() {
  try {
    const res = await fetch("/api/effects");
    const data = await res.json();
    cachedEffectsList = data.effects || [];
  } catch (err) {
    console.error("Erro ao carregar efeitos mágicos:", err);
  }
}

function openEffectPickerModal(callback) {
  activeEffectCallback = callback;
  document.getElementById("modal-effect-picker-title").innerText = t("modal_effect_picker_title");
  document.getElementById("effect-picker-modal").classList.add("active");
  document.getElementById("modal-effect-search").value = "";
  renderEffectsPickerGrid("");
}

function renderEffectsPickerGrid(query) {
  const container = document.getElementById("modal-effects-container");
  const q = (query || "").toLowerCase().trim();

  const filtered = cachedEffectsList.filter(eff => {
    if (!q) return true;
    return String(eff.id).includes(q) || eff.name.toLowerCase().includes(q) || eff.desc.toLowerCase().includes(q);
  });

  container.innerHTML = filtered.map(eff => `
    <div class="effect-select-card" onclick="selectModalEffect(${eff.id}, '${eff.name.replace(/'/g, "\\'")}')">
      <div class="effect-thumb-box">
        <img src="/api/effect_sprite/${eff.id}?zoom=1" alt="${eff.name}">
      </div>
      <div class="effect-meta">
        <span class="effect-name">${eff.name}</span>
        <span class="effect-id-badge">EffectID: ${eff.id}</span>
      </div>
    </div>
  `).join("");
}

window.selectModalEffect = function(effectId, name) {
  if (activeEffectCallback) {
    activeEffectCallback({ id: effectId, name: name });
  }
  document.getElementById("effect-picker-modal").classList.remove("active");
};

// =============================================================================
// TOAST NOTIFICATIONS
// =============================================================================
function showToast(message, type = "info") {
  const container = document.getElementById("toast-container");
  const toast = document.createElement("div");
  toast.className = `toast ${type}`;
  toast.innerText = message;

  container.appendChild(toast);
  setTimeout(() => {
    toast.style.opacity = "0";
    setTimeout(() => toast.remove(), 200);
  }, 3000);
}


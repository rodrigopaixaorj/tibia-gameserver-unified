"""
Tibia Data Studio - Inicializador Automático
Inicia o servidor backend e abre o navegador padrão automaticamente.
"""

import os
import sys
import threading
import time
import webbrowser

# Garante que o diretório base esteja correto
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, BASE_DIR)

from app import app, auto_load_data

def open_browser(port):
    time.sleep(1.2)
    url = f"http://localhost:{port}"
    print(f"[Tibia Data Studio] Abrindo navegador em: {url}")
    webbrowser.open(url)

if __name__ == "__main__":
    auto_load_data()
    port = 8080
    
    # Inicia a abertura do navegador em segundo plano
    threading.Thread(target=open_browser, args=(port,), daemon=True).start()
    
    print("=" * 65)
    print("   ⚔️  TIBIA DATA STUDIO - EDITOR UNIFICADO DE DADOS")
    print(f"   🌐  Painel disponível em: http://localhost:{port}")
    print("   🛡️  Sistema de backups com versionamento automático ATIVO")
    print("=" * 65)
    print("Pressione CTRL + C para encerrar o editor.\n")
    
    app.run(host="127.0.0.1", port=port, debug=False)

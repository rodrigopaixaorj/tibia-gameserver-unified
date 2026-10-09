"""
Gerenciador de Backups e Segurança de Dados
Cria cópias com timestamp antes de qualquer gravação e previne corrupção de arquivos.
"""

import os
import shutil
from datetime import datetime

class BackupManager:
    def __init__(self, backup_root: str = None):
        self.backup_root = backup_root

    def create_backup(self, original_file_path: str) -> str:
        """Cria uma cópia segura do arquivo antes de sobrescrevê-lo"""
        if not os.path.exists(original_file_path):
            return ""

        file_dir = os.path.dirname(os.path.abspath(original_file_path))
        file_name = os.path.basename(original_file_path)
        
        backup_dir = os.path.join(file_dir, ".backups")
        os.makedirs(backup_dir, exist_ok=True)

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_filename = f"{file_name}.{timestamp}.bak"
        backup_path = os.path.join(backup_dir, backup_filename)

        shutil.copy2(original_file_path, backup_path)
        return backup_path

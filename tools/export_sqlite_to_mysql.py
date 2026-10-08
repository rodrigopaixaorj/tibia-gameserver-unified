#!/usr/bin/env python3
"""
Tibia 7.72 SQLite to MySQL / MariaDB Exporter & Migrator
Exports all tables, accounts, characters, rights, and house owners from SQLite (tibia.db)
to a complete MySQL / MariaDB SQL dump file (tibia_mysql_full.sql).
"""

import sqlite3
import os
import sys

def escape_sql_val(val):
    if val is None:
        return "NULL"
    if isinstance(val, (bytes, bytearray)):
        return f"UNHEX('{val.hex()}')"
    if isinstance(val, (int, float)):
        return str(val)
    val_str = str(val).replace("\\", "\\\\").replace("'", "\\'").replace("\0", "")
    return f"'{val_str}'"

def export_db(sqlite_file="tibia.db", output_sql="tibia_mysql_full.sql", schema_file="schema_mysql.sql"):
    if not os.path.exists(sqlite_file):
        print(f"[-] Error: SQLite database '{sqlite_file}' not found.")
        return False

    print(f"[*] Reading SQLite database: {sqlite_file}")
    con = sqlite3.connect(sqlite_file)
    cur = con.cursor()

    print(f"[*] Generating MySQL dump: {output_sql}")
    with open(output_sql, "w", encoding="utf-8") as f:
        f.write("-- ==============================================================================\n")
        f.write("-- Tibia CipSoft 7.72 Full MySQL/MariaDB Database Dump\n")
        f.write(f"-- Source: {sqlite_file}\n")
        f.write("-- ==============================================================================\n\n")
        f.write("SET FOREIGN_KEY_CHECKS=0;\n")
        f.write("SET SQL_MODE = 'NO_AUTO_VALUE_ON_ZERO';\n")
        f.write("SET NAMES utf8mb4;\n\n")

        # Include Table DDL from schema_mysql.sql if present
        if os.path.exists(schema_file):
            with open(schema_file, "r", encoding="utf-8") as sf:
                lines = sf.readlines()
            for line in lines:
                if line.strip().startswith("-- Initial Seed Data") or line.strip().startswith("REPLACE INTO") or line.strip().startswith("INSERT INTO"):
                    break
                f.write(line)
            f.write("\n-- ==============================================================================\n")
            f.write("-- Table Data Export\n")
            f.write("-- ==============================================================================\n\n")

        # Tables to export in proper relational order
        tables = [
            "Worlds",
            "Accounts",
            "Characters",
            "CharacterRights",
            "HouseOwners",
            "Buddies",
            "CharacterDeaths",
            "Guilds",
            "GuildRanks",
            "GuildMembers",
            "GuildInvites",
            "Houses",
            "HouseAuctions",
            "HouseTransfers",
            "HouseAssignments",
            "Banishments",
            "IPBanishments",
            "Namelocks",
            "Notations",
            "Statements",
            "ReportedStatements",
            "KillStatistics",
            "OnlineCharacters"
        ]

        total_exported = 0
        for table in tables:
            try:
                cur.execute(f'PRAGMA table_info("{table}")')
                cols = [row[1] for row in cur.fetchall()]
                if not cols:
                    continue

                cur.execute(f'SELECT * FROM "{table}"')
                rows = cur.fetchall()
                if not rows:
                    continue

                print(f"  -> Exporting `{table}` ({len(rows):,} rows)...")
                f.write(f"-- Table `{table}` ({len(rows)} rows)\n")
                f.write(f"TRUNCATE TABLE `{table}`;\n")

                col_names_str = ", ".join([f"`{c}`" for c in cols])
                batch_size = 500
                for i in range(0, len(rows), batch_size):
                    batch = rows[i:i + batch_size]
                    values_list = []
                    for row in batch:
                        vals = [escape_sql_val(v) for v in row]
                        values_list.append(f"({', '.join(vals)})")

                    f.write(f"INSERT INTO `{table}` ({col_names_str}) VALUES\n")
                    f.write(",\n".join(values_list))
                    f.write(";\n\n")

                total_exported += len(rows)
            except Exception as e:
                print(f"[-] Warning on table `{table}`: {e}")

        f.write("SET FOREIGN_KEY_CHECKS=1;\n")
        f.write("-- Dump complete.\n")

    size_mb = os.path.getsize(output_sql) / (1024 * 1024)
    print(f"[+] Successfully exported {total_exported:,} total rows to '{output_sql}' ({size_mb:.2f} MB).")
    return True

if __name__ == "__main__":
    sqlite_db = sys.argv[1] if len(sys.argv) > 1 else "tibia.db"
    out_sql = sys.argv[2] if len(sys.argv) > 2 else "tibia_mysql_full.sql"
    export_db(sqlite_db, out_sql)

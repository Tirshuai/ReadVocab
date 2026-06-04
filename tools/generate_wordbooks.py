#!/usr/bin/env python3
"""
从 ecdict.csv 中筛选出各词书的单词条目，生成独立的 SQLite .db 文件。

词书范围：cet4, cet6, ky, toefl, gre, ielts
筛选依据：tag 列中包含对应的标签
输出位置：lib/<tag>.db
"""

import csv
import sqlite3
import os
import sys

# 目标词书标签
TARGET_TAGS = ["cet4", "cet6", "ky", "toefl", "gre", "ielts"]

# 输入 CSV 路径（相对于脚本所在目录）
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
CSV_PATH = os.path.join(PROJECT_DIR, "lib", "ecdict.csv")
OUTPUT_DIR = os.path.join(PROJECT_DIR, "lib")


def create_tables(conn):
    """创建 vocabulary 表及辅助表（与 C++ 代码保持一致）。"""
    cursor = conn.cursor()

    # 单词表（与 main.cpp / README.md 保持一致）
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS vocabulary (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            word TEXT NOT NULL UNIQUE,
            phonetic TEXT,
            part TEXT,
            meaning TEXT,
            example TEXT
        )
    """)

    # 学习记录表
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS learn_record (
            word_id INTEGER,
            is_learned INTEGER DEFAULT 0,
            study_date TEXT,
            is_correct INTEGER DEFAULT 0,
            starred INTEGER DEFAULT 0,
            PRIMARY KEY(word_id)
        )
    """)

    # 复习记录表
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS review_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            word TEXT NOT NULL,
            is_correct INTEGER NOT NULL,
            review_time DATETIME NOT NULL,
            mode TEXT NOT NULL
        )
    """)

    # 单词关联器 - 分组表
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS relate_group (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            group_name TEXT
        )
    """)

    # 单词关联器 - 单词-分组关联表
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS relate_word (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id INTEGER,
            word_id INTEGER
        )
    """)

    conn.commit()


def has_tag(tag_field, target_tag):
    """检查 tag 列中是否包含指定的标签（空格分隔）。"""
    if not tag_field or not tag_field.strip():
        return False
    tags = set(tag_field.strip().split())
    return target_tag in tags


def main():
    if not os.path.exists(CSV_PATH):
        print(f"错误：找不到 ecdict.csv 文件：{CSV_PATH}")
        sys.exit(1)

    print(f"正在读取：{CSV_PATH}")
    print(f"输出目录：{OUTPUT_DIR}")
    print(f"目标词书：{', '.join(TARGET_TAGS)}")
    print("-" * 60)

    # 为每个词书创建独立的数据库连接和写入器
    connections = {}
    cursors = {}
    counts = {tag: 0 for tag in TARGET_TAGS}

    for tag in TARGET_TAGS:
        db_path = os.path.join(OUTPUT_DIR, f"{tag}.db")
        # 删除旧的 .db 文件，重新生成
        if os.path.exists(db_path):
            os.remove(db_path)
            print(f"  已删除旧文件：{db_path}")

        conn = sqlite3.connect(db_path)
        create_tables(conn)
        connections[tag] = conn
        cursors[tag] = conn.cursor()

    # 预编译 INSERT 语句
    insert_sql = "INSERT OR IGNORE INTO vocabulary (word, phonetic, part, meaning) VALUES (?, ?, ?, ?)"

    # 读取 CSV 并分类
    total = 0
    matched = 0

    with open(CSV_PATH, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)

        for row in reader:
            total += 1

            if total % 100000 == 0:
                print(f"  已处理 {total} 行，已匹配 {matched} 条...")

            tag_field = row.get("tag", "")
            if not tag_field or not tag_field.strip():
                continue

            word = (row.get("word") or "").strip()
            if not word:
                continue

            phonetic = (row.get("phonetic") or "").strip()
            pos = (row.get("pos") or "").strip()
            translation = (row.get("translation") or "").strip()

            # 一行的 tag 可能匹配多个词书（如 cet4 + cet6）
            for tag in TARGET_TAGS:
                if has_tag(tag_field, tag):
                    try:
                        cursors[tag].execute(insert_sql, (word, phonetic, pos, translation))
                        counts[tag] += 1
                        matched += 1
                    except Exception as e:
                        print(f"  警告：插入失败 [{tag}] {word}: {e}")

    # 提交并关闭所有连接
    for tag in TARGET_TAGS:
        connections[tag].commit()
        connections[tag].close()

    print("-" * 60)
    print(f"处理完成！共处理 {total} 行，匹配 {matched} 条记录。")
    print()
    print("各词书单词数：")
    for tag in TARGET_TAGS:
        db_path = os.path.join(OUTPUT_DIR, f"{tag}.db")
        size_kb = os.path.getsize(db_path) / 1024 if os.path.exists(db_path) else 0
        print(f"  {tag:8s} → {counts[tag]:6d} 词 | {db_path} ({size_kb:.1f} KB)")
    print()

    # 验证
    for tag in TARGET_TAGS:
        db_path = os.path.join(OUTPUT_DIR, f"{tag}.db")
        conn = sqlite3.connect(db_path)
        cur = conn.cursor()
        cur.execute("SELECT COUNT(*) FROM vocabulary")
        actual = cur.fetchone()[0]
        cur.execute("SELECT word, phonetic, part, meaning FROM vocabulary LIMIT 3")
        samples = cur.fetchall()
        conn.close()
        print(f"  [{tag}] 验证：{actual} 条记录")
        for s in samples:
            try:
                w = s[0] if s[0] else '-'
                ph = (s[1][:30] if s[1] else '-')
                po = (s[2][:20] if s[2] else '-')
                me = (s[3][:40] if s[3] else '-')
                print(f"    {w} | {ph} | {po} | {me}")
            except UnicodeEncodeError:
                print(f"    (含特殊字符，跳过显示)")
        print()


if __name__ == "__main__":
    main()

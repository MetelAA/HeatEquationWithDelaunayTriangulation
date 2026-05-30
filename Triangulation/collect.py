#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Скрипт для сбора кода из всех текстовых файлов в указанной папке (по умолчанию — текущая).
Рекурсивно обходит все подпапки, пропускает бинарные файлы и файлы с ошибками чтения.
"""

import os
import sys
import argparse
from pathlib import Path

def should_skip_file(file_path: Path, skip_extensions: set, skip_dirs: set, output_file: Path) -> bool:
    """Определяет, нужно ли пропустить файл."""
    if file_path.resolve() == output_file.resolve():
        return True
    if file_path.suffix.lower() in skip_extensions:
        return True
    for part in file_path.parts:
        if part in skip_dirs:
            return True
    return False

def collect_files(root_dir: Path, output_file: Path, skip_extensions: set, skip_dirs: set):
    """Обходит root_dir и записывает содержимое всех текстовых файлов в output_file."""
    with open(output_file, 'w', encoding='utf-8') as out_f:
        out_f.write(f"# Сбор кода из папки: {root_dir.resolve()}\n")
        out_f.write(f"# Дата и время: {__import__('datetime').datetime.now()}\n\n")

        for current_dir, dirs, files in os.walk(root_dir):
            dirs[:] = [d for d in dirs if d not in skip_dirs]

            for file_name in files:
                file_path = Path(current_dir) / file_name
                if should_skip_file(file_path, skip_extensions, skip_dirs, output_file):
                    continue

                try:
                    with open(file_path, 'r', encoding='utf-8') as in_f:
                        content = in_f.read()
                except (UnicodeDecodeError, PermissionError, OSError):
                    print(f"Пропущен (не текст или ошибка доступа): {file_path}")
                    continue

                rel_path = file_path.relative_to(root_dir)
                out_f.write(f"\n{'='*80}\n")
                out_f.write(f"# Файл: {rel_path}\n")
                out_f.write(f"{'='*80}\n\n")
                out_f.write(content)
                out_f.write("\n\n")
                print(f"Обработан: {rel_path}")

def main():
    parser = argparse.ArgumentParser(
        description="Собрать весь код из файлов в папке в один текстовый файл."
    )
    parser.add_argument(
        "source_dir",
        nargs="?",                      # делает аргумент необязательным
        default=".",                    # текущая директория по умолчанию
        help="Путь к папке (по умолчанию: текущая директория)"
    )
    parser.add_argument(
        "-o", "--output",
        default="collected_code.txt",
        help="Имя выходного файла (по умолчанию: collected_code.txt)"
    )
    parser.add_argument(
        "--skip-ext",
        nargs="+",
        default=[".jpg", ".jpeg", ".png", ".gif", ".bmp", ".ico",
                 ".mp3", ".mp4", ".avi", ".mov",
                 ".zip", ".tar", ".gz", ".7z",
                 ".exe", ".dll", ".so", ".dylib",
                 ".pyc", ".pyo", ".pyd",
                 ".pdf", ".doc", ".docx", ".xls", ".xlsx"],
        help="Расширения файлов для пропуска (по умолчанию: бинарные и медиа)"
    )
    parser.add_argument(
        "--skip-dirs",
        nargs="+",
        default=[".git", "__pycache__", "node_modules", ".venv", "venv", "env",
                 ".idea", ".vscode", "build", "dist"],
        help="Имена папок для пропуска (по умолчанию: системные и виртуальные окружения)"
    )

    args = parser.parse_args()

    source_dir = Path(args.source_dir)
    if not source_dir.is_dir():
        print(f"Ошибка: '{source_dir}' не является существующей папкой.", file=sys.stderr)
        sys.exit(1)

    output_file = Path(args.output)
    skip_extensions = set(ext.lower() for ext in args.skip_ext)
    skip_dirs = set(args.skip_dirs)

    collect_files(source_dir, output_file, skip_extensions, skip_dirs)
    print(f"\nГотово. Результат сохранён в {output_file.resolve()}")

if __name__ == "__main__":
    main()

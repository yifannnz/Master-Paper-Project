#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Count words in LaTeX thesis files"""

import re
from pathlib import Path

def clean_text(text):
    """Remove LaTeX markup"""
    text = re.sub(r'%.*', '', text)
    text = re.sub(r'\\begin\{[^}]*\}.*?\\end\{[^}]*\}', '', text, flags=re.DOTALL)
    text = re.sub(r'\$\$.*?\$\$', '', text, flags=re.DOTALL)
    text = re.sub(r'\$.*?\$', '', text, flags=re.DOTALL)
    text = re.sub(r'\\[a-zA-Z_]+\{[^}]*\}', '', text)
    text = re.sub(r'\\[a-zA-Z_]+', '', text)
    text = re.sub(r'[{}]', ' ', text)
    return text

def count_words(text):
    text = clean_text(text)
    chinese = len(re.findall(r'[\u4e00-\u9fff]', text))
    english = len(re.findall(r'\b[a-zA-Z]+\b', text))
    return chinese, english

root = Path('.')
tex_files = sorted([f for f in root.rglob('*.tex') 
                   if 'out-build' not in str(f) and '论文' not in str(f)])

print(f"\n{'文件':<55} {'中文':<8} {'英文':<8}")
print("=" * 75)

total_ch, total_en = 0, 0
file_data = []

for tex_file in tex_files:
    try:
        content = tex_file.read_text(encoding='utf-8')
        ch, en = count_words(content)
        if ch > 0 or en > 0:
            file_data.append((tex_file, ch, en))
            total_ch += ch
            total_en += en
    except:
        pass

for tex_file, ch, en in file_data:
    fname = str(tex_file.relative_to(root))
    print(f"{fname:<55} {ch:<8} {en:<8}")

print("=" * 75)
print(f"{'合计':<55} {total_ch:<8} {total_en:<8}")
print(f"\n学位论文字数统计（按规范：中文字数 + 英文词数2）: {total_ch + total_en//2:.0f}")
print(f"  中文字数: {total_ch}")
print(f"  英文词数: {total_en}\n")

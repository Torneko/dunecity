#!/usr/bin/env python3
"""Fail CI if a standard faction label is missing from the French catalog."""

from pathlib import Path
import re
import sys

catalog_path = Path(__file__).resolve().parents[1] / "data/locale/French.fr.po"
catalog_text = catalog_path.read_text(encoding="utf-8")

required_labels = {
    "Harkonnen",
    "Atreides",
    "Ordos",
    "Fremen",
    "Sardaukar",
    "Mercenary",
    "Neutral",
    "Rebels",
}
translations = {}

for block in re.split(r"\n[ \t]*\n", catalog_text):
    msgid_match = re.search(r'^msgid "([^"\\]*)"$', block, re.MULTILINE)
    if not msgid_match or msgid_match.group(1) not in required_labels:
        continue

    msgstr_match = re.search(r'^msgstr "([^"\\]*)"$', block, re.MULTILINE)
    translations.setdefault(msgid_match.group(1), []).append(
        msgstr_match.group(1) if msgstr_match else ""
    )

errors = []
for label in sorted(required_labels):
    values = translations.get(label, [])
    if not values:
        errors.append(f'missing msgid "{label}"')
    elif len(values) != 1:
        errors.append(f'msgid "{label}" appears {len(values)} times')
    elif not values[0].strip():
        errors.append(f'msgid "{label}" has an empty French translation')

if errors:
    print("French faction catalog check failed:", file=sys.stderr)
    for error in errors:
        print(f"  - {error}", file=sys.stderr)
    raise SystemExit(1)

print("French faction labels have explicit translations.")

"""Small lexical helpers, not a C preprocessor or a reachability proof."""
import re


def mask_comments_and_literals(text):
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\''
    return re.sub(pattern, lambda m: re.sub(r"[^\n]", " ", m.group()), text)


def function_blocks(text):
    masked = mask_comments_and_literals(text)
    pattern = re.compile(r"(?m)^(?:static\s+)?(?:inline\s+)?(?:void|BOOL|bool8|bool16|bool32|u8|u16|u32|s8|s16|s32|int|enum\s+\w+)\s+(\w+)\s*\([^;{}]*\)\s*\{")
    end_previous = 0
    for match in pattern.finditer(masked):
        if match.start() < end_previous:
            continue
        opening = masked.index("{", match.start(), match.end())
        depth, cursor = 1, opening + 1
        while depth and cursor < len(masked):
            if masked[cursor] == "{": depth += 1
            elif masked[cursor] == "}": depth -= 1
            cursor += 1
        if depth:
            raise ValueError(f"Unclosed function: {match.group(1)}")
        end_previous = cursor
        yield {"name": match.group(1), "line": text.count("\n", 0, match.start()) + 1,
               "start": match.start(), "end": cursor, "body": text[match.start():cursor]}


def extract_function(text, name):
    matches = [block for block in function_blocks(text) if block["name"] == name]
    if len(matches) != 1:
        raise ValueError(f"Expected exactly one definition of {name}, found {len(matches)}")
    return matches[0]["body"]


def catalog_symbols(text, prefix, kind):
    clean = mask_comments_and_literals(text)
    if kind == "define":
        pattern = rf"(?m)^\s*#define\s+({prefix}\w*)\b"
    else:
        pattern = rf"(?m)^\s*({prefix}\w*)\s*(?:=[^,\n]+)?\s*,?\s*$"
    return set(re.findall(pattern, clean))

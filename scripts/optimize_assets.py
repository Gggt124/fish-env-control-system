import os
import re
import gzip
import sys

def clean_js(code):
    state = "normal"
    out = []
    i = 0
    n = len(code)
    
    while i < n:
        c = code[i]
        
        if state in ("string_single", "string_double"):
            out.append(c)
            if c == '\\':
                if i + 1 < n:
                    out.append(code[i+1])
                    i += 2
                    continue
            elif (state == "string_single" and c == "'") or \
                 (state == "string_double" and c == '"'):
                state = "normal"
            i += 1
            continue
            
        if state == "regex":
            out.append(c)
            if c == '\\':
                if i + 1 < n:
                    out.append(code[i+1])
                    i += 2
                    continue
            elif c == '/':
                state = "normal"
            i += 1
            continue
            
        if state == "comment_line":
            if c == '\n' or c == '\r':
                state = "normal"
                out.append(c)
            i += 1
            continue
            
        if state == "comment_block":
            if c == '*' and i + 1 < n and code[i+1] == '/':
                state = "normal"
                i += 2
            else:
                i += 1
            continue
            
        # normal state
        next_c = code[i+1] if i + 1 < n else ""
        if c == '/' and next_c == '/':
            state = "comment_line"
            i += 2
        elif c == '/' and next_c == '*':
            state = "comment_block"
            i += 2
        elif c == "'":
            state = "string_single"
            out.append(c)
            i += 1
        elif c == '"':
            state = "string_double"
            out.append(c)
            i += 1
        elif c == '/':
            # regex or division heuristic
            is_regex = False
            idx = len(out) - 1
            while idx >= 0 and out[idx].isspace():
                idx -= 1
            if idx >= 0:
                last_char = out[idx]
                if last_char in "=+(,;:[!&|?*~^%<>-":
                    is_regex = True
                else:
                    word_chars = []
                    while idx >= 0 and (out[idx].isalnum() or out[idx] == '_'):
                        word_chars.append(out[idx])
                        idx -= 1
                    word = "".join(reversed(word_chars))
                    if word in ("return", "match", "test", "throw", "typeof", "delete", "void", "in", "instanceof"):
                        is_regex = True
            else:
                is_regex = True
            
            if is_regex:
                state = "regex"
                out.append(c)
                i += 1
            else:
                out.append(c)
                i += 1
        else:
            out.append(c)
            i += 1
            
    cleaned_code = "".join(out)
    
    # Remove empty lines and trailing spaces
    result_lines = []
    for line in cleaned_code.splitlines():
        line_stripped = line.strip()
        if line_stripped:
            result_lines.append(line.rstrip())
            
    return "\n".join(result_lines)

def clean_css(css):
    # Remove comments
    css = re.sub(r'/\*.*?\*/', '', css, flags=re.DOTALL)
    # Compress whitespaces
    css = re.sub(r'\s+', ' ', css)
    # Remove space around delimiters
    css = re.sub(r'\s*([\{\};:,])\s*', r'\1', css)
    return css.strip()

def clean_html(html):
    # Remove comments
    html = re.sub(r'<!--.*?-->', '', html, flags=re.DOTALL)
    # Collapse multiple spaces and newlines
    lines = []
    for line in html.splitlines():
        line_stripped = line.strip()
        if line_stripped:
            lines.append(line_stripped)
    return "\n".join(lines)

def compress_file(src_path, dest_raw_path, dest_gz_path, file_type):
    print(f"Optimizing {src_path}...")
    with open(src_path, "r", encoding="utf-8") as f:
        content = f.read()
    
    if file_type == "js":
        optimized = clean_js(content)
    elif file_type == "css":
        optimized = clean_css(content)
    elif file_type == "html":
        optimized = clean_html(content)
    else:
        optimized = content

    # Write optimized raw file
    with open(dest_raw_path, "w", encoding="utf-8") as f:
        f.write(optimized)
    
    # Write gzipped file (highest compression level 9)
    optimized_bytes = optimized.encode("utf-8")
    with gzip.open(dest_gz_path, "wb", compresslevel=9) as f:
        f.write(optimized_bytes)
        
    orig_size = os.path.getsize(src_path)
    opt_size = len(optimized_bytes)
    gz_size = os.path.getsize(dest_gz_path)
    print(f"  Original:   {orig_size:6} bytes")
    print(f"  Minified:   {opt_size:6} bytes ({opt_size/orig_size*100:.1f}%)")
    print(f"  Gzipped:    {gz_size:6} bytes ({gz_size/orig_size*100:.1f}%)")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    
    src_dir = os.path.join(project_root, "main", "static")
    dest_dir = os.path.join(project_root, "main", "static_compressed")
    
    os.makedirs(dest_dir, exist_ok=True)
    
    files_to_process = [
        ("index.html", "html"),
        ("style.css", "css"),
        ("app.js", "js")
    ]
    
    for filename, file_type in files_to_process:
        src = os.path.join(src_dir, filename)
        dest_raw = os.path.join(dest_dir, filename)
        dest_gz = os.path.join(dest_dir, f"{filename}.gz")
        compress_file(src, dest_raw, dest_gz, file_type)

if __name__ == "__main__":
    main()

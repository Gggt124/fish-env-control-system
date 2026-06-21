import urllib.request
import re
import os

FONTS = {
    "ibm_plex_sans_thai": "https://fonts.googleapis.com/css2?family=IBM+Plex+Sans+Thai:wght@400;600;700&display=swap",
    "jetbrains_mono": "https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;700&display=swap"
}

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "main", "static")
os.makedirs(OUT_DIR, exist_ok=True)

headers = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"
}

def download_fonts_and_generate_css():
    css_output = []
    
    for name, url in FONTS.items():
        req = urllib.request.Request(url, headers=headers)
        print(f"Fetching CSS for {name}...")
        with urllib.request.urlopen(req) as response:
            css = response.read().decode('utf-8')
        
        # We need to find the blocks that correspond to 'thai' or 'latin' subsets
        # We split the CSS by "/* " to separate by subset comments
        sections = css.split('/* ')
        
        for section in sections:
            if not section.strip(): continue
            
            # The section should start with "subset_name */"
            parts = section.split('*/', 1)
            if len(parts) != 2: continue
            
            subset_name = parts[0].strip()
            # We only care about thai, latin, and latin-ext
            if subset_name not in ['thai', 'latin']:
                continue
                
            block = parts[1]
            
            # Extract weight
            weight_match = re.search(r'font-weight:\s*(\d+)', block)
            if not weight_match: continue
            weight = weight_match.group(1)
            
            # Extract URL
            url_match = re.search(r"src:\s*url\((https://[^)]+\.woff2)\)", block)
            if not url_match: continue
            woff2_url = url_match.group(1)
            
            # Generate local filename
            filename = f"{name}_{weight}_{subset_name}.woff2"
            filepath = os.path.join(OUT_DIR, filename)
            
            # Download file
            print(f"Downloading {filename}...")
            with urllib.request.urlopen(woff2_url) as woff_resp:
                with open(filepath, 'wb') as f:
                    f.write(woff_resp.read())
            
            # Replace URL in block with local path
            local_block = block.replace(woff2_url, f"/{filename}")
            css_output.append(f"/* {subset_name} */\n{local_block}")

    # Write CSS snippet to console to copy into style.css
    print("\n--- ADD THIS TO style.css ---\n")
    print("".join(css_output))

if __name__ == "__main__":
    download_fonts_and_generate_css()

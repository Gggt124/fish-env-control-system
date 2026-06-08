import re

def extract_content(filename, tag_pattern):
    with open(filename, 'r', encoding='utf-8') as f:
        content = f.read()
    match = re.search(tag_pattern, content)
    if match:
        return match.group(1).strip()
    return ""

login = extract_content("main/static/login.html", r'(?s)(<main class="login-card">.*?</main>)')
dashboard = extract_content("main/static/dashboard.html", r'(?s)<main class="main-content">(.*?)</main>')
status = extract_content("main/static/status.html", r'(?s)<main class="main-content">(.*?)</main>')
wifi = extract_content("main/static/wifi.html", r'(?s)<main class="main-content">(.*?)</main>')
hardware = extract_content("main/static/hardware.html", r'(?s)<main class="main-content">(.*?)</main>')

hw_full = extract_content("main/static/hardware.html", r'(?s)<main class="main-content">(.*?)</main>\s*(<div id="toast" class="toast"></div>)?')
toast_match = re.search(r'<div id="toast" class="toast"></div>', open("main/static/hardware.html", 'r', encoding='utf-8').read())
toast = '<div id="toast" class="toast"></div>' if toast_match else ''

with open("main/static/index.html", 'r', encoding='utf-8') as f:
    index_html = f.read()

def replacer(match, replacement):
    return match.group(1) + '\n' + replacement.replace('\\', '\\\\') + '\n' + match.group(2)

index_html = re.sub(r'(<section id="view-login" class="view hidden">).*?(</section>)', 
                    lambda m: replacer(m, '<div class="login-container">\n' + login + '\n</div>'), 
                    index_html, flags=re.DOTALL)

index_html = re.sub(r'(<section id="view-dashboard" class="view hidden">).*?(</section>)', 
                    lambda m: replacer(m, dashboard), 
                    index_html, flags=re.DOTALL)

index_html = re.sub(r'(<section id="view-status" class="view hidden">).*?(</section>)', 
                    lambda m: replacer(m, status), 
                    index_html, flags=re.DOTALL)

index_html = re.sub(r'(<section id="view-wifi" class="view hidden">).*?(</section>)', 
                    lambda m: replacer(m, wifi), 
                    index_html, flags=re.DOTALL)

index_html = re.sub(r'(<section id="view-hardware" class="view hidden">).*?(</section>)', 
                    lambda m: replacer(m, hardware), 
                    index_html, flags=re.DOTALL)

if toast and toast not in index_html:
    index_html = index_html.replace('</main>', '</main>\n' + toast)

with open("main/static/index.html", 'w', encoding='utf-8') as f:
    f.write(index_html)

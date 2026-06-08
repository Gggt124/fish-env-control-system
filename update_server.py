with open("main/web_server.c", "r", encoding="utf-8") as f:
    content = f.read()

import re

decl_pattern = r'extern const uint8_t _binary_login_html_start\[\] asm\("_binary_login_html_start"\);\n.*_binary_status_html_end"\);\n'
new_decl = """extern const uint8_t _binary_index_html_start[] asm("_binary_index_html_start");
extern const uint8_t _binary_index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t _binary_style_css_start[] asm("_binary_style_css_start");
extern const uint8_t _binary_style_css_end[]   asm("_binary_style_css_end");
extern const uint8_t _binary_app_js_start[] asm("_binary_app_js_start");
extern const uint8_t _binary_app_js_end[]   asm("_binary_app_js_end");
"""

content = re.sub(decl_pattern, new_decl, content, flags=re.DOTALL)

content = content.replace("_binary_login_html_start, _binary_login_html_end", "_binary_index_html_start, _binary_index_html_end")
content = content.replace("_binary_dashboard_html_start, _binary_dashboard_html_end", "_binary_index_html_start, _binary_index_html_end")
content = content.replace("_binary_wifi_html_start, _binary_wifi_html_end", "_binary_index_html_start, _binary_index_html_end")
content = content.replace("_binary_hardware_html_start, _binary_hardware_html_end", "_binary_index_html_start, _binary_index_html_end")
content = content.replace("_binary_status_html_start, _binary_status_html_end", "_binary_index_html_start, _binary_index_html_end")

with open("main/web_server.c", "w", encoding="utf-8") as f:
    f.write(content)

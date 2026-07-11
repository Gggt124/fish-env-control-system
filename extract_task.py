import sys
import os
import re

if len(sys.argv) < 3:
    print("Usage: extract_task.py <plan_file> <task_number>")
    sys.exit(1)

plan_file = sys.argv[1]
task_number = sys.argv[2]

out_dir = os.path.join(os.path.dirname(os.path.dirname(plan_file)), "sdd")
os.makedirs(out_dir, exist_ok=True)
out_file = os.path.join(out_dir, f"task-{task_number}-brief.md")

with open(plan_file, "r", encoding="utf-8") as f:
    lines = f.readlines()

in_task = False
in_fence = False
out_lines = []
task_regex = re.compile(r"^#+\s+Task\s+" + task_number + r"(?:[^0-9]|$)")

for line in lines:
    if line.startswith("```"):
        in_fence = not in_fence
    
    if not in_fence and re.match(r"^#+\s+Task\s+\d+", line):
        if task_regex.match(line):
            in_task = True
        else:
            in_task = False
    
    if in_task:
        out_lines.append(line)

if not out_lines:
    print(f"Task {task_number} not found.")
    sys.exit(1)

with open(out_file, "w", encoding="utf-8") as f:
    f.writelines(out_lines)

print(f"wrote {out_file}: {len(out_lines)} lines")

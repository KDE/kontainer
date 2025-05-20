import subprocess
import re
from typing import List, Dict, Optional

DISTROS = [
    "alma", "alpine", "amazon", "bazzite", "arch", "centos", "clearlinux", "crystal",
    "debian", "deepin", "fedora", "gentoo", "kali", "mageia", "mint", "neon", "opensuse",
    "oracle", "redhat", "rhel", "rocky", "slackware", "steamos", "ubuntu", "ublue",
    "vanilla", "void"
]

def run_command(command: List[str]) -> str:
    try:
        result = subprocess.run(
            command, 
            check=True, 
            text=True, 
            capture_output=True,
            timeout=60
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        return e.stdout.strip() if e.stdout else str(e)
    except subprocess.TimeoutExpired:
        return "Error: Command timed out"

def get_all_distroboxes() -> List[Dict]:
    output = run_command(["distrobox", "list", "--no-color"])
    lines = output.splitlines()
    if not lines:
        return []

    headings = [col.strip() for col in lines[0].split('|')]
    idx = {h: headings.index(h) for h in ("NAME", "IMAGE", "ID", "STATUS")}

    boxes = []
    for line in lines[1:]:
        if not line.strip():
            continue
        parts = [col.strip() for col in line.split('|')]
        status = parts[idx["STATUS"]]
        boxes.append({
            "name": parts[idx["NAME"]],
            "distro": try_parse_distro_from_url(parts[idx["IMAGE"]]),
            "image_url": parts[idx["IMAGE"]],
            "container_id": parts[idx["ID"]],
            "status": status,
            "is_running": not any(s in status for s in ("Exited", "Created"))
        })
    return boxes

def try_parse_distro_from_url(url: str) -> str:
    last = url.split('/')[-1].lower()
    for d in DISTROS:
        if d in last:
            return d
    for d in DISTROS:
        if d in url.lower():
            return d
    return "unknown"

def get_apps_in_box(box_name: str) -> List[str]:
    from pathlib import Path
    apps = []
    host_dir = Path.home() / ".local/share/applications"
    prefix = f"{box_name}-"
    for f in host_dir.glob(f"{prefix}*.desktop"):
        apps.append(f.stem[len(prefix):])
    return apps

def get_apps_from_container_fs(box_name: str) -> List[str]:
    output = run_command([
        "distrobox", "enter", box_name, "--",
        "find", "/usr/share/applications", "-name", "*.desktop"
    ])
    return [line.split('/')[-1].replace('.desktop', '') 
            for line in output.splitlines() if line]

def get_available_images() -> List[Dict]:
    output = run_command(["distrobox", "create", "-C"])
    return [
        {'name': line.strip().split()[-1].split('/')[-1],
         'url': line.strip().split()[-1]}
          for line in output.splitlines() 
    ]

def enter_box(box_name: str) -> None:
    # Try to detect the default terminal
    terminals = [
        "gnome-terminal", "konsole", "xfce4-terminal",
        "lxterminal", "mate-terminal", "tilix", "alacritty", "xterm"
    ]

    for term in terminals:
        if subprocess.run(["which", term], capture_output=True).returncode == 0:
            subprocess.Popen([term, "-e", "distrobox", "enter", box_name])
            return

    # Fallback to xterm
    subprocess.Popen(["xterm", "-e", "distrobox", "enter", box_name])

def run_in_box(command: str, box_name: str) -> None:
    subprocess.Popen(["distrobox", "enter", box_name, "--", command])

def export_app(app_name: str, box_name: str) -> str:
    return run_command([
        "distrobox", "enter", box_name, "--",
        "distrobox-export", "--app", app_name
    ])

def remove_exported_app(app_name: str, box_name: str) -> str:
    return run_command([
        "distrobox", "enter", box_name, "--",
        "distrobox-export", "--app", app_name, "--delete"
    ])

def upgrade_box(box_name: str) -> None:
        # Try to detect the default terminal
    terminals = [
        "gnome-terminal", "konsole", "xfce4-terminal",
        "lxterminal", "mate-terminal", "tilix", "alacritty", "xterm"
    ]
    
    for term in terminals:
        if subprocess.run(["which", term], capture_output=True).returncode == 0:
            subprocess.Popen([term, "-e", "distrobox", "upgrade", box_name])
            return
    
    # Fallback to xterm
    subprocess.Popen(["xterm", "-e", "distrobox", "upgrade", box_name])

def delete_box(box_name: str) -> str:
    return run_command(["distrobox", "rm", box_name, "--force"])

def create_box(box_name: str, image: str, 
               home_path: str = "", 
               use_init: bool = False, 
               volumes: Optional[List[str]] = None) -> str:
    args = ["distrobox", "create", "-n", box_name, "-i", image, "-Y"]
    if use_init:
        args.extend(["--init", "--additional-packages", "systemd"])
    if home_path:
        args.extend(["--home", home_path])
    if volumes:
        for v in volumes:
            args.extend(["--volume", v])
    
    # Create container
    create_result = run_command(args)
    
    
       
    return f"{create_result}\n{init_result}"

def assemble_box(ini_file: str) -> str:
    return run_command(["distrobox", "assemble", "create", "--file", ini_file])

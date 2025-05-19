import subprocess
import re

# distrobox_backend.py

DISTROS = [
    "alma", "alpine", "amazon", "bazzite", "arch", "centos", "clearlinux", "crystal",
    "debian", "deepin", "fedora", "gentoo", "kali", "mageia", "mint", "neon", "opensuse",
    "oracle", "redhat", "rhel", "rocky", "slackware", "steamos", "ubuntu", "ublue",
    "vanilla", "void"
]

def run_command(command):
    try:
        result = subprocess.run(command, check=True, text=True, capture_output=True)
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        return e.stdout.strip() if e.stdout else str(e)


def get_all_distroboxes():
    output = run_command(["distrobox", "list", "--no-color"])
    lines = output.splitlines()
    if not lines:
        return []

    headings = [col.strip() for col in lines[0].split('|')]
    idx = {h: headings.index(h) for h in ("NAME","IMAGE","ID","STATUS")}

    boxes = []
    for line in lines[1:]:
        if not line.strip(): continue
        parts = [col.strip() for col in line.split('|')]
        status = parts[idx["STATUS"]]
        boxes.append({
            "name": parts[idx["NAME"]],
            "distro": try_parse_distro_from_url(parts[idx["IMAGE"]]),
            "image_url": parts[idx["IMAGE"]],
            "container_id": parts[idx["ID"]],
            "status": status,
            "is_running": not any(s in status for s in ("Exited","Created"))
        })
    return boxes


def try_parse_distro_from_url(url):
    last = url.split('/')[-1].lower()
    for d in DISTROS:
        if d in last: return d
    for d in DISTROS:
        if d in url.lower(): return d
    return "zunknown"


def get_apps_in_box(box_name):
    # parse exported .desktop files on host
    from pathlib import Path
    apps = []
    host_dir = Path.home() / ".local/share/applications"
    prefix = f"{box_name}-"
    for f in host_dir.glob(f"{prefix}*.desktop"):
        apps.append(f.stem[len(prefix):])
    return apps


def get_apps_from_container_fs(box_name):
    output = run_command([
        "distrobox","enter",box_name,"--",
        "find","/usr/share/applications","-name","*.desktop"
    ])
    apps = []
    for line in output.splitlines():
        name = line.split('/')[-1].replace('.desktop','')
        apps.append(name)
    return apps


def get_available_images():
    output = run_command(["distrobox","create","-C"])
    images = []
    for line in output.splitlines():
        if 'docker.io' in line or 'ghcr.io' in line:
            url = line.strip().split()[-1]
            images.append({
                'name': url.split('/')[-1],
                'url': url
            })
    return images


def enter_box(box_name):
    subprocess.Popen(["distrobox","enter",box_name])

def run_in_box(command, box_name):
    subprocess.Popen(["distrobox","enter",box_name,"--",command])

def export_app(app_name, box_name):
    return run_command([
        "distrobox","enter",box_name,"--",
        "distrobox-export","--app",app_name
    ])

def remove_exported_app(app_name, box_name):
    return run_command([
        "distrobox","enter",box_name,"--",
        "distrobox-export","--app",app_name,"--delete"
    ])

def upgrade_box(box_name):
    subprocess.Popen(["bash","-c",f"distrobox upgrade {box_name}; distrobox enter {box_name}"])

def delete_box(box_name):
    return run_command(["distrobox","rm",box_name,"--force"])

def create_box(box_name, image, home_path="", use_init=False, volumes=None):
    args = ["distrobox","create","-n",box_name,"-i",image,"-Y"]
    if use_init: args += ["--init","--additional-packages","systemd"]
    if home_path: args += ["--home",home_path]
    if volumes:
        for v in volumes: args += ["--volume",v]
    return run_command(args)

def assemble_box(ini_file):
    return run_command(["distrobox","assemble","create","--file",ini_file])


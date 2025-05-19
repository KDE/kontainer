# distrobox_backend_mock.py
import subprocess
import re

DISTROS = [
    "alma", "alpine", "amazon", "bazzite", "arch", "centos", "clearlinux", "crystal",
    "debian", "deepin", "fedora", "gentoo", "kali", "mageia", "mint", "neon", "opensuse",
    "oracle", "redhat", "rhel", "rocky", "slackware", "steamos", "ubuntu", "ublue",
    "vanilla", "void"
]

# Mock data storage
containers = {
    "devbox": {
        "distro": "alpine",
        "image_url": "docker.io/library/alpine:latest",
        "status": "running",
        "is_running": True,
        "exported_apps": ["htop", "vim"],
        "available_apps": ["htop", "vim", "python3", "bash", "nano"]
    },
    "testbox": {
        "distro": "debian",
        "image_url": "docker.io/library/debian:stable",
        "status": "exited",
        "is_running": False,
        "exported_apps": [],
        "available_apps": ["bash", "apt", "dpkg"]
    }
}

def run_command(command):
    return f"Simulated command: {' '.join(command)}"

def get_all_distroboxes():
    return [{
        "name": name,
        "distro": data["distro"],
        "image_url": data["image_url"],
        "container_id": f"mock_{name}",
        "status": data["status"],
        "is_running": data["is_running"]
    } for name, data in containers.items()]

def try_parse_distro_from_url(url):
    last = url.split('/')[-1].lower()
    for d in DISTROS:
        if d in last: return d
    for d in DISTROS:
        if d in url.lower(): return d
    return "unknown"

def get_apps_in_box(box_name):
    """Returns already exported apps"""
    return containers.get(box_name, {}).get("exported_apps", [])

def get_apps_from_container_fs(box_name):
    """Returns apps available in container but not yet exported"""
    container = containers.get(box_name, {})
    exported = set(container.get("exported_apps", []))
    available = set(container.get("available_apps", []))
    return list(available - exported)

def get_available_images():
    return [
        {'name': 'alpine:latest', 'url': 'docker.io/library/alpine:latest'},
        {'name': 'debian:stable', 'url': 'docker.io/library/debian:stable'},
        {'name': 'ubuntu:latest', 'url': 'docker.io/library/ubuntu:latest'},
        {'name': 'archlinux:latest', 'url': 'docker.io/library/archlinux:latest'},
        {'name': 'fedora:latest', 'url': 'docker.io/library/fedora:latest'},
    ]

def enter_box(box_name):
    return f"Simulated entering box: {box_name}"

def run_in_box(command, box_name):
    return f"Simulated running '{command}' in {box_name}"

def export_app(app_name, box_name):
    if box_name not in containers:
        return f"Error: Container {box_name} not found"
    
    if app_name in containers[box_name]["exported_apps"]:
        return f"Error: {app_name} already exported"
    
    if app_name not in containers[box_name]["available_apps"]:
        return f"Error: {app_name} not found in container"
    
    containers[box_name]["exported_apps"].append(app_name)
    return f"Successfully exported {app_name} from {box_name}"

def remove_exported_app(app_name, box_name):
    if box_name not in containers:
        return f"Error: Container {box_name} not found"
    
    if app_name not in containers[box_name]["exported_apps"]:
        return f"Error: {app_name} not found in exported apps"
    
    containers[box_name]["exported_apps"].remove(app_name)
    return f"Successfully removed {app_name} from {box_name}"

def upgrade_box(box_name):
    return f"Simulated upgrade for {box_name}"

def delete_box(box_name):
    if box_name in containers:
        del containers[box_name]
    return f"Simulated deletion of {box_name}"

def create_box(box_name, image, home_path="", use_init=False, volumes=None):
    if not box_name or not image:
        return "Error: Box name and image are required"
    
    containers[box_name] = {
        "distro": try_parse_distro_from_url(image),
        "image_url": image,
        "status": "created",
        "is_running": False,
        "exported_apps": [],
        "available_apps": ["bash", "ls", "cat"]  # Default apps for new containers
    }
    return f"""
    Created new container:
    - Name: {box_name}
    - Image: {image}
    - Home: {home_path or 'default'}
    - Init: {'enabled' if use_init else 'disabled'}
    - Volumes: {volumes or 'none'}
    """

def assemble_box(ini_file):
    return f"Simulated assembling containers from {ini_file}"


from typing import List, Dict, Optional
import random

DISTROS = [
    "alma", "alpine", "amazon", "bazzite", "arch", "centos", "clearlinux", "crystal",
    "debian", "deepin", "fedora", "gentoo", "kali", "mageia", "mint", "neon", "opensuse",
    "oracle", "redhat", "rhel", "rocky", "slackware", "steamos", "ubuntu", "ublue",
    "vanilla", "void"
]

class MockBackend:
    def __init__(self):
        self.containers = {
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

    def run_command(self, command: List[str]) -> str:
        return f"Mock command: {' '.join(command)}"

    def get_all_distroboxes(self) -> List[Dict]:
        return [{
            "name": name,
            "distro": data["distro"],
            "image_url": data["image_url"],
            "container_id": f"mock_{name}",
            "status": data["status"],
            "is_running": data["is_running"]
        } for name, data in self.containers.items()]

    def try_parse_distro_from_url(self, url: str) -> str:
        last = url.split('/')[-1].lower()
        for d in DISTROS:
            if d in last: return d
        for d in DISTROS:
            if d in url.lower(): return d
        return "unknown"

    def get_apps_in_box(self, box_name: str) -> List[str]:
        return self.containers.get(box_name, {}).get("exported_apps", [])

    def get_apps_from_container_fs(self, box_name: str) -> List[str]:
        container = self.containers.get(box_name, {})
        exported = set(container.get("exported_apps", []))
        available = set(container.get("available_apps", []))
        return list(available - exported)

    def get_available_images(self) -> List[Dict]:
        return [
            {'name': 'alpine:latest', 'url': 'docker.io/library/alpine:latest'},
            {'name': 'debian:stable', 'url': 'docker.io/library/debian:stable'},
            {'name': 'ubuntu:latest', 'url': 'docker.io/library/ubuntu:latest'},
            {'name': 'archlinux:latest', 'url': 'docker.io/library/archlinux:latest'},
            {'name': 'fedora:latest', 'url': 'docker.io/library/fedora:latest'},
        ]

    def enter_box(self, box_name: str) -> None:
        print(f"Mock: Entering container {box_name}")

    def run_in_box(self, command: str, box_name: str) -> None:
        print(f"Mock: Running '{command}' in {box_name}")

    def export_app(self, app_name: str, box_name: str) -> str:
        if box_name not in self.containers:
            return f"Error: Container {box_name} not found"
        
        if app_name in self.containers[box_name]["exported_apps"]:
            return f"Error: {app_name} already exported"
        
        if app_name not in self.containers[box_name]["available_apps"]:
            return f"Error: {app_name} not found in container"
        
        self.containers[box_name]["exported_apps"].append(app_name)
        return f"Successfully exported {app_name} from {box_name}"

    def remove_exported_app(self, app_name: str, box_name: str) -> str:
        if box_name not in self.containers:
            return f"Error: Container {box_name} not found"
        
        if app_name not in self.containers[box_name]["exported_apps"]:
            return f"Error: {app_name} not found in exported apps"
        
        self.containers[box_name]["exported_apps"].remove(app_name)
        return f"Successfully removed {app_name} from {box_name}"

    def upgrade_box(self, box_name: str) -> None:
        print(f"Mock: Upgrading container {box_name}")

    def delete_box(self, box_name: str) -> str:
        if box_name in self.containers:
            del self.containers[box_name]
            return f"Successfully deleted {box_name}"
        return f"Container {box_name} not found"

    def create_box(self, box_name: str, image: str, 
                  home_path: str = "", 
                  use_init: bool = False, 
                  volumes: Optional[List[str]] = None) -> str:
        if not box_name or not image:
            return "Error: Box name and image are required"
        
        self.containers[box_name] = {
            "distro": self.try_parse_distro_from_url(image),
            "image_url": image,
            "status": "running",
            "is_running": True,
            "exported_apps": [],
            "available_apps": ["bash", "ls", "cat"]
        }
        
        # Simulate entering the container
        self.enter_box(box_name)
        
        return f"""
        Created new container:
        - Name: {box_name}
        - Image: {image}
        - Home: {home_path or 'default'}
        - Init: {'enabled' if use_init else 'disabled'}
        - Volumes: {volumes or 'none'}
        """

    def assemble_box(self, ini_file: str) -> str:
        return f"Mock: Assembled containers from {ini_file}"

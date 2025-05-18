import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QPushButton, QListWidget, QListWidgetItem,
    QHBoxLayout, QVBoxLayout, QLabel, QLineEdit, QCheckBox, QScrollArea, QGroupBox,
    QMessageBox, QFrame
)
from PyQt6.QtGui import QIcon, QPixmap
from PyQt6.QtCore import Qt


class DistroboxBackend:
    def __init__(self, test=False):
        self.test = test
        self.test_boxes = []
        if self.test:
            self.test_boxes = [
                {
                    "name": "box-alpha",
                    "apps": [
                        {"name": "Firefox", "icon": "firefox", "exported": False},
                        {"name": "GNOME Terminal", "icon": "utilities-terminal", "exported": True},
                        {"name": "Gedit", "icon": "accessories-text-editor", "exported": True},
                        {"name": "VLC", "icon": "vlc", "exported": False},
                    ]
                },
                {
                    "name": "box-beta",
                    "apps": [
                        {"name": "Chromium", "icon": "chromium", "exported": False},
                        {"name": "Thunar", "icon": "folder", "exported": False},
                    ]
                }
            ]

    def list_distroboxes(self):
        if self.test:
            return self.test_boxes
        return []

    def add_distrobox(self, name: str, image: str) -> bool:
        if self.test:
            print(f"[TEST] Adding distrobox {name} with image {image}")
            self.test_boxes.append({"name": name, "apps": []})
            return True
        return False

    def remove_distrobox(self, name: str) -> bool:
        if self.test:
            print(f"[TEST] Removing distrobox {name}")
            self.test_boxes = [b for b in self.test_boxes if b["name"] != name]
            return True
        return False

    def enter_distrobox(self, name: str) -> bool:
        if self.test:
            print(f"[TEST] Entering {name}")
            return True
        return False

    def export_app(self, box_name: str, app_name: str) -> bool:
        if self.test:
            print(f"[TEST] Exporting app {app_name} from {box_name}")
            # Mark exported status true in test data
            for box in self.test_boxes:
                if box["name"] == box_name:
                    for app in box["apps"]:
                        if app["name"] == app_name:
                            app["exported"] = True
            return True
        return False

    def unexport_app(self, box_name: str, app_name: str) -> bool:
        if self.test:
            print(f"[TEST] Unexporting app {app_name} from {box_name}")
            for box in self.test_boxes:
                if box["name"] == box_name:
                    for app in box["apps"]:
                        if app["name"] == app_name:
                            app["exported"] = False
            return True
        return False


class AppWidget(QWidget):
    def __init__(self, box_name, app, backend, parent=None):
        super().__init__(parent)
        self.box_name = box_name
        self.app = app
        self.backend = backend
        self.setup_ui()

    def setup_ui(self):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(5, 2, 5, 2)
        layout.setSpacing(10)

        icon = QIcon.fromTheme(self.app["icon"])
        if icon.isNull():
            # fallback icon if theme icon missing
            icon = QIcon.fromTheme("application-x-executable")

        icon_label = QLabel()
        pix = icon.pixmap(24, 24)
        icon_label.setPixmap(pix)
        layout.addWidget(icon_label)

        name_label = QLabel(self.app["name"])
        layout.addWidget(name_label, stretch=1)

        self.checkbox = QCheckBox()
        self.checkbox.setChecked(self.app["exported"])
        self.checkbox.stateChanged.connect(self.on_checkbox_toggled)
        layout.addWidget(self.checkbox)

    def on_checkbox_toggled(self, state):
        if state == Qt.CheckState.Checked.value:
            success = self.backend.export_app(self.box_name, self.app["name"])
            if success:
                QMessageBox.information(self, "Exported", f"Exported {self.app['name']} from {self.box_name}")
            else:
                QMessageBox.warning(self, "Error", f"Failed to export {self.app['name']}")
                self.checkbox.setChecked(False)
        else:
            success = self.backend.unexport_app(self.box_name, self.app["name"])
            if success:
                QMessageBox.information(self, "Unexported", f"Unexported {self.app['name']} from {self.box_name}")
            else:
                QMessageBox.warning(self, "Error", f"Failed to unexport {self.app['name']}")
                self.checkbox.setChecked(True)


class DistroboxGroup(QGroupBox):
    def __init__(self, box_data, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self.box_data = box_data
        self.setTitle(box_data["name"])
        self.setCheckable(True)
        self.setChecked(False)
        self.setFlat(True)
        self.setStyleSheet("QGroupBox { font-weight: bold; }")

        self.app_container = QWidget()
        self.app_layout = QVBoxLayout(self.app_container)
        self.app_layout.setContentsMargins(20, 5, 5, 5)
        self.app_layout.setSpacing(2)

        for app in box_data["apps"]:
            app_widget = AppWidget(box_data["name"], app, backend)
            self.app_layout.addWidget(app_widget)

        layout = QVBoxLayout(self)
        layout.addWidget(self.app_container)

        self.toggled.connect(self.on_toggled)
        self.app_container.setVisible(False)

    def on_toggled(self, checked):
        self.app_container.setVisible(checked)


class MainWindow(QMainWindow):
    def __init__(self, backend=None):
        super().__init__()
        self.backend = backend or DistroboxBackend()
        self.distrobox_groups = []
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Distrobox Manager")
        self.resize(700, 500)

        central = QWidget()
        self.setCentralWidget(central)

        main_layout = QVBoxLayout(central)

        # Search bar
        self.search_bar = QLineEdit()
        self.search_bar.setPlaceholderText("Search apps...")
        self.search_bar.textChanged.connect(self.on_search)
        main_layout.addWidget(self.search_bar)

        # Scroll area for distroboxes
        self.scroll = QScrollArea()
        self.scroll.setWidgetResizable(True)
        main_layout.addWidget(self.scroll)

        self.container = QWidget()
        self.scroll.setWidget(self.container)
        self.vbox = QVBoxLayout(self.container)
        self.vbox.setContentsMargins(10, 10, 10, 10)
        self.vbox.setSpacing(10)

        # Buttons at bottom
        btn_layout = QHBoxLayout()
        main_layout.addLayout(btn_layout)

        create_btn = QPushButton(QIcon.fromTheme("list-add"), "Create Distrobox")
        create_btn.clicked.connect(self.show_add_dialog)
        btn_layout.addWidget(create_btn)

        refresh_btn = QPushButton(QIcon.fromTheme("view-refresh"), "Refresh")
        refresh_btn.clicked.connect(self.refresh_list)
        btn_layout.addWidget(refresh_btn)

        self.refresh_list()

    def refresh_list(self):
        # Clear previous groups
        for group in self.distrobox_groups:
            group.deleteLater()
        self.distrobox_groups.clear()

        boxes = self.backend.list_distroboxes()
        for box in boxes:
            group = DistroboxGroup(box, self.backend)
            self.vbox.addWidget(group)
            self.distrobox_groups.append(group)

        self.vbox.addStretch()

    def on_search(self, text):
        text = text.lower()
        for group in self.distrobox_groups:
            match_in_group = False
            for i in range(group.app_layout.count()):
                app_widget = group.app_layout.itemAt(i).widget()
                app_name = app_widget.app["name"].lower()
                visible = text in app_name
                app_widget.setVisible(visible)
                if visible:
                    match_in_group = True
            group.setVisible(match_in_group)

    def show_add_dialog(self):
        from PyQt6.QtWidgets import QDialog, QDialogButtonBox, QVBoxLayout, QLabel, QLineEdit

        class AddBoxDialog(QDialog):
            def __init__(self, parent=None):
                super().__init__(parent)
                self.setWindowTitle("Add Distrobox")
                self.name_field = QLineEdit(self)
                self.name_field.setPlaceholderText("Box name")
                self.image_field = QLineEdit(self)
                self.image_field.setPlaceholderText("Image (e.g., ubuntu:22.04)")

                button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
                button_box.accepted.connect(self.accept)
                button_box.rejected.connect(self.reject)

                layout = QVBoxLayout(self)
                layout.addWidget(QLabel("Name:"))
                layout.addWidget(self.name_field)
                layout.addWidget(QLabel("Image:"))
                layout.addWidget(self.image_field)
                layout.addWidget(button_box)

            def get_data(self):
                return self.name_field.text().strip(), self.image_field.text().strip()

        dialog = AddBoxDialog(self)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            name, image = dialog.get_data()
            if name and image:
                success = self.backend.add_distrobox(name, image)
                if success:
                    self.refresh_list()
                else:
                    QMessageBox.warning(self, "Error", "Failed to create distrobox.")


if __name__ == '__main__':
    test_mode = '--test' in sys.argv
    app = QApplication(sys.argv)
    backend = DistroboxBackend(test=test_mode)
    window = MainWindow(backend=backend)
    window.show()
    sys.exit(app.exec())

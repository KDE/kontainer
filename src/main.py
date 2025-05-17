import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QPushButton, QListWidget, QListWidgetItem,
    QHBoxLayout, QVBoxLayout, QLabel, QLineEdit, QDialog, QDialogButtonBox, QMessageBox, QListView
)
from PyQt6.QtGui import QIcon
from PyQt6.QtCore import Qt


class DistroboxBackend:
    """
    Backend logic stubs for Distrobox operations.
    Implement these methods to interact with actual distrobox CLI or API.
    """
    def list_distroboxes(self):
        # TODO: Return a list of existing distrobox names
        return []

    def add_distrobox(self, name: str, image: str) -> bool:
        # TODO: Create a new distrobox with given name and base image
        return False

    def remove_distrobox(self, name: str) -> bool:
        # TODO: Remove an existing distrobox by name
        return False

    def enter_distrobox(self, name: str) -> bool:
        # TODO: Enter the shell of a given distrobox
        return False

    def export_app(self, box_name: str, app_name: str) -> bool:
        # TODO: Export an app from the distrobox
        return False

    def unexport_app(self, box_name: str, app_name: str) -> bool:
        # TODO: Remove the exported app from the host system
        return False

    def open_package_file(self, box_name: str, package_path: str) -> bool:
        # TODO: Open and install a package file inside the distrobox
        return False


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


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.backend = DistroboxBackend()
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Distrobox Manager")
        self.resize(600, 400)

        # Central widget
        central = QWidget()
        self.setCentralWidget(central)
        vbox = QVBoxLayout(central)

        # Create button
        create_btn = QPushButton(QIcon.fromTheme("list-add"), "Create Distrobox")
        create_btn.clicked.connect(self.show_add_dialog)
        vbox.addWidget(create_btn, alignment=Qt.AlignmentFlag.AlignRight)

        # List of distroboxes
        self.list_widget = QListWidget()
        vbox.addWidget(self.list_widget)

        # Populate list
        self.refresh_list()

    def refresh_list(self):
        self.list_widget.clear()
        for name in self.backend.list_distroboxes():
            item = QListWidgetItem(name)
            self.list_widget.addItem(item)

            widget = QWidget()
            hbox = QHBoxLayout(widget)
            hbox.setContentsMargins(0, 0, 0, 0)

            enter_btn = QPushButton(QIcon.fromTheme("system-run"), "Enter")
            enter_btn.clicked.connect(lambda _, n=name: self.backend.enter_distrobox(n))
            hbox.addWidget(enter_btn)

            clone_btn = QPushButton(QIcon.fromTheme("edit-copy"), "Clone")
            clone_btn.clicked.connect(lambda _, n=name: QMessageBox.information(self, "Clone", f"Clone {n}"))
            hbox.addWidget(clone_btn)

            update_btn = QPushButton(QIcon.fromTheme("view-refresh"), "Update")
            update_btn.clicked.connect(lambda _, n=name: QMessageBox.information(self, "Update", f"Update {n}"))
            hbox.addWidget(update_btn)

            delete_btn = QPushButton(QIcon.fromTheme("edit-delete"), "Delete")
            delete_btn.clicked.connect(lambda _, n=name: self.remove_distrobox(n))
            hbox.addWidget(delete_btn)

            export_btn = QPushButton(QIcon.fromTheme("application-export"), "Export Apps")
            export_btn.clicked.connect(lambda _, n=name: QMessageBox.information(self, "Export", f"Export apps from {n}"))
            hbox.addWidget(export_btn)

            self.list_widget.setItemWidget(item, widget)

    def show_add_dialog(self):
        dialog = AddBoxDialog(self)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            name, image = dialog.get_data()
            if name and image:
                success = self.backend.add_distrobox(name, image)
                if success:
                    self.refresh_list()
                else:
                    QMessageBox.warning(self, "Error", "Failed to create distrobox.")

    def remove_distrobox(self, name):
        confirm = QMessageBox.question(self, "Delete", f"Are you sure you want to delete '{name}'?")
        if confirm == QMessageBox.StandardButton.Yes:
            success = self.backend.remove_distrobox(name)
            if success:
                self.refresh_list()
            else:
                QMessageBox.warning(self, "Error", "Failed to delete distrobox.")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


from PyQt6.QtWidgets import (
    QDialog, QFormLayout, QLineEdit, QComboBox, 
    QCheckBox, QDialogButtonBox, QMessageBox
)
from PyQt6.QtCore import Qt

class CreateBoxDialog(QDialog):
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self.setWindowTitle("Create New DistroBox")
        self._setup_ui()

    def _setup_ui(self):
        layout = QFormLayout(self)
        
        self.name_input = QLineEdit()
        self.image_combo = QComboBox()
        self.home_input = QLineEdit()
        self.volumes_input = QLineEdit()
        self.init_checkbox = QCheckBox("Enable systemd init")
        
        try:
            images = self.backend.get_available_images()
            for img in images:
                self.image_combo.addItem(img.get('url', 'N/A'))
        except Exception as e:
            QMessageBox.warning(self, "Warning", f"Couldn't load images: {str(e)}")
            self.image_combo.addItem("docker.io/library/alpine:latest")
        
        layout.addRow("Box Name:", self.name_input)
        layout.addRow("Image:", self.image_combo)
        layout.addRow("Custom Home Path:", self.home_input)
        layout.addRow("Additional Volumes:", self.volumes_input)
        layout.addRow(self.init_checkbox)
        
        button_box = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | 
            QDialogButtonBox.StandardButton.Cancel
        )
        button_box.accepted.connect(self._validate)
        button_box.rejected.connect(self.reject)
        layout.addRow(button_box)

    def _validate(self):
        if not self.name_input.text() or not self.image_combo.currentText():
            QMessageBox.warning(self, "Error", "Name and image are required")
            return
        self.accept()

    def get_data(self):
        return {
            'name': self.name_input.text(),
            'image': self.image_combo.currentText(),
            'home': self.home_input.text(),
            'volumes': [
                v.strip() 
                for v in self.volumes_input.text().split(',') 
                if v.strip()
            ],
            'use_init': self.init_checkbox.isChecked()
        }

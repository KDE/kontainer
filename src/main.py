import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QListWidget, QVBoxLayout, QWidget, QMenu,
    QMessageBox, QPushButton, QHBoxLayout, QTreeWidget, QTreeWidgetItem,
    QSplitter, QTabWidget, QLabel, QLineEdit, QComboBox, QTextEdit,
    QToolBar, QStatusBar, QDialog, QFormLayout, QCheckBox, QDialogButtonBox
)
from PyQt6.QtCore import Qt, QSize
from PyQt6.QtGui import QIcon, QAction

# Use mock backend if --test flag is passed
test_mode = "--test" in sys.argv
if test_mode:
    import distrobox_backend_mock as backend
else:
    import distrobox_backend as backend

class DistroBoxManager(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DistroBox Manager")
        self.setGeometry(100, 100, 800, 600)
        self.current_box = None
        self.init_ui()
        self.load_containers()
        self.create_toolbar()
        self.create_statusbar()

    def init_ui(self):
        # Main central widget with splitter
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QHBoxLayout(self.central_widget)
        
        # Splitter for left/right panels
        self.splitter = QSplitter(Qt.Orientation.Horizontal)
        self.main_layout.addWidget(self.splitter)

        # Left panel - Container list
        self.left_panel = QWidget()
        self.left_layout = QVBoxLayout(self.left_panel)
        
        # Search box
        self.search_box = QLineEdit()
        self.search_box.setPlaceholderText("Search containers...")
        self.search_box.textChanged.connect(self.filter_containers)
        self.left_layout.addWidget(self.search_box)

        # Container tree widget
        self.container_tree = QTreeWidget()
        self.container_tree.setHeaderLabels(["Name", "Distro", "Status"])
        self.container_tree.setColumnWidth(0, 200)
        self.container_tree.setColumnWidth(1, 100)
        self.container_tree.setColumnWidth(2, 100)
        self.container_tree.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.container_tree.customContextMenuRequested.connect(self.show_context_menu)
        self.container_tree.itemClicked.connect(self.show_box_details)
        self.left_layout.addWidget(self.container_tree)

        # Right panel - Details view
        self.right_panel = QTabWidget()
        
        # Details tab
        self.details_tab = QWidget()
        self.details_layout = QVBoxLayout(self.details_tab)
        self.details_text = QTextEdit()
        self.details_text.setReadOnly(True)
        self.details_layout.addWidget(self.details_text)
        self.right_panel.addTab(self.details_tab, "Details")
        
        # Applications tab
        self.apps_tab = QWidget()
        self.apps_layout = QVBoxLayout(self.apps_tab)
        
        # Tab widget for Available/Exported apps
        self.apps_tabs = QTabWidget()
        
        # Available apps tab
        self.available_apps_tab = QWidget()
        self.available_apps_layout = QVBoxLayout(self.available_apps_tab)
        self.available_apps_list = QListWidget()
        self.export_button = QPushButton("Export Selected App")
        self.export_button.clicked.connect(self.export_selected_app)
        self.available_apps_layout.addWidget(self.available_apps_list)
        self.available_apps_layout.addWidget(self.export_button)
        self.apps_tabs.addTab(self.available_apps_tab, "Available Apps")
        
        # Exported apps tab
        self.exported_apps_tab = QWidget()
        self.exported_apps_layout = QVBoxLayout(self.exported_apps_tab)
        self.exported_apps_list = QListWidget()
        self.remove_button = QPushButton("Remove Selected App")
        self.remove_button.clicked.connect(self.remove_selected_app)
        self.exported_apps_layout.addWidget(self.exported_apps_list)
        self.exported_apps_layout.addWidget(self.remove_button)
        self.apps_tabs.addTab(self.exported_apps_tab, "Exported Apps")
        
        self.apps_layout.addWidget(self.apps_tabs)
        self.right_panel.addTab(self.apps_tab, "Applications")
        
        # Add panels to splitter
        self.splitter.addWidget(self.left_panel)
        self.splitter.addWidget(self.right_panel)
        self.splitter.setSizes([300, 500])

        # Bottom buttons
        self.button_layout = QHBoxLayout()
        self.create_box_button = QPushButton("Create New Box")
        self.create_box_button.setIcon(QIcon.fromTheme("list-add"))
        self.create_box_button.clicked.connect(self.create_box_dialog)
        self.refresh_button = QPushButton("Refresh")
        self.refresh_button.setIcon(QIcon.fromTheme("view-refresh"))
        self.refresh_button.clicked.connect(self.load_containers)
        self.button_layout.addWidget(self.create_box_button)
        self.button_layout.addWidget(self.refresh_button)
        self.left_layout.addLayout(self.button_layout)

    def create_toolbar(self):
        toolbar = QToolBar("Main Toolbar")
        toolbar.setIconSize(QSize(16, 16))
        self.addToolBar(toolbar)

        # Container actions
        self.enter_action = QAction(QIcon.fromTheme("utilities-terminal"), "Enter", self)
        self.enter_action.triggered.connect(self.enter_current_box)
        toolbar.addAction(self.enter_action)

        self.upgrade_action = QAction(QIcon.fromTheme("system-upgrade"), "Upgrade", self)
        self.upgrade_action.triggered.connect(self.upgrade_current_box)
        toolbar.addAction(self.upgrade_action)

        self.delete_action = QAction(QIcon.fromTheme("edit-delete"), "Delete", self)
        self.delete_action.triggered.connect(self.delete_current_box)
        toolbar.addAction(self.delete_action)

    def create_statusbar(self):
        self.statusBar().showMessage("Ready")

    def load_containers(self):
        self.container_tree.clear()
        try:
            boxes = backend.get_all_distroboxes()
            for box in boxes:
                # Ensure all required fields are present
                box.setdefault('image_url', 'N/A')
                box.setdefault('container_id', 'N/A')
                box.setdefault('is_running', False)
                
                item = QTreeWidgetItem([
                    box.get('name', 'N/A'),
                    box.get('distro', 'N/A'),
                    box.get('status', 'N/A')
                ])
                item.setData(0, Qt.ItemDataRole.UserRole, box)
                self.container_tree.addTopLevelItem(item)
            
            self.statusBar().showMessage(f"Loaded {len(boxes)} containers")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to load containers:\n{str(e)}")
            self.statusBar().showMessage("Failed to load containers")

    def filter_containers(self):
        filter_text = self.search_box.text().lower()
        for i in range(self.container_tree.topLevelItemCount()):
            item = self.container_tree.topLevelItem(i)
            name = item.text(0).lower()
            distro = item.text(1).lower()
            item.setHidden(filter_text not in name and filter_text not in distro)

    def show_context_menu(self, pos):
        item = self.container_tree.itemAt(pos)
        if not item:
            return
        
        self.current_box = item.text(0)
        menu = QMenu(self)
        
        menu.addAction(QIcon.fromTheme("utilities-terminal"), "Open Terminal", 
                      lambda: backend.enter_box(self.current_box))
        menu.addAction(QIcon.fromTheme("system-upgrade"), "Upgrade Box", 
                      lambda: backend.upgrade_box(self.current_box))
        menu.addAction(QIcon.fromTheme("edit-delete"), "Delete Box", 
                      lambda: self.delete_box(self.current_box))
        
        menu.exec(self.container_tree.mapToGlobal(pos))

    def show_box_details(self, item, column):
        try:
            box_data = item.data(0, Qt.ItemDataRole.UserRole)
            self.current_box = box_data.get('name', 'N/A')
            
            details = f"""
            <b>Name:</b> {box_data.get('name', 'N/A')}<br>
            <b>Distribution:</b> {box_data.get('distro', 'N/A')}<br>
            <b>Status:</b> {box_data.get('status', 'N/A')}<br>
            <b>Image:</b> {box_data.get('image_url', 'N/A')}<br>
            <b>Container ID:</b> {box_data.get('container_id', 'N/A')}<br>
            """
            self.details_text.setHtml(details)
            
            # Load apps for this container
            self.load_apps_for_box(box_data.get('name', ''))
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to show box details:\n{str(e)}")

    def load_apps_for_box(self, box_name):
        """Load both available and exported apps for the container"""
        try:
            # Load available apps (not yet exported)
            self.available_apps_list.clear()
            available_apps = backend.get_apps_from_container_fs(box_name)
            if available_apps:
                self.available_apps_list.addItems(available_apps)
            else:
                self.available_apps_list.addItem("No applications available to export")
            
            # Load exported apps
            self.exported_apps_list.clear()
            exported_apps = backend.get_apps_in_box(box_name)
            if exported_apps:
                self.exported_apps_list.addItems(exported_apps)
            else:
                self.exported_apps_list.addItem("No applications currently exported")
            
        except Exception as e:
            self.available_apps_list.addItem(f"Error loading available apps: {str(e)}")
            self.exported_apps_list.addItem(f"Error loading exported apps: {str(e)}")

    def export_selected_app(self):
        """Export the currently selected available app"""
        if not self.current_box:
            QMessageBox.warning(self, "No Container", "Please select a container first")
            return
            
        selected = self.available_apps_list.currentItem()
        if not selected or selected.text().startswith("No applications"):
            QMessageBox.warning(self, "No Selection", "Please select an app to export")
            return
            
        app_name = selected.text()
        try:
            result = backend.export_app(app_name, self.current_box)
            QMessageBox.information(self, "Export App", result)
            self.load_apps_for_box(self.current_box)
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to export app:\n{str(e)}")

    def remove_selected_app(self):
        """Remove the currently selected exported app"""
        if not self.current_box:
            QMessageBox.warning(self, "No Container", "Please select a container first")
            return
            
        selected = self.exported_apps_list.currentItem()
        if not selected or selected.text().startswith("No applications"):
            QMessageBox.warning(self, "No Selection", "Please select an app to remove")
            return
            
        app_name = selected.text()
        try:
            result = backend.remove_exported_app(app_name, self.current_box)
            QMessageBox.information(self, "Remove App", result)
            self.load_apps_for_box(self.current_box)
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to remove app:\n{str(e)}")

    def create_box_dialog(self):
        dialog = QDialog(self)
        dialog.setWindowTitle("Create New DistroBox")
        layout = QFormLayout(dialog)
        
        # Form fields
        self.name_input = QLineEdit()
        self.image_combo = QComboBox()
        self.home_input = QLineEdit()
        self.volumes_input = QLineEdit()
        self.init_checkbox = QCheckBox("Enable systemd init")
        
        # Populate image combo
        try:
            images = backend.get_available_images()
            for img in images:
                self.image_combo.addItem(img.get('url', 'N/A'))
        except Exception as e:
            QMessageBox.warning(self, "Warning", f"Couldn't load available images: {str(e)}")
            self.image_combo.addItem("docker.io/library/alpine:latest")
        
        # Add fields to form
        layout.addRow("Box Name:", self.name_input)
        layout.addRow("Image:", self.image_combo)
        layout.addRow("Custom Home Path:", self.home_input)
        layout.addRow("Additional Volumes (comma separated):", self.volumes_input)
        layout.addRow(self.init_checkbox)
        
        # Buttons
        button_box = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        button_box.accepted.connect(lambda: self.create_box(dialog))
        button_box.rejected.connect(dialog.reject)
        layout.addRow(button_box)
        
        dialog.exec()

    def create_box(self, dialog):
        name = self.name_input.text()
        image = self.image_combo.currentText()
        home = self.home_input.text()
        volumes = [v.strip() for v in self.volumes_input.text().split(',') if v.strip()]
        use_init = self.init_checkbox.isChecked()
        
        if not name or not image:
            QMessageBox.warning(self, "Error", "Name and image are required")
            return
            
        try:
            out = backend.create_box(name, image, home, use_init, volumes)
            QMessageBox.information(self, "Success", f"Box created successfully:\n{out}")
            self.load_containers()
            dialog.accept()
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to create box:\n{str(e)}")

    def delete_box(self, box_name):
        reply = QMessageBox.question(
            self, "Delete Box",
            f"Are you sure you want to delete {box_name}?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )
        if reply == QMessageBox.StandardButton.Yes:
            try:
                out = backend.delete_box(box_name)
                QMessageBox.information(self, "Deleted", out)
                self.load_containers()
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to delete box:\n{str(e)}")

    def enter_current_box(self):
        if self.current_box:
            backend.enter_box(self.current_box)

    def upgrade_current_box(self):
        if self.current_box:
            backend.upgrade_box(self.current_box)

    def delete_current_box(self):
        if self.current_box:
            self.delete_box(self.current_box)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = DistroBoxManager()
    window.show()
    sys.exit(app.exec())

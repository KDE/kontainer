import sys
from PyQt6.QtWidgets import QApplication

# Use mock backend if --test flag is passed
test_mode = "--test" in sys.argv
if test_mode:
    import backend_mock as backend
else:
    import backend

from ui.main_window import MainWindow

def main():
    app = QApplication(sys.argv)
    
    window = MainWindow(backend)
    window.show()
    
    sys.exit(app.exec())

if __name__ == '__main__':
    main()

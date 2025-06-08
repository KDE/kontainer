# Distrobox Qt GUI

This is a **fully-featured** graphical user interface for [Distrobox](https://github.com/89luca89/distrobox), written in **C++** using **Qt 6.9**.

Inspired by [BoxBuddy](https://github.com/feaneron/boxbuddy), this application now reaches **feature parity**, providing a clean, native, and powerful GUI for managing your Distrobox containers — with seamless integration into KDE Plasma and adherence to KDE development standards.

## Project Status

- ✅ **Backend:** Fully implemented, including container and app management  
- ✅ **UI:** Functional and stable, built with Qt 6.9 (C++)  
- ✅ **Features:** Parity with BoxBuddy (create, delete, enter, upgrade, export/unexport apps)  
- ✅ **Refactoring:** Planned to improve modularity and maintainability    
- ✅ **Configuration system:** Planned — support for dynamic user preferences  
- ⏳ **i18n:** Ongoing work  
- ⏳ **Packaging:** Flatpak support planned

## Features

- ✅ View all Distrobox containers  
- ✅ Create new containers  
- ✅ Enter containers (interactive shell)  
- ✅ Delete containers  
- ✅ Export and unexport container apps  
- ✅ Upgrade all containers  
- ✅ Manage apps via host or container context  
- ⏳ KDE-style full i18n (using `KLocalizedString`)  
- ❌ No automatic Flatpak manifest generation (yet)

## Goals

- Adhere to [KDE Coding Style](https://community.kde.org/Policies/Frameworks_Coding_Style)  
- Use KDE’s **KLocalizedString** or Qt-compatible i18n tooling  
- Integrate with KDE system themes, icons, and color schemes  
- Enable user-friendly configuration and customization  
- Package as a **Flatpak** for sandboxed deployment

## Requirements

- Qt 6.9 or higher (with C++17 support)  
- A working `distrobox` installation

## Build Instructions

1. Ensure Qt 6.9+ is installed  
2. Clone and enter the project directory  
3. Build the project:

```bash
cmake .
make
```

4. This will build the project completely and produce the executable.

## License

Licensed under the **GNU General Public License v2.0 or later**. See `LICENSE` for details.

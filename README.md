# Distrobox Qt GUI (Work in Progress)

This is a **work-in-progress** graphical user interface for [Distrobox](https://github.com/89luca89/distrobox), now rewritten in **C++** using **Qt 6.9**. The UI skeleton is completed, but a refactor is planned to improve structure and maintainability.

The goal remains to provide a native, clean, and powerful GUI for managing your Distrobox containers — creating, entering, exporting apps, and more — with seamless integration into KDE Plasma and adherence to KDE development standards.

## Project Status

- **Backend:** Fully implemented  
- **UI:** Basic skeleton completed in Qt 6.9 (C++)  
- **Refactoring:** Planned to improve code quality and modularity  
- **Configuration:** Planned — will support dynamic settings to reduce hardcoding  
- **Quality-of-life:** KDE coding standards, system integration, and internationalization (i18n) support ongoing  
- **Packaging:** Flatpak packaging planned for portable, sandboxed deployment

## Features

- ✅ View all Distrobox containers  
- ✅ Create new containers  
- ✅ Enter, export, delete, and upgrade containers  
- ✅ Manage apps from host or container filesystem  
- ⏳ KDE-style UI and full translation/i18n integration in progress  
- ⏳ Dynamic configuration and user preferences  
- ❌ No automatic Flatpak manifest generation yet

## Goals

- Follow [KDE Coding Style](https://community.kde.org/Policies/Frameworks_Coding_Style)  
- Use KDE’s **KLocalizedString** or Qt-compatible i18n tooling  
- Integrate with system icon themes and KDE color schemes  
- Offer user-driven configuration for images, volumes, and preferences  
- Package as a **Flatpak** for sandboxed and portable use

## Requirements

- Qt 6.9 or higher (with C++17 support)  
- A working `distrobox` installation

## Build Instructions

1. Make sure Qt 6.9 (or newer) is installed on your system.  
2. Inside the project directory, run:

```bash
cmake .
make
```

3. This will build the project completely and produce the executable.

## License

Licensed under the **GNU General Public License v2.0 or later**. See `LICENSE` for details.

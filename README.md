# Distrobox Qt GUI (Work in Progress)

This is a **work-in-progress** graphical user interface for [Distrobox](https://github.com/89luca89/distrobox), built with **Python** and **PyQt6**, designed to seamlessly integrate with **KDE Plasma** and follow **KDE development standards**.

The goal is to provide a native, clean, and powerful GUI for managing your Distrobox containers — creating, entering, exporting apps, and more — with proper support for KDE’s aesthetics, translations, and development practices.

## Project Status

- **Backend:** Fully implemented
- **UI:** Complete rework in progress (KDE-style)
- **Configuration:** Planned – will support dynamic settings to reduce hardcoding
- **Quality-of-life:** KDE coding standards, system integration, and internationalization (i18n) support in progress
- **Packaging:** Flatpak packaging planned for portable, sandboxed deployment

## Features

- ✅ View all Distroboxes
- ✅ Create new containers
- ✅ Enter, export, delete, and upgrade containers
- ✅ List and manage apps from host or container filesystem
- ⏳ KDE-style UI and full translation/i18n integration
- ⏳ Dynamic configuration and user preferences
- ❌ No automatic Flatpak manifest generation (yet)

## Goals

- Follow [KDE Coding Style](https://community.kde.org/Policies/Frameworks_Coding_Style)
- Use KDE’s **KLocalizedString** or PyQt-compatible i18n tooling
- Integrate with **system icon themes** and **KDE color schemes**
- Offer user-driven configuration for images, volumes, and preferences
- Package as a **Flatpak** for sandboxed and portable use

## Requirements

- Python 3.8+
- PyQt6
- A working `distrobox` installation

Install PyQt6 via pip:

```bash
pip install PyQt6
```
## License

Licensed under the **GNU General Public License v2.0 or later**. See `LICENSE` for details.

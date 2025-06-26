# Kontainer – Qt Container Manager for Distrobox & Toolbox

Kontainer is a **feature-rich graphical interface** for managing containerized environments using [Distrobox](https://github.com/89luca89/distrobox) and [Toolbox](https://github.com/containers/toolbox), written in **C++ with Qt 6.9**.

> 🧠 **Inspired by** [BoxBuddy](https://github.com/Dvlv/BoxBuddyRS) and later by [DistroShelf](https://github.com/kirbylife/distroshelf), Kontainer has **surpassed BoxBuddy** in both features and usability, and is **close to surpassing DistroShelf**, with ongoing work on Toolbox integration, optimizations, and KDE-native APIs.

---

## 🧪 Project Status

- **Backends:**  
  - ✅ **Distrobox:** Fully supported  
  - 🧪 **Toolbox:** Experimental, nearly complete (export/unexport, container management, RPM/DEB/pkg.tar install support)
- **UI:**  
  - ✅ Qt 6.9 with strong KDE integration  
- **Feature Set:**  
  - ✅ Surpasses BoxBuddy  
  - ⚙️ Almost on par with DistroShelf — Toolbox upgrade and deeper API integration planned  
- **i18n:**  
  - ✅ KDE-style translations with `KLocalizedString`  
- **User Configuration:**  
  - ✅ Users can choose backend and terminal emulator  
- **Packaging:**  
  - ✅ Flatpak (recommended)  
  - ✅ AUR available

---

## ✨ Features

- View, create, delete, and enter containers  
- Export and unexport apps (including Toolbox apps — downstream addition)  
- Install `.rpm`, `.deb`, `.pkg.tar` packages into containers  
- Full Distrobox backend support including upgrades  
- Toolbox backend (experimental but usable)  
- App and Distro icons with proper export handling  
- KDE Plasma integration and theming  
- Configurable terminal backend and container backend  
- KDE i18n via `KLocalizedString`

---

## 🚧 Roadmap

- Replace hardcoded logic with KDE APIs  
- Add **Toolbox upgrade** capability  
- Optimize Toolbox backend and final stabilization  
- Possibly rewrite UI in **Kirigami** for responsiveness  
- Move past KDE Playground and go through **official KDE review**  
- Expand user settings and UX polish

> ⚠️ **Note:** Kontainer is currently in **Playground** stage. Please do **not ship or package it** until it passes KDE Review and is marked stable.

---

## 📦 Build & Install

### 🟢 Flatpak (Recommended)

```bash
git clone https://invent.kde.org/system/kontainer
flatpak install org.kde.Sdk//6.9
flatpak-builder --force-clean build-dir kontainer.flatpak.yaml --user --install
```

### 🟡 AUR (Arch Linux)

```bash
yay -S kontainer-git
```

---

## 🔧 Build from Source

1. Install Qt 6.9+ with C++17 support  
2. Clone and build:

```bash
git clone https://invent.kde.org/system/kontainer
cd kontainer
cmake .
make
```

---

## 📜 License

Licensed under **GPL-2.0-or-later**. See `LICENSES/` and SPDX headers.

---

## 🖼️ Trademark & Attribution

All icons and branding are owned by their respective trademark holders. Justifications and documentation are located in `res/trademarks/`, organized per distro.

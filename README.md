# ⚠️ Kontainer is **ALPHA Software** ⚠️

**This project is under heavy development.**
- 🐞 There are still **bugs and regressions** in the application  
- 🚧 Kontainer is **feature frozen** until core issues are fixed  
- ⏳ Development may move slower since the lead dev (Hadi Chokr) is now employed  
- 📦 Releases are **for progress tracking only**, *not stable builds*  

---

# Kontainer – Qt Container Manager  

Kontainer is a **Qt-based GUI** for managing container environments.  
It is free and open source, but still in an early alpha state.  

---

## ✨ Current Features  

- 🗂️ View, create, delete, and enter containers  
- 📤 Export and 📥 unexport applications  
- 📦 Install `.rpm`, `.deb`, `.pkg.tar` into containers  
- 🟢 Distrobox backend support  
- 🟡 Toolbox backend (incomplete)  
- 🎨 KDE Plasma integration, theming, and translations  
- ⚙️ Configurable backend and terminal  

> ⚠️ **Limitations right now:**  
> - No-terminal mode is temporarily broken  
> - Not all operations are async → some UI freezes may occur  

---

## 🗺️ Roadmap  

- 🔧 Bug fixes & stabilization  
- 🔄 Make all operations fully async  
- 📦 Complete Toolbox backend support  
- 📱 Rewrite UI in **Kirigami** for responsiveness  
- ✅ Lift feature freeze once the above are complete  

---

## 📦 Build & Install  

### 🟢 Flatpak Builder (for development)  

```bash
git clone https://invent.kde.org/system/kontainer
flatpak install org.kde.Sdk//6.9
flatpak-builder --force-clean build-dir kontainer.flatpak.yaml --user --install
```  

### 🔧 Build from Source  

1. Install Qt 6.9+ with C++17 support  
2. Clone and build:  

```bash
git clone https://invent.kde.org/system/kontainer
cd kontainer
cmake .
make
```  

### 🛠️ Using kde-builder  

Alternatively, you can use [kde-builder](https://invent.kde.org/sdk/kde-builder):  

```bash
kde-builder kontainer
```  

---

## 📜 License  

Licensed under **GPL-2.0-or-later**. See `LICENSES/` and SPDX headers.  

---

## 🖼️ Trademark & Attribution  

All icons and branding are owned by their respective trademark holders.  
Documentation is located in `res/trademarks/`, organized per distro.  

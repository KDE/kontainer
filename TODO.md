## ✅ Implemented

### 🔹 MainUI
- Backend Selector if multiple backends available
- Remember backend preference

### 🔹 Backend
- Return available backends
- Use the backend variable from MainUI:
  - getContainers()
  - parseDistrofromImage()
  - createContainer()
  - deleteContainer()
  - assembleContainer()
    ⚠️ No Toolbox equivalent → hide button in MainUI
  - enterContainer()
  - upgradeContainer()
    ⚠️ No Toolbox equivalent → hide button in MainUI
  - upgradeAllContainers()
    ⚠️ No Toolbox equivalent → hide button in MainUI
  - getAvailableApps()
  - installDebPackage() and no-terminal version - **Apts CLI Interface is unstable and leads to problems with the output dialog**
  - installRpmPackage() and no-terminal version
  - installArchPackage() and no-terminal version
  - getAvailableImages() → **hardcode** in header for now
---

## 🚧 TODO

### 🔹 Backend
- Implement functionality in Upstream Toolbox:
  - getExportedApps() → 🧠 Implement & upstream — essentially a wrapper script exporting to `.local/share/applications`
  - exportApp() → same as above
  - unexportApp() → same as above

---

### 🔹 Container Dialog UI

- Container creation: hide unsupported options:
  - CreateContainerDialog() - **The new Toolbox List doesnt appear properly**
  - Possibly startContainerCreation()


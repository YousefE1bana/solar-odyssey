# Solar Odyssey - Documentation Index

**Author:** Yousef Osama  
**Position:** Cybersecurity Engineer  
**University:** Egyptian Chinese University

---

## 📚 Main Documentation

| File | Purpose | Target Audience |
|:---|:---|:---|
| **[`README.md`](../README.md)** | Primary project overview, features, keybind map, build guide | Everyone |
| **[`SOLAR_ODYSSEY_ROADMAP_V2.md`](../SOLAR_ODYSSEY_ROADMAP_V2.md)** | Master architectural roadmap & cycle specifications | Developers & Architects |
| **[`THIRD_PARTY_NOTICES.md`](../THIRD_PARTY_NOTICES.md)** | Third-party licenses, attributions, and asset provenance | Legal / Compliance |
| **[`Documentation/COMPLETE_GUIDE.md`](COMPLETE_GUIDE.md)** | Comprehensive architectural guide, audio, and controls | Developers |
| **[`Documentation/FILE_OVERVIEW.md`](FILE_OVERVIEW.md)** | Detailed structural map of all headers, sources, and shaders | Developers & Reviewers |
| **[`Documentation/QUICK_SETUP.md`](QUICK_SETUP.md)** | Rapid MSYS2/CMake installation instructions | Users & Developers |
| **[`Documentation/requirements.md`](requirements.md)** | Minimum and recommended system prerequisites | Users |
| **[`docs/verification/CYCLE_0_BASELINE_REPORT.md`](../docs/verification/CYCLE_0_BASELINE_REPORT.md)** | Cycle 0 verification, deterministic benchmarks, and visual regression report | QA & Maintainers |

---

## ⚡ Build & Verification Scripts

| Script | Purpose | Platform |
|:---|:---|:---|
| **[`build.bat`](../build.bat)** | Fast MinGW build script compiling main application & test suite | Windows (MinGW-w64) |
| **[`CMakeLists.txt`](../CMakeLists.txt)** | Standard cross-platform CMake build configuration | Windows / Linux |
| **[`tools/visual_regression/run_regression.py`](../tools/visual_regression/run_regression.py)** | 4-scene automated visual regression testing harness | Python 3 |
| **[`tools/run_benchmarks.py`](../tools/run_benchmarks.py)** | 8-scene official 1000-frame deterministic benchmark suite | Python 3 |

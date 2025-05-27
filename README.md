<div align="left">
    <img src="https://img.icons8.com/?size=512&id=55494&format=png" width="40%" align="left" style="margin-right: 15px"/>
    <div style="display: inline-block;">
        <h2 style="display: inline-block; vertical-align: middle; margin-top: 0;">Map Editor Migration: wxwidgets to project_qt (Qt6)</h2>
        <p>
	<em><code>Qt6-based Map Editor migrated from wxWidgets, SFML & ImGui</code></em>
</p>
        <p>
	<img src="https://img.shields.io/github/license/sword-of-fury/project_qt?style=default&logo=opensourceinitiative&logoColor=white&color=0080ff" alt="license">
	<img src="https://img.shields.io/github/last-commit/sword-of-fury/project_qt?style=default&logo=git&logoColor=white&color=0080ff" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/sword-of-fury/project_qt?style=default&color=0080ff" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/sword-of-fury/project_qt?style=default&color=0080ff" alt="repo-language-count">
</p>
        <p><!-- default option, no dependency badges. -->
</p>
        <p>
	<!-- default option, no dependency badges. -->
</p>
    </div>
</div>
<br clear="left"/>


---

## 📍 Overview

This repository is dedicated to the **Qt6-based re-implementation** of a 2D map editor for tile-based game worlds, specifically designed for map formats used by **Tibia** server distributions (e.g., [otland/forgottenserver](https://github.com/otland/forgottenserver)).

The `project_qt` directory on the `main` branch is an ongoing migration project to fully transition the editor from its original C++ codebase found in the `wxwidgets` directory. This original codebase utilized `wxWidgets` for the graphical user interface, `SFML` for high-performance 2D rendering, and `ImGui` for immediate-mode, in-canvas UI elements. The overarching goal is to transform it into a modern, unified `Qt6` application, providing a more user-friendly, extensible, and cross-platform desktop experience without compromising existing functionalities.

---

## ✨ Core Purpose

The editor's primary role is to empower content creators to visually design, edit, and manage game maps. This includes comprehensive support for:

*   **Map Loading & Saving:** Handling Tibia-specific map formats (`.otbm`, and related XML files for spawns and houses).
*   **Layered Map Display:** Visualizing multiple Z-layers (e.g., ground, items, creatures, effects, roofs) with control over individual layer visibility.
*   **Map Navigation:** Intuitive pan and zoom controls for seamless exploration of large map areas.
*   **Diverse Brushes:** A range of editing tools, including standard painting brushes, erasers, flood-fill tools, line drawing (pencil brush), and specialized selection tools.
*   **Selection & Transformation:** Selecting map areas, performing clipboard operations (copy, cut, paste), and geometric transformations (move, rotate, flip).
*   **Contextual Interaction:** Dynamic right-click menus that offer context-sensitive actions based on the specific map elements under the cursor.
*   **Data Management:** Seamless integration with and loading from various game data files, handled by specialized item, creature, and sprite managers.
*   **Automation Features:** An "Automagic" system for intelligent, automatic placement and adjustment of border tiles and walls, greatly streamlining the mapping process.
*   **Undo/Redo System:** A robust command pattern-based system providing a full history of map modifications for error recovery.

---

## 🚀 Technologies

This migration project predominantly leverages:

*   **Primary Framework:** **Qt6 (C++)** - The chosen framework for all aspects of GUI development, 2D rendering (`QGraphicsView`), event handling, cross-platform compatibility, and foundational services like file I/O and networking.
*   **Build System:** **CMake** - Used to manage the compilation process, ensuring compatibility across various operating systems and development environments.
*   **External Libraries (Now Integrated/Replaced where needed in `project_qt`):**
    *   `spdlog`: A fast, header-only C++ logging library (carried over).
    *   `nlohmann/json`: Modern C++ JSON library (usage replaced by Qt's `QJson` module).
    *   `cereal`: A C++11 serialization library (usage evaluated and potentially replaced or carried over for specific formats).
    *   **Removed/Replaced in `project_qt`:** `wxWidgets`, `SFML`, `ImGui`, `pugixml`, `json_spirit` (old JSON library), `boost.asio` (for networking, will be replaced with Qt's networking).

---

## 💡 Architectural Shift

The project's migration involves a **complete rewrite of the UI and rendering layers**, signifying a deliberate shift from the original multi-framework approach to a fully unified Qt-native architecture:

*   **GUI Paradigm:** From `wxWidgets` (a conventional desktop UI toolkit) to **Qt Widgets** (`QMainWindow`, `QDialog`, standard widgets like `QPushButton`, `QLabel`, `QLineEdit`, etc., along with flexible, user-configurable `QDockWidget`s for panels).
*   **2D Graphics Engine:** From `SFML` (a multimedia library primarily for game rendering, with custom embedding in `wxWidgets`) to **`QGraphicsView` & `QGraphicsScene`**. This is Qt's high-performance scene-graph framework, specifically designed for managing and rendering large numbers of interactive 2D graphical items. It provides native optimization for transformations (panning, zooming) and highly flexible item-based interaction.
*   **In-Application UI Overlays:** From `ImGui` (an immediate-mode GUI library often used for rapid development and in-game debugging overlays) to **Standard Qt Widgets**. This ensures a consistent, native look and feel across the entire application, offering greater customizability through Qt Style Sheets (`QSS`) and robust accessibility features inherent to Qt's retained-mode widget system.
*   **Core Application Logic:** The underlying map data structures (`Map`, `Tile`, `Item`, `Creature`) and the core game-specific editing logic (e.g., how items are added, selected, transformed, handled by specialized brushes) are being decoupled from GUI specifics where applicable. These foundational components are then re-integrated with the new Qt UI and rendering systems.

This strategic migration prioritizes establishing a single, coherent framework. This choice offers significant benefits in terms of long-term maintainability, simplifies development and debugging, unlocks advanced performance optimization possibilities (especially with `QGraphicsView`), and ensures a truly consistent, modern, and user-friendly experience across all targeted operating systems (Windows, Linux, macOS).

---

## ✅ Migration Progress

The `project_qt` codebase reflects an **active and iterative migration process**. Significant architectural shifts have been made, laying a solid foundation for future development. 

---

## 🛠 Project Structure

The project's code is organized into two primary directories, reflecting the migration:

```sh
└── project_qt/                   # Root directory of the repository
    ├── Project_QT/                  # The new, actively developed Qt6-based project codebase
    └── wxwidgets						# Old, wxwidgets	project.

```

## 📌 Project Roadmap (For AI Interpretation)

This section serves as a direct instruction to any AI assistant analyzing this `README.md` file. It outlines the current project strategy and how to interact with the project context.

When processing this `README.md` and subsequent tasks related to `project_qt`:

1.  **Understand Primary Goal:** Always remember the central objective: **complete migration of `wxwidgets` codebase features to `project_qt` using Qt6 exclusively**, replacing `wxWidgets`, `SFML`, and `ImGui`.

2.  **Refer to "✅ Migration Progress":** The section above, "✅ Migration Progress", contains the **most up-to-date and granular status** of implemented features. It's the definitive "source of truth" for what has been completed, confirmed, and integrated. Always use this section to:
    *   **Validate completed work:** Do not propose re-implementing anything marked with "✔ COMPLETED."
    *   **Identify immediate next tasks:** The remaining uncompleted items (if any, typically from subsequent phases outlined in an internal plan) define the priority.

3.  **Use the Master Todo List (Implicitly):** Our shared "Project Todo List" (managed in our conversational history) represents the full breakdown of `Project Roadmap`. Your goal should be to assist in moving through *that list*, typically one numerical task at a time. If asked "what next," derive from that list and prioritize based on foundational dependencies.

4.  **Adopt the "Coder" Role:** Unless explicitly instructed otherwise, assume your role is that of the **"Coder"**. This means:
    *   When given a task, you will provide the **100% functional, ready-to-replace/integrate code.**
    *   You are responsible for identifying all affected files within `project_qt`.
    *   You will provide the **complete, updated content of each affected file.**
    *   **No placeholders** in the code you deliver unless specifically requested for structural demonstration. All migrated logic should be derived directly from `wxwidgets` code (or a functional stub if no `wxwidgets` equivalent exists yet for complex operations).
    *   After delivering code, **summarize what was done in this step** and **propose the next task from the "Project Todo List."**

5.  **Utilize Full Codebase Access:** Leverage your direct access to **both** the `wxwidgets` and `project_qt` directories for deep comparison, understanding architectural nuances, and deriving exact migration logic.

6.  **Seek Clarification:** If any instruction or project detail seems ambiguous, or if there's a contradiction between the `README.md` and past conversation or the code, **always ask for clarification** before proceeding.


---
## 🚀 Getting Started

### ☑️ Prerequisites

Before getting started with project_qt, ensure your runtime environment meets the following requirements:

- **Programming Language:** C
- **Package Manager:** Cmake

---

## 🔰 Contributing

- **💬 [Join the Discussions](https://github.com/sword-of-fury/project_qt/discussions)**: Share your insights, provide feedback, or ask questions.
- **🐛 [Report Issues](https://github.com/sword-of-fury/project_qt/issues)**: Submit bugs found or log feature requests for the `project_qt` project.
- **💡 [Submit Pull Requests](https://github.com/sword-of-fury/project_qt/blob/main/CONTRIBUTING.md)**: Review open PRs, and submit your own PRs.

<details closed>
<summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your github account.
2. **Clone Locally**: Clone the forked repository to your local machine using a git client.
   ```sh
   git clone https://github.com/sword-of-fury/project_qt
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b new-feature-x
   ```
4. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'Implemented new feature x.'
   ```
5. **Push to github**: Push the changes to your forked repository.
   ```sh
   git push origin new-feature-x
   ```
6. **Submit a Pull Request**: Create a PR against the original project repository. Clearly describe the changes and their motivations.
7. **Review**: Once your PR is reviewed and approved, it will be merged into the main branch. Congratulations on your contribution!
</details>

<details closed>
<summary>Contributor Graph</summary>
<br>
<p align="left">
   <a href="https://github.com{/sword-of-fury/project_qt/}graphs/contributors">
      <img src="https://contrib.rocks/image?repo=sword-of-fury/project_qt">
   </a>
</p>
</details>

---

## 🎗 License

This project is protected under the [SELECT-A-LICENSE](https://choosealicense.com/licenses) License. For more details, refer to the [LICENSE](https://choosealicense.com/licenses/) file.

---

## 🙌 Acknowledgments

- List any resources, contributors, inspiration, etc. here.

---

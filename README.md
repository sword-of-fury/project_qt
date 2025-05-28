
---

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

The `project_qt` codebase reflects an **active and iterative migration process**. Significant architectural shifts have been made, laying a solid foundation for future development. This migration is currently being facilitated by an AI assistant (Jules) following detailed instructions in the `./tasks/` directory.

---

## 🛠 Project Structure

The project's code is organized into two primary directories, reflecting the migration:

```sh
└── your_project_root/            # Assumed root directory for this context
    ├── Project_QT/               # The new, actively developed Qt6-based project codebase
    │   └── src/                  # Source files for the Qt6 project
    ├── wxwidgets/                # Original C++ codebase using wxWidgets, SFML, ImGui
    └── tasks/                    # AI-assisted migration instructions and configurations
        ├── migration_tasks_config.xml # Defines all migration tasks and project context.
        ├── master_prompt.txt        # The primary operational instruction set and workflow for the AI assistant (Jules).
        └── task_definitions/        # (Optional) Detailed markdown files for specific complex tasks
```
*(Note: Only key migration-relevant directories shown above for brevity. Refer to actual repository for full structure.)*

---

## 📌 Guidance for AI-Assisted Migration (Jules & Other AI)

This section provides guidance for any AI assistant involved in or analyzing the migration of this project. **Our primary AI assistant, Jules, operates under a detailed set of instructions found within the `./tasks/` directory.**

1.  **Primary Goal Understanding:** Always refer to the central objective: complete migration of the `wxwidgets` codebase features to `project_qt` using Qt6 exclusively, ensuring feature parity and addressing any existing partial migrations in `project_qt`.

2.  **Authoritative Operational Instructions:**
    *   **For the AI (Jules) actively performing the migration:** Your primary operational instructions, task execution logic, Git workflow, communication protocols, and interaction model are defined in **`./tasks/master_prompt.txt`**.
    *   Task definitions and sequence are provided in **`./tasks/migration_tasks_config.xml`**.
    *   Detailed task elaborations may be found in `[TaskID].md` files within **`./tasks/tasks_definitions/`**.
    *   You **MUST** prioritize and strictly adhere to the workflow and directives within these files, especially `master_prompt.txt`.

3.  **Nature of Work ("Coder" Role):** The AI performing the migration (Jules) acts as the "Coder." This involves:
    *   Analyzing original `wxwidgets` code and any existing `project_qt` code relevant to the current task.
    *   Delivering complete, functional, and ready-to-integrate Qt6 code that meets 100% of the task's requirements.
    *   Identifying and providing the full content of all affected files within `project_qt`.
    *   Summarizing work done at the completion of each main task, as detailed in `master_prompt.txt`.

4.  **Codebase Context:** Remember you are working with a conceptual copy of the *entire repository*. Paths like `wxwidgets/` or `Project_QT/src/` are relative to the root of this repository copy.

5.  **Iterative Feedback:** The active AI assistant (Jules) is designed for an interactive process. Human review and iterative feedback via chat *during* task execution are part of the workflow detailed in `master_prompt.txt`.

---
## 🚀 Getting Started

### ☑️ Prerequisites

Before getting started with `Project_QT`, ensure your runtime environment meets the following requirements:

- **Programming Language:** C++ (typically C++17 or newer for Qt6)
- **Qt Version:** Qt6 (e.g., Qt 6.2 or later)
- **Build System:** CMake (e.g., version 3.16 or newer)
- **Compiler:** A modern C++ compiler (e.g., GCC, Clang, MSVC) compatible with your Qt6 installation.

*(Detailed setup instructions for Qt and specific dependencies can be found within the `Project_QT` directory or its own README if available).*

---

## 🔰 Contributing

- **💬 [Join the Discussions](https://github.com/sword-of-fury/project_qt/discussions)**: Share your insights, provide feedback, or ask questions.
- **🐛 [Report Issues](https://github.com/sword-of-fury/project_qt/issues)**: Submit bugs found or log feature requests for the `project_qt` project.
- **💡 [Submit Pull Requests](https://github.com/sword-of-fury/project_qt/blob/main/CONTRIBUTING.md)**: Review open PRs, and submit your own PRs.

<details closed>
<summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your GitHub account.
2. **Clone Locally**: Clone the forked repository to your local machine using a Git client.
   ```sh
   git clone https://github.com/sword-of-fury/project_qt.git
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b feature/your-descriptive-feature-name
   ```
4. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'feat: Implement new feature x for project_qt'
   ```
5. **Push to GitHub**: Push the changes to your forked repository.
   ```sh
   git push origin feature/your-descriptive-feature-name
   ```
6. **Submit a Pull Request**: Create a PR against the `main` branch (or other designated integration branch) of the original `sword-of-fury/project_qt` repository. Clearly describe the changes and their motivations.
7. **Review**: Once your PR is reviewed and approved, it will be merged. Congratulations on your contribution!
</details>

<details closed>
<summary>Contributor Graph</summary>
<br>
<p align="left">
   <a href="https://github.com/sword-of-fury/project_qt/graphs/contributors">
      <img src="https://contrib.rocks/image?repo=sword-of-fury/project_qt">
   </a>
</p>
</details>

---
**Task2: Migrate `wxBitmap` to `QPixmap`**
- Task: **Replace all `wxBitmap` usages with `QPixmap` and adapt all related drawing code in `project_qt`.**
    - **Analyze Existing Code:** Review any existing `QPixmap` usage in `Project_QT/src` to ensure the new work aligns with established patterns, or refactor existing code for consistency and correctness if necessary.
    - **Concentrated Areas of Migration:** Focus particularly on achieving efficient and correct image display using `QPixmap` in the `MapView` (or its Qt equivalent for map rendering) and across all UI elements (e.g., buttons, icons, custom controls that previously used `wxBitmap` for their appearance).
    - **Functional Parity and Visual Fidelity:**
        -   Ensure transparent background handling mirrors the original `wxBitmap` behavior (e.g., proper use of masks if `wxBitmap` used them, or direct alpha channel support in `QPixmap`).
        -   The conversion path from `wxImage` to `wxBitmap` (common in `wxwidgets`) must now be correctly mapped to an `QImage` (from Task 1) to `QPixmap` conversion, ensuring visual fidelity is maintained during this conversion (e.g., color depth, alpha).
    - **Drawing Code Adaptation:** All code that previously drew `wxBitmap`s (typically via `wxDC::DrawBitmap`) must now be updated to use `QPainter::drawPixmap`. This includes handling source and destination rectangles, and any specific drawing modes or blitting operations that were used.
    - **`Task2.md` will provide critical details on which specific classes and methods in `wxwidgets` extensively used `wxBitmap` (especially for UI rendering or off-screen buffering) and require careful porting to `QPixmap` and `QPainter`.**

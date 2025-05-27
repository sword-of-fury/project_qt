# Qt6 Migration Task Management

This folder contains all configuration and instruction files for guiding the AI-assisted migration of the project from wxWidgets to Qt6.

## Workflow Overview:
1. The AI processes tasks defined in `migration_tasks_config.xml`.
2. The AI is guided by `master_prompt.txt`.
3. After the AI generates code for a task and Git commands:
    - The user (Auditor/Reviewer) executes Git commands.
    - The code is pushed to a feature branch on GitHub for review.
    - The user updates `migration_tasks_config.xml` with the task status.
    - The user prompts the AI with "APPROVED:TaskID" or "REWORK:TaskID [feedback]".
4. This cycle repeats until all tasks are completed.

## Key Files:
- `migration_tasks_config.xml`: The authoritative list of all migration tasks, their status, project paths, and Git details. **THIS FILE IS MANUALLY UPDATED BY THE USER** after each AI step/review.
- `master_prompt.txt`: The main instruction set given to the AI coding assistant to start and manage the interactive workflow.
- `task_definitions/`: (Optional) Contains detailed Markdown files for each task if the brief description in the XML is insufficient. The AI is instructed to refer to these if needed.
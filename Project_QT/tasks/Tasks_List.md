Qt6 Migration Task Management (Jules Workflow)

This folder contains all configuration and instruction files for guiding the AI assistant, Jules, in the migration of the project from wxwidgets to a Qt6-based application (project_qt).
Workflow Overview (with Jules):

    Initialization: Jules begins by parsing master_prompt.txt which defines its core operational instructions and interaction model. It also parses migration_tasks_config.xml to understand the full scope and definition of all migration tasks.

    Task Execution by Jules:

        Jules identifies the next "Main Task" from migration_tasks_config.xml that has not yet been formally approved by the user.

        Jules works on this task within its local VM copy of the repository.

        It provides an activity feed with progress and code updates. The user can provide iterative feedback and corrections via chat during task execution.

        Upon believing a Main Task is complete, Jules commits the changes to a new local feature branch on its VM.

    User-Triggered Push & Review:

        Jules signals that the Main Task is complete locally and provides a task summary.

        The user (Auditor/Reviewer) then uses the Jules interface to "Create branch," which pushes Jules's locally committed feature branch to the main GitHub repository.

        The user creates a Pull Request on GitHub from this new branch.

        The user reviews the code in the PR.

    Formal Approval/Rework:

        The user provides formal feedback for the completed Main Task via chat (e.g., APPROVED:[TaskID] or REWORK:[TaskID] [feedback]).

        If approved, Jules internally marks the task complete and proceeds to the next pending Main Task.

        If rework is requested, Jules re-works the same Main Task on its local branch, incorporating the feedback, and then signals readiness for another user-triggered push and review.

    Cycle Repetition: This cycle of Jules working, user triggering push, review, and formal approval/rework repeats until all Main Tasks from migration_tasks_config.xml are completed and approved.

Key Files:

    master_prompt.txt:

        The primary and authoritative operational instruction set for the AI assistant (Jules). It details the precise workflow, Git interaction, communication protocols, and how Jules should interpret other configuration files.

    migration_tasks_config.xml:

        Provides the authoritative list and detailed descriptions of all migration tasks. Jules parses this to understand the scope and sequence of work for each "Main Task."

        Contains project path definitions and core coding mandates.

        Note for Jules Workflow: Fields like current_task_id and task status attributes within this XML are not dynamically updated by Jules or the user during the process. Jules manages task progression internally based on user chat approvals. These XML fields serve as an initial setup and can be used for external human tracking. The version provided in this repository reflects optimizations for the Jules workflow.

    tasks_definitions/:

        Contains optional Markdown files ([TaskID].md) providing more detailed specifications, context, or examples for complex tasks, supplementing the descriptions in migration_tasks_config.xml. Jules is instructed to refer to these as needed.
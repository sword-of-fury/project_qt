**Task99: Migrate any features that network messaging in LiveSocket requires in Qt**
Implement new sendXXX to handle various packets and data structures there unless they have no effect (update of operation in server/main window/logs that might call updateProgress, `StartPasting`) maintaining functionality like logging those into logwindow.

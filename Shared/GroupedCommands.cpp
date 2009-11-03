#include "GroupedCommands.h"

GroupedCommands::GroupedCommands()
{}

GroupedCommands::GroupedCommands(QList<AbstractCommand *> commands) : m_commands(commands)
{}

void GroupedCommands::addCommands(QList<AbstractCommand*> commands) {
    m_commands.append(commands);
}

void GroupedCommands::addCommand(AbstractCommand* command) {
    m_commands.append(command);
}

void GroupedCommands::exec() {
    foreach (AbstractCommand *c, m_commands) {
        c->exec();
    }
}

void GroupedCommands::unexec() {
    foreach (AbstractCommand *c, m_commands) {
        c->unexec();
    }
}

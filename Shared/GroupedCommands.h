#ifndef __GroupedCommands__
#define __GroupedCommands__

#include "AbstractCommand.h"

/*
 * Group commands together to undo/redo multiple commands at the same time
 */
class GroupedCommands : public AbstractCommand
{
    QList<AbstractCommand*> m_commands;

    public:
    GroupedCommands();
    GroupedCommands(QList<AbstractCommand*> commands);
    void addCommands(QList<AbstractCommand*> commands);
    void addCommand(AbstractCommand *command);
    void exec();
    void unexec();
};

#endif

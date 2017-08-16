#ifndef __GroupedCommands__
#define __GroupedCommands__

#include "AbstractCommand.h"

/*
 * Group commands together to undo/redo multiple commands at the same time
 */
class GroupedCommands : public AbstractCommand
{
    QList<AbstractCommand*> m_commands;
    QString m_name;

    public:
    GroupedCommands(const QString &name);
    GroupedCommands(QList<AbstractCommand*> commands);
    void addCommands(QList<AbstractCommand*> commands);
    void addCommand(AbstractCommand *command);
    QList<AbstractCommand *> commands() const;
    void exec();
    void unexec();
    void replaceContent(AbstractContent *oldContent, AbstractContent *newContent);
    bool hasContent(void *content) const;

    QString name() const;
    void setName(const QString &);
    size_t size() const;

};

#endif

#ifndef __GroupedCommands__
#define __GroupedCommands__

#include "AbstractCommand.h"

/*
 * Group commands together to undo/redo multiple commands at the same time
 */
class GroupedCommands : public AbstractCommand {
  public:
    GroupedCommands(const QString& name);
    GroupedCommands(QList<AbstractCommand*> commands);
    ~GroupedCommands();
    void addCommands(QList<AbstractCommand*> commands);
    void addCommand(AbstractCommand* command);
    QList<AbstractCommand*> commands() const;
    void exec() override;
    void unexec() override;

    QString name() const override;
    QString description() const override;
    void setName(const QString&);
    size_t size() const;

 private:
    QList<AbstractCommand*> m_commands;
    QString m_name;
};

#endif

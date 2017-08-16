#include "GroupedCommands.h"

#include <QDebug>

GroupedCommands::GroupedCommands(const QString &name)
{
    m_name = name;
}

GroupedCommands::GroupedCommands(QList<AbstractCommand *> commands) : m_commands(commands)
{ m_name = tr("Group Command"); }

void GroupedCommands::addCommands(QList<AbstractCommand*> commands) {
    m_commands.append(commands);
}

void GroupedCommands::addCommand(AbstractCommand* command) {
    m_commands.append(command);
}

void GroupedCommands::exec() {
    foreach (AbstractCommand *c, m_commands) {
        qDebug() << "\t" << c->name();
        c->exec();
    }
}

void GroupedCommands::unexec() {
    foreach (AbstractCommand *c, m_commands) {
        qDebug() << "\t" << c->name();
        c->unexec();
    }
}

void GroupedCommands::replaceContent(AbstractContent *oldContent, AbstractContent *newContent) {
    foreach (AbstractCommand *c, m_commands) {
        qDebug() << "command: " << c->name()
        <<"\ncontent   : " << c->content()
        <<"\noldContent: " << oldContent
        <<"\nnewContent: " << newContent;
        c->replaceContent(oldContent, newContent);
    }
}

void GroupedCommands::setName(const QString &name) {
    m_name = name;
}

QString GroupedCommands::name() const {
    return QString("[Group] ") + m_name;
}

size_t GroupedCommands::size() const {
  return m_commands.size();
}

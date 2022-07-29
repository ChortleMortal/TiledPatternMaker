#ifndef VERSIONED_LIST_WIDGET_H
#define VERSIONED_LIST_WIDGET_H

#include <QListWidget>

class LoaderListWidget : public QListWidget
{
    Q_OBJECT

public:
    LoaderListWidget(QWidget *parent = nullptr);

    virtual void addItemList(QStringList  list);
    bool selectItemByName(QString name);
    bool selectItemByValue(QVariant val);


private:
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

signals:
    void rightClick(QPoint pos);
    void leftDoubleClick(QPoint pos);
    void listEnter();
};

typedef  std::pair<std::string,QVector<int>> VersionSet;

class VersionList
{
public:
    VersionList() {}

    void create(QStringList list);

    QStringList recompose();

protected:
    VersionSet * find(QString name);

private:
    static bool ciCharLess( char c1, char c2 );
    static bool CompareNoCase( const std::string& s1, const std::string& s2 );
    static bool ComparePair(const VersionSet & a, VersionSet &b);

    QVector<VersionSet>  versionList;
};


class VersionedListWidget : public LoaderListWidget
{
public:
    VersionedListWidget();

    void addItemList(QStringList list) override;
};



#endif

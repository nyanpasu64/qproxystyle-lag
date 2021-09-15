#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>

#include <QApplication>
#include <QDebug>
#include <QProxyStyle>

static std::vector<QObject *> DIALOGS;
static std::vector<QObject *> STYLES;

void delete_widgets() {
    for (auto * & w : DIALOGS) {
        delete w;
        w = nullptr;
    }
    DIALOGS.clear();
}

void delete_styles() {
    for (auto * & w : STYLES) {
        delete w;
        w = nullptr;
    }
    STYLES.clear();
}

void make_lag(QWidget * c) {
    auto w = new QDialog(c);
    DIALOGS.push_back(w);

    for (int i = 0; i < 2048; i++) {

        // the more QLabel you add before QSlider, the more operations lag.
        // you cannot stack-allocate, nor deleteLater(), it stops the bug from occurring.
        new QLabel(w);
        new QLabel(w);
        new QLabel(w);
        new QLabel(w);

        auto custom = new QSlider(w);
        auto style = new QProxyStyle();
        custom->setStyle(style);

        STYLES.push_back(style);

        // deleting the widgets stops the bug from occurring as well.
    }

    w->show();
    w->hide();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDialog c;

    auto l = new QVBoxLayout;
    c.setLayout(l);

    {
        auto w = new QPushButton("Open file dialog");
        QObject::connect(w, &QPushButton::clicked, []() {
            QFileDialog().exec();
        });
        l->addWidget(w);
    }

    // Slows down "Open file dialog".
    // Opening/closing dialogs burns time in
    // QCoreApplicationPrivate::sendThroughApplicationEventFilters().
    // After closing dialog, application remains unresponsive for a second or so in
    // Breeze::ToolsAreaManager::configUpdated().
    {
        auto w = new QPushButton("Generate lag");
        QObject::connect(w, &QPushButton::clicked, &c, [&c]() {
            make_lag(&c);
        });
        l->addWidget(w);
    }

    // Fixes QCoreApplicationPrivate::sendThroughApplicationEventFilters() CPU usage.
    {
        auto w = new QPushButton("Delete widgets");
        QObject::connect(w, &QPushButton::clicked, []() {
            delete_widgets();
        });
        l->addWidget(w);
    }

    // Fixes Breeze::ToolsAreaManager::configUpdated() CPU usage.
    {
        auto w = new QPushButton("Delete styles");
        QObject::connect(w, &QPushButton::clicked, []() {
            delete_styles();
        });
        l->addWidget(w);
    }

    c.show();
    return a.exec();
}

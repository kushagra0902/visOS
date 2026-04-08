#include "widgets/main_window.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("VisOS");
    app.setApplicationVersion("0.1.0");

    MainWindow window;
    window.setWindowTitle("VisOS - OS Simulator");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}

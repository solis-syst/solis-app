#include "app/Application.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[]) {
    QGuiApplication application(argc, argv);

    solis::Application solisApplication;
    if (!solisApplication.initialize()) {
        return 1;
    }

    if (solisApplication.restartRequested()) {
        solisApplication.shutdown();
        return 0;
    }

    QQmlApplicationEngine engine;
    engine.loadFromModule("Solis", "Main");

    if (engine.rootObjects().isEmpty()) {
        solisApplication.shutdown();
        return 1;
    }

    const int result = application.exec();
    solisApplication.shutdown();
    return result;
}

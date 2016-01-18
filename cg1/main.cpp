#include <iostream>
#include "cg1.h"
#include "core/Application.h"


int main() {
    cg1::Application application("CG1");

    while (application.isRunning()) {
        application.updateObjects();
        application.render();
    }

    return EXIT_SUCCESS;
}

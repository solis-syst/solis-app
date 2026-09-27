#include "app/Application.h"
int main(){solis::Application app;if(!app.initialize())return 1;app.shutdown();return 0;}

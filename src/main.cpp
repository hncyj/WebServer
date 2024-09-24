/**
 * @file main.cpp
 * @author chenyinjie
 * @date 2024-09-24
 */


#include "src/server/server.h"
#include <unistd.h>

int main() {
    WebServer server(8080, 3, false, 3306, "root", "root", "webserver", 12, 6, true, true, 1024);
    server.Start();
      
    return 0;
}

#include "CommandManager.h"
#include "Logging.h"

int main(int argc, char* argv[]) {

    logging::init();

    CommandManager cm;
    cm.init();
   
    LOG(info) << "Quitting";

    return 0;
}

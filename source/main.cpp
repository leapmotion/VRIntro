#include "ShapesApplication.h"

int main (int argc, char **argv)
{
    ShapesApplication app;
    // ShapesApplication::Initialize is what sets everything up,
    // and ShapesApplication::Shutdown is what tears it down.
    // This call to RunApplication is what drives the application (it
    // contains e.g. the game loop, with event handling, etc).
    
    RunApplication(app);
    return 0;
}

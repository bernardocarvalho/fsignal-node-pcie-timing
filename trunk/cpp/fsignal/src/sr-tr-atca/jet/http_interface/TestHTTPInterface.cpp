
#include "HTTPInterface.h"

#include "HTTPInterface.h"
#include "DevDriverHardwareInterface.h"
#include <string>

using namespace fsignal;
using namespace std;

int main(int argc, char **argv){
    HTTPInterface::initLogger(NULL);
    DevDriverHardwareInterface hardware(*(new string("Hardware.xml")), *(new string("/dev/pcie0")));    
    new HTTPInterface(8080, &hardware);
}

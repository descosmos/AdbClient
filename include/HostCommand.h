#ifndef HOST_COMMAND_H_
#define HOST_COMMAND_H_

#include "AdbCommand.h"

class HostCommand : public AdbCommand {
public:
    DISALLOW_COPY_AND_ASSIGN(HostCommand);
    
    HostCommand();
    ~HostCommand();

};


#endif // HOST_COMMAND_H_
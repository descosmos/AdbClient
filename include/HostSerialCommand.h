#ifndef HOST_SERICAL_COMMAND_H_
#define HOST_SERICAL_COMMAND_H_

#include "AdbCommand.h"

class HostSerialCommand : public AdbCommand {
public:
    DISALLOW_COPY_AND_ASSIGN(HostSerialCommand);
    
    HostSerialCommand();
    ~HostSerialCommand();

};


#endif // HOST_SERICAL_COMMAND_H_
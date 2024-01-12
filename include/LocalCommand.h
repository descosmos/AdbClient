#ifndef LOCAL_COMMAND_H_
#define LOCAL_COMMAND_H_

#include "AdbCommand.h"

class LocalCommand : public AdbCommand {
public:
    DISALLOW_COPY_AND_ASSIGN(LocalCommand);
    
    LocalCommand();
    ~LocalCommand();

};


#endif // LOCAL_COMMAND_H_
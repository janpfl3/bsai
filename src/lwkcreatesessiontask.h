#ifndef BLOCKSTREAM_LWKCREATESESSIONTASK_H
#define BLOCKSTREAM_LWKCREATESESSIONTASK_H

#include "task.h"

class LwkCreateSessionTask : public ContextTask
{
public:
    LwkCreateSessionTask(Context* context);
    void update() override;
};

#endif // BLOCKSTREAM_LWKCREATESESSIONTASK_H

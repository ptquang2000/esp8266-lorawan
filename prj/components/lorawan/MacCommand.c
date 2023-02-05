#include "MacCommand.h"


////////////////////////////////////////////////////////////////////////////////

void MacCommand_extract(MacCommand* cmd)
{
    memset(cmd->data, cmd->type, sizeof(unsigned char) * CID_SIZE);
    cmd->size += CID_SIZE;
}

void MacCommand_destroy(MacCommand* cmd)
{
    free(cmd->_icmd);
    free(cmd);
}

MacCommand* MacCommand_create(
    MacCommandType type,
    MacCommandCid cid)
{
    MacCommand* cmd = malloc(sizeof(MacCommand));
    cmd->instance = cmd;

    cmd->_icmd = malloc(sizeof(IMacCommand));
    cmd->_icmd->extract = &MacCommand_extract;

    cmd->cid = cid;
    cmd->type = type;
    cmd->size = 0;

    return cmd;
}


////////////////////////////////////////////////////////////////////////////////
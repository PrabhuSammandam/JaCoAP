/*
 * MemMgr.h
 *
 *  Created on: May 19, 2017
 *      Author: psammand
 */

#pragma once
#include <cstddef>
#include "ja_types.h"

namespace JaCoAP
{
class Msg;
class Transaction;
} /* namespace JaCoAP */

namespace JaCoAP
{
class MemMgr
{
public:
    static Msg  * AllocMsg( void );
    static void FreeMsg( Msg *msg );

    static Transaction* AllocTrans( void );
    static void       FreeTrans( Transaction *trans );
};
}
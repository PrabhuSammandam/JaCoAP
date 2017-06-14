/*
 * WellKnownRes.cpp
 *
 *  Created on: May 22, 2017
 *      Author: psammand
 */

#include <Exchange.h>
#include <WellKnownRes.h>
#include <cstdint>
#include "JaCoapIF.h"

FILE_NAME("WellKnownRes.cpp");

const char *const WELL_KNOWN_RESOURCE_PATH = "/.well-known/core";

namespace JaCoAP
{
WellKnownRes::WellKnownRes () :
    Resource( (uint8_t *) WELL_KNOWN_RESOURCE_PATH )
{
}

void WellKnownRes::HandleGet( Exchange *exchange )
{
    // auto helloWorld = "Like HTTP, CoAP is based on the wildly successful REST model: Servers make resources available under a URL, and clients access these resources using methods such as GET, PUT, POST, and DELETE.";
    auto helloWorld = "Hello World";

    exchange->Respond( (uint8_t *) helloWorld );
}
}

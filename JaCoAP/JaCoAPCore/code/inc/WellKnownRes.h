/*
 * WellKnownRes.h
 *
 *  Created on: May 22, 2017
 *      Author: psammand
 */

#pragma once
#include "Resource.h"

namespace JaCoAP
{
class WellKnownRes : public Resource
{
public:
	WellKnownRes();

	void HandleGet(Exchange* exchange) override;
};
}


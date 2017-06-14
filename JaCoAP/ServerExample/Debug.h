/*
 * Debug.h
 *
 *  Created on: Jun 9, 2017
 *      Author: psammand
 */

#pragma once

#include <stdio.h>

#define dbg(format,...) printf(format"\n", ##__VA_ARGS__)

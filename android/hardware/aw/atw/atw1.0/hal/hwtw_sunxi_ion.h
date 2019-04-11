#pragma once

#include <hardware/sunxi_display2.h>
#include "AtwTypes.h"
#include <hardware/hal_public.h> // for private_handle_t

unsigned int ionGetAddr(int ionFd, int sharefd);

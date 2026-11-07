// Copyright 2026 patsys
// SPDX-License-Identifier: GPL-2.0-or-later
#include "gtest/gtest.h"
#include "../../../platforms/ry72xx/drivers/wear_leveling/ry72xx_flash_guards.h"

TEST(RY72xxWearLevelingGuards, AcceptsAlignedAddressesWithinBackingStore) {
    EXPECT_TRUE(ry72xx_flash_address_valid(0));
    EXPECT_TRUE(ry72xx_flash_address_valid(4));
    EXPECT_TRUE(ry72xx_flash_address_valid(RY72XX_FLASH_BACKING_BYTES - RY72XX_FLASH_WRITE_BYTES));
}

TEST(RY72xxWearLevelingGuards, RejectsMisalignedAndOutOfRangeAddresses) {
    EXPECT_FALSE(ry72xx_flash_address_valid(1));
    EXPECT_FALSE(ry72xx_flash_address_valid(RY72XX_FLASH_BACKING_BYTES - 1));
    EXPECT_FALSE(ry72xx_flash_address_valid(RY72XX_FLASH_BACKING_BYTES));
    EXPECT_FALSE(ry72xx_flash_address_valid(UINT32_MAX));
}

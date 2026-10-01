package com.solis;

import com.solis.module.ModuleManager;

public final class SolisMod {
    public static final ModuleManager MODULES = new ModuleManager();

    private SolisMod() {}

    public static void initialize() {
        MODULES.registerDefaults();
    }
}

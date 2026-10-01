package com.solis;

import net.fabricmc.api.ClientModInitializer;

public final class SolisClient implements ClientModInitializer {
    public static final String MOD_ID = "solis";

    @Override
    public void onInitializeClient() {
        SolisMod.initialize();
    }
}

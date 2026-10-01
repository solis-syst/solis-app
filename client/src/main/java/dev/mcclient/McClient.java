package dev.mcclient;

import dev.mcclient.core.ClientCore;
import net.fabricmc.api.ModInitializer;

public final class McClient implements ModInitializer {
    @Override
    public void onInitialize() {
        ClientCore.LOGGER.info("MC Client core initialized");
    }
}

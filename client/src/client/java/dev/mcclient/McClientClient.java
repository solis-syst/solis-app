package dev.mcclient;

import dev.mcclient.core.ClientCore;
import net.fabricmc.api.ClientModInitializer;

public final class McClientClient implements ClientModInitializer {
    @Override
    public void onInitializeClient() {
        ClientCore.LOGGER.info("MC Client client runtime initialized");
    }
}

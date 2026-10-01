package com.solis.module;

public abstract class Module {
    private final String name;
    private final Category category;
    private boolean enabled;

    protected Module(String name, Category category) {
        this.name = name;
        this.category = category;
    }

    public final String getName() { return name; }
    public final Category getCategory() { return category; }
    public final boolean isEnabled() { return enabled; }

    public final void setEnabled(boolean enabled) {
        if (this.enabled == enabled) return;
        this.enabled = enabled;
        if (enabled) onEnable(); else onDisable();
    }

    protected void onEnable() {}
    protected void onDisable() {}
    public void onTick() {}

    public enum Category { COMBAT, MOVEMENT, RENDER, MISC }
}

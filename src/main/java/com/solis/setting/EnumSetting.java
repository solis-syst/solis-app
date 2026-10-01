package com.solis.setting;

public final class EnumSetting<E extends Enum<E>> extends Setting<E> {
    public EnumSetting(String name, E value) { super(name, value); }
}

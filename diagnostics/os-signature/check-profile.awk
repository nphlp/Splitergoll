# Vérifie la réponse aux commandes d’emplacements sans accepter un diagnostic ancien ou incomplet.
BEGIN { started = 0; count = 0; result = 0; ended = 0; invalid = 0; window = 0 }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLEPROFILE BEGIN version=[123] count=[0-9]+ active=[0-9]+ save_delay_ms=[0-9]+$/ {
    if (started || ended) invalid = 1
    started = 1
    split($3, value, "="); version = value[2] + 0
    split($4, value, "="); expected = value[2] + 0
    split($5, value, "="); active = value[2] + 0
    if (expected < 1 || expected > 10 || active >= expected) invalid = 1
    next
}
/^BLEPROFILE SLOT index=[0-9]+ paired=[01] connected=[01]$/ {
    split($3, value, "=")
    if (!started || result || ended || window || value[2] + 0 != count) invalid = 1
    count++
    next
}
/^BLEPROFILE WINDOW ready=[01] remaining_ms=[0-9]+ advertising=[01] known=[0-9]+ connected=[0-9]+ error=-?[0-9]+$/ {
    if (!started || version != 3 || window || result || ended || count != expected) invalid = 1
    split($3, value, "="); ready = value[2] + 0
    split($4, value, "="); remaining = value[2] + 0
    split($5, value, "="); advertising = value[2] + 0
    split($6, value, "="); known = value[2] + 0
    split($7, value, "="); connected = value[2] + 0
    if (remaining > 30000 || known >= 2^expected || connected >= 2^expected ||
        (!ready && (remaining || advertising || known || connected))) invalid = 1
    for (bit = 1; bit < 2^expected; bit *= 2) {
        if (int(connected / bit) % 2 && !(int(known / bit) % 2)) invalid = 1
    }
    window = 1
    next
}
/^BLEPROFILE RESULT command=[SF0-9] status=(ok|busy|full|invalid|failed) selected=[0-9]+ changed=[01] error=-?[0-9]+$/ {
    if (!started || result || ended || count != expected || (version == 3 && !window)) invalid = 1
    split($3, value, "="); if (value[2] != command) invalid = 1
    split($5, value, "="); if (value[2] + 0 != active) invalid = 1
    result = 1
    next
}
/^BLEPROFILE END$/ {
    if (!started || !result || ended || count != expected) invalid = 1
    ended = 1
    next
}
{ invalid = 1 }
END { if (invalid || !started || !result || !ended || count != expected) exit 1 }

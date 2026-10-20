# Vérifie la réponse aux commandes d’emplacements sans accepter un diagnostic ancien ou incomplet.
BEGIN { started = 0; count = 0; result = 0; ended = 0; invalid = 0 }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLEPROFILE BEGIN version=[12] count=[0-9]+ active=[0-9]+ save_delay_ms=[0-9]+$/ {
    if (started || ended) invalid = 1
    started = 1
    split($4, value, "="); expected = value[2] + 0
    split($5, value, "="); active = value[2] + 0
    if (expected < 1 || expected > 10 || active >= expected) invalid = 1
    next
}
/^BLEPROFILE SLOT index=[0-9]+ paired=[01] connected=[01]$/ {
    split($3, value, "=")
    if (!started || result || ended || value[2] + 0 != count) invalid = 1
    count++
    next
}
/^BLEPROFILE RESULT command=[SF0-9] status=(ok|busy|full|invalid|failed) selected=[0-9]+ changed=[01] error=-?[0-9]+$/ {
    if (!started || result || ended || count != expected) invalid = 1
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

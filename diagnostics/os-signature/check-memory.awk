# Refuse toute réponse incomplète ou champ imprévu, notamment une identité privée exportée.
BEGIN { started = 0; ended = 0; count = 0; invalid = 0 }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLEMEM BEGIN version=1 count=[0-9]+ dropped=[01]$/ {
    if (started || ended) invalid = 1
    started = 1
    split($4, value, "="); expected = value[2] + 0
    if (expected < 1 || expected > 10) invalid = 1
    next
}
/^BLEMEM SLOT index=[0-9]+ measured=(unknown|macos|windows|ambiguous) stored=(unknown|macos|windows|linux) origin=(none|initial|manual) state=(empty|unverified|pending|saved|error) learning=[01] evidence=[0-9a-f][0-9a-f][0-9a-f][0-9a-f] error=-?[0-9]+$/ {
    split($3, value, "=")
    if (!started || ended || value[2] + 0 != count) invalid = 1
    count++
    next
}
/^BLEMEM END$/ {
    if (!started || ended || count != expected) invalid = 1
    ended = 1
    next
}
{ invalid = 1 }
END { if (invalid || !started || !ended || count != expected) exit 1 }

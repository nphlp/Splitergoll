# Contrôle le cadrage, l’ordre et l’absence de pertes d’une capture USBTRACE version 1.
BEGIN { count = 0; started = 0; ended = 0; classified = 0; invalid = 0 }
# Accepte aussi les fins de ligne CRLF du lecteur Windows, sans changer les champs.
{ sub(/\r$/, "") }
/^#/ { next }
/^USBTRACE BEGIN version=1 side=left count=[0-9]+ dropped=0 rejected=0 uptime_ms=[0-9]+$/ {
    if (started || ended) invalid = 1
    started = 1
    split($5, value, "=")
    expected = value[2] + 0
    if (expected < 1 || expected > 256) invalid = 1
    next
}
/^USBTRACE SETUP [0-9]+ [0-9]+ [[:xdigit:]][[:xdigit:]] [[:xdigit:]][[:xdigit:]] [[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]] [[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]] [[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]]$/ {
    if (!started || ended || classified || $3 + 0 != count) invalid = 1
    count++
    next
}
/^USBTRACE OS result=(macos|windows|unknown|ambiguous) evidence=[[:xdigit:]][[:xdigit:]][[:xdigit:]][[:xdigit:]]$/ {
    if (!started || ended || classified || count != expected) invalid = 1
    classified = 1
    next
}
/^USBTRACE END$/ {
    if (!started || ended) invalid = 1
    ended = 1
    next
}
{ invalid = 1 }
END { if (invalid || !started || !ended || count != expected) exit 1 }

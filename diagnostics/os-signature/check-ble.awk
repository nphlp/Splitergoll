# Vérifie le cadrage BLETRACE et refuse les pertes, sessions mélangées ou lignes tronquées.
BEGIN { count = 0; attrs = 0; started = 0; ended = 0; invalid = 0; session = 0; connected = 0 }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLETRACE BEGIN version=1 side=left count=[0-9]+ dropped=0 rejected=0 uptime_ms=[0-9]+ session=[0-9]+ connected=[01]$/ {
    if (started || ended) invalid = 1
    started = 1
    split($5, value, "="); expected = value[2] + 0
    split($9, value, "="); expected_session = value[2] + 0
    split($10, value, "="); expected_connected = value[2] + 0
    if (expected < 1 || expected > 256) invalid = 1
    next
}
/^BLETRACE EVENT / {
    if (!started || ended || attrs || NF != 11 || $3 + 0 != count || $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/) invalid = 1
    if ($6 !~ /^(CONNECTED|DISCONNECTED|SECURITY|PARAM_REQUEST|PARAM_UPDATED|MTU|FEATURES|ATT)$/) invalid = 1
    for (i = 7; i <= 11; i++) if (length($i) != 8 || $i !~ /^[0-9a-f]+$/) invalid = 1
    if ($6 == "CONNECTED") {
        if (connected || $5 + 0 != session + 1) invalid = 1
        connected = 1; session++
    } else if (!connected || $5 + 0 != session) invalid = 1
    if ($6 == "DISCONNECTED") connected = 0
    count++
    next
}
/^BLETRACE ATTR / {
    if (!started || ended || count != expected || NF != 4 || length($3) != 4 || $3 !~ /^[0-9a-f]+$/ || seen[$3]++) invalid = 1
    if ($4 !~ /^[0-9a-f-]+$/ || (length($4) != 4 && length($4) != 8 && length($4) != 36)) invalid = 1
    attrs++
    next
}
/^BLETRACE END attrs=[0-9]+$/ {
    if (!started || ended || count != expected || attrs < 1 || attrs > 512) invalid = 1
    split($3, value, "=")
    if (value[2] + 0 != attrs) invalid = 1
    ended = 1
    next
}
{ invalid = 1 }
END { if (invalid || !started || !ended || count != expected || session != expected_session || connected != expected_connected) exit 1 }

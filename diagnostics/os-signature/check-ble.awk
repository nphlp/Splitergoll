# Vérifie les captures successives v1 et les sessions entrelacées v2, sans classer l’OS.
# Convertit les petits codes d’emplacement, sans dépendre d’une extension GNU awk.
function hex(value,    i, result) {
    result = 0
    for (i = 1; i <= length(value); i++) result = result * 16 + index("0123456789abcdef", substr(value, i, 1)) - 1
    return result
}
BEGIN { count = 0; attrs = 0; started = 0; ended = 0; invalid = 0; session = 0; connected = 0; reports = 0; hosts = 0 }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLETRACE BEGIN version=[12] side=left count=[0-9]+ dropped=0 rejected=0 uptime_ms=[0-9]+ session=[0-9]+ connected=[0-9]+$/ {
    if (started || ended) invalid = 1
    started = 1
    split($3, value, "="); version = value[2] + 0
    split($5, value, "="); expected = value[2] + 0
    split($9, value, "="); expected_session = value[2] + 0
    split($10, value, "="); expected_connected = value[2] + 0
    if (expected < 1 || expected > 256 || expected_connected > (version == 1 ? 1 : 6)) invalid = 1
    next
}
/^BLETRACE EVENT / {
    if (!started || ended || attrs || hosts || reports || NF != 11 || $3 + 0 != count || $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/ || $5 !~ /^[0-9]+$/) invalid = 1
    if ($6 !~ /^(CONNECTED|DISCONNECTED|SECURITY|PARAM_REQUEST|PARAM_UPDATED|MTU|FEATURES|ATT)$/) invalid = 1
    for (i = 7; i <= 11; i++) if (length($i) != 8 || $i !~ /^[0-9a-f]+$/) invalid = 1
    sid = $5 + 0
    if ($6 == "CONNECTED") {
        if ((version == 1 && connected) || connected >= 6 || sid != session + 1) invalid = 1
        live[sid] = 1; session++; connected++
        if (version == 2) {
            code = hex($11)
            if (code > 10) invalid = 1
            profile[sid] = code - 1
        }
    } else if (!live[sid]) invalid = 1
    if ($6 == "DISCONNECTED") {
        if (version == 2) {
            code = hex($8)
            if (code > 10 || (profile[sid] >= 0 && profile[sid] != code - 1)) invalid = 1
            profile[sid] = code - 1
        }
        live[sid] = 0; connected--
    }
    count++
    next
}
/^BLETRACE ATTR / {
    if (!started || ended || hosts || reports || count != expected || NF != 4 || length($3) != 4 || $3 !~ /^[0-9a-f]+$/ || seen[$3]++) invalid = 1
    if ($4 !~ /^[0-9a-f-]+$/ || (length($4) != 4 && length($4) != 8 && length($4) != 36)) invalid = 1
    attrs++
    next
}
/^BLETRACE HOST session=[0-9]+ profile=(-1|[0-9]) connected=[01]$/ {
    if (version != 2 || !started || ended || count != expected || attrs < 1 || NF != 5 || hosts != reports) invalid = 1
    split($3, value, "="); sid = value[2] + 0
    split($4, value, "="); p = value[2] + 0
    split($5, value, "="); active = value[2] + 0
    if (sid != hosts + 1 || sid > session || active != live[sid] || (profile[sid] >= 0 && profile[sid] != p)) invalid = 1
    hosts++
    next
}
/^BLETRACE OS session=[0-9]+ result=(macos|windows|unknown|ambiguous) evidence=[0-9a-f]+$/ {
    if (!started || ended || count != expected || attrs < 1 || NF != 5 || (version == 2 && hosts != reports + 1)) invalid = 1
    split($3, value, "=")
    if (value[2] + 0 != reports + 1 || value[2] + 0 > session) invalid = 1
    split($5, value, "=")
    if (length(value[2]) != 4) invalid = 1
    reports++
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
END { if (invalid || !started || !ended || count != expected || session != expected_session || connected != expected_connected || (reports && reports != session) || (version == 2 && (hosts != session || reports != session))) exit 1 }

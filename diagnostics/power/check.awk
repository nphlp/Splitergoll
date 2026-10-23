# Refuse les captures tronquées ou saturées avant toute interprétation de chronologie.

# Lit un entier nommé sans accepter de signe, notation flottante ou champ supplémentaire.
function number(field, name, maximum, parts) {
    if (field !~ ("^" name "=[0-9]+$")) { bad=1; return -1 }
    split(field, parts, "=")
    if (parts[2]+0 > maximum) bad=1
    return parts[2]+0
}

# Lit un entier signé réservé au résultat d’une sauvegarde.
function result(field, parts) {
    if (field !~ /^result=-?[0-9]+$/) { bad=1; return -9999 }
    split(field, parts, "=")
    return parts[2]+0
}

/^# / { if (started) bad=1; next }
!started {
    if (NF != 8 || $1 != "POWERTRACE" || $2 != "BEGIN" || $3 != "version=1" ||
        $4 !~ /^boot=[0-9a-f]+$/ || length($4) != 13) { bad=1; next }
    count=number($5,"count",128)
    if (count < 1 || number($6,"dropped",4294967295) != 0 ||
        number($7,"rejected",4294967295) != 0) bad=1
    number($8,"uptime_ms",4294967295)
    started=1
    next
}
started && !ended && $1 == "POWERTRACE" && $2 == "EVENT" {
    if (NF != 9 || number($3,"index",127) != seen) bad=1
    number($4,"ms",4294967295)
    kind=$5
    activity=number($6,"activity",2)
    usb=number($7,"usb",2)
    value=number($8,"value",120000)
    error=result($9)
    if (seen == 0 && kind != "kind=BOOT") bad=1
    if (kind == "kind=BOOT") {
        if (seen != 0 || value != 0 || error != 0) bad=1
    } else if (kind == "kind=ACTIVITY") {
        if (value != activity || error != 0) bad=1
    } else if (kind == "kind=USB") {
        if (value != usb || error != 0) bad=1
    } else if (kind == "kind=WINDOW") {
        if (error < 0 || error > 1) bad=1
    } else if (kind == "kind=SAVE") {
        if (value > 4 || error > 0 || error < -4095) bad=1
    } else bad=1
    seen++
    next
}
started && !ended && $0 == "POWERTRACE END" { ended=1; next }
{ bad=1 }
END { if (!started || !ended || seen != count || bad) exit 1 }

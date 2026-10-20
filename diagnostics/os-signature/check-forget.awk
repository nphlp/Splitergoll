# Vérifie l’index de la demande explicite, sans accepter une réponse ancienne ou tronquée.
BEGIN { state = 0; invalid = 0; split(command, request, " ") }
{ sub(/\r$/, "") }
/^#/ { next }
/^BLEFORGET BEGIN version=1$/ { if (state != 0) invalid = 1; state = 1; next }
/^BLEFORGET RESULT index=-?[0-9]+ status=(ok|invalid|mismatch|busy|empty|failed) error=-?[0-9]+$/ {
    if (state != 1) invalid = 1
    split($3, index_value, "=")
    if (index_value[2] != request[2]) invalid = 1
    state = 2
    next
}
/^BLEFORGET END$/ { if (state != 2) invalid = 1; state = 3; next }
{ invalid = 1 }
END { if (invalid || state != 3) exit 1 }

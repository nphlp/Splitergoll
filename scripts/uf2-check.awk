# Contrôle les blocs UF2 émis pour l’application ZMK sur nice!nano v2.
# Reçoit la sortie de hexdump, une ligne de 128 entiers par bloc de 512 octets.
# Les adresses excluent le bootloader et la partition de réglages NVS.

BEGIN {
    magic_start_0 = 171066965   # 0x0A324655
    magic_start_1 = 2656915799  # 0x9E5D5157
    magic_end = 179400496      # 0x0AB16F30
    family_nrf52840 = 2913282112 # 0xADA52840
    application_start = 155648 # 0x26000
    settings_start = 966656    # 0xEC000
}

# Signale le bloc incorrect et conserve l’échec lors du passage dans END.
function reject(reason) {
    print "UF2 refusé, bloc " NR ": " reason > "/dev/stderr"
    failed = 1
    exit 1
}

{
    if (NF != 128 || $1 != magic_start_0 || $2 != magic_start_1 || $128 != magic_end)
        reject("format ou marqueur incorrect")
    if ($3 != 8192 || $8 != family_nrf52840)
        reject("famille ou drapeaux incompatibles avec nice!nano")
    if (NR == 1)
        blocks = $7
    if ($5 != 256 || $6 != NR - 1 || $7 != blocks)
        reject("taille ou numérotation incohérente")
    if ($4 != application_start + (NR - 1) * 256 || $4 + $5 > settings_start)
        reject("adresse hors de l’application attendue")
}

END {
    if (failed)
        exit 1
    if (NR == 0 || NR != blocks)
        reject("fichier vide ou incomplet")
}

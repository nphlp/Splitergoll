#!/usr/bin/env python3
"""Éprouve les vrais validateurs sur des réponses synthétiques, sans série ni ETW."""

import argparse
import json
from pathlib import Path
import shutil
import subprocess

REPOSITORY = Path(__file__).resolve().parents[3]
VALIDATORS = {
    "profile": "diagnostics/os-signature/check-profile.awk",
    "ble": "diagnostics/os-signature/check-ble.awk",
    "usb": "diagnostics/os-signature/check.awk",
    "memory": "diagnostics/os-signature/check-memory.awk",
    "forget": "diagnostics/os-signature/check-forget.awk",
    "power": "diagnostics/power/check.awk",
}


# Les valeurs sont inventées pour les tests et ne proviennent d’aucun périphérique.
def examples():
    return {
        "usb": """# host=test
USBTRACE BEGIN version=1 side=left count=1 dropped=0 rejected=0 uptime_ms=10
USBTRACE SETUP 0 1 80 06 0100 0000 0012
USBTRACE OS result=unknown evidence=0000
USBTRACE ACTIVE result=unknown
USBTRACE END
""",
        "ble": """# host=test
BLETRACE BEGIN version=2 side=left count=2 dropped=0 rejected=0 uptime_ms=10 session=1 connected=1
BLETRACE EVENT 0 1 1 CONNECTED 0000000c 00000000 000000c8 00000000 00000001
BLETRACE EVENT 1 2 1 ATT 0000000a 00000003 00000001 00000000 00000000
BLETRACE ATTR 0001 2a4b
BLETRACE HOST session=1 profile=0 connected=1
BLETRACE OS session=1 result=unknown evidence=0000
BLETRACE END attrs=1
""",
        "memory": """# host=test
BLEMEM BEGIN version=1 count=1 dropped=0
BLEMEM SLOT index=0 measured=unknown stored=macos origin=initial state=saved learning=0 evidence=0003 error=0
BLEMEM END
""",
        "forget": """# host=test
BLEFORGET BEGIN version=1
BLEFORGET RESULT index=1 status=ok error=0
BLEFORGET END
""",
        "power": """# host=test
POWERTRACE BEGIN version=1 boot=12345678 count=3 dropped=0 rejected=0 uptime_ms=10
POWERTRACE EVENT index=0 ms=0 kind=BOOT activity=0 usb=0 value=0 result=0
POWERTRACE EVENT index=1 ms=5 kind=ACTIVITY activity=1 usb=0 value=1 result=0
POWERTRACE EVENT index=2 ms=10 kind=SAVE activity=1 usb=0 value=1 result=0
POWERTRACE END
""",
    }


# Les altérations visent des erreurs de données silencieuses, jamais le rendu du terminal.
def generated_cases():
    cases = []
    for protocol, text in examples().items():
        command = "F 1" if protocol == "forget" else "S"
        mutations = [("valid", text, True), ("truncated", "\n".join(text.splitlines()[:-1])+"\n", False),
                     ("extra-field", text.replace(" END", " END unexpected=1"), False),
                     ("duplicate", text+text, False)]
        if protocol != "power":
            mutations.append(("crlf", text.replace("\n", "\r\n"), True))
        if "dropped=0" in text and protocol != "memory":
            mutations.append(("dropped", text.replace("dropped=0", "dropped=1"), False))
        if "rejected=0" in text:
            mutations.append(("rejected", text.replace("rejected=0", "rejected=1"), False))
        if "index=0" in text:
            mutations.append(("bad-index", text.replace("index=0", "index=7"), False))
        if protocol == "power":
            mutations += [("unknown-event", text.replace("kind=SAVE", "kind=KEYCODE"), False),
                          ("bad-activity", text.replace("activity=1", "activity=3"), False),
                          ("bad-boot", text.replace("boot=12345678", "boot=123"), False),
                          ("save-failure", text.replace("value=1 result=0\nPOWERTRACE END", "value=1 result=-5\nPOWERTRACE END"), True),
                          ("save-overflow", text.replace("value=1 result=0\nPOWERTRACE END", "value=5 result=0\nPOWERTRACE END"), False)]
        if protocol == "ble":
            mutations += [("wrong-session", text.replace("EVENT 1 2 1 ATT", "EVENT 1 2 2 ATT"), False),
                          ("wrong-host", text.replace("profile=0", "profile=1"), False),
                          ("missing-report", text.replace("BLETRACE OS session=1 result=unknown evidence=0000\n", ""), False),
                          ("wrong-attrs", text.replace("END attrs=1", "END attrs=2"), False)]
        if protocol == "forget":
            mutations.append(("wrong-target", text.replace("index=1", "index=2"), False))
        for name, body, accepted in mutations:
            cases.append(dict(name=protocol+"-"+name, protocol=protocol, command=command,
                              accepted=accepted, text=body))
    return cases


# Un runtime PowerShell explicite complète AWK sans exécuter le lecteur matériel.
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--powershell")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    cases = json.loads(Path(__file__).with_name("profile_cases.json").read_text()) + generated_cases()
    manifest = []
    for index, case in enumerate(cases):
        path = (args.output / f"{index:03}-{case['name']}.txt").resolve()
        path.write_bytes(case["text"].encode("utf-8"))
        result = subprocess.run(["awk", "-v", "command="+case["command"], "-f",
                                 str(REPOSITORY/VALIDATORS[case["protocol"]]), str(path)],
                                capture_output=True, check=False)
        if (result.returncode == 0) != case["accepted"]:
            raise RuntimeError(f"Validation AWK inattendue : {case['name']} (code {result.returncode})")
        manifest.append({**case, "path": str(path)})
    print(f"AWK : {len(cases)} cas de protocole passent.")
    powershell = args.powershell or shutil.which("pwsh")
    if powershell:
        manifest_path = (args.output/"cases.json").resolve()
        manifest_path.write_text(json.dumps(manifest))
        subprocess.run([powershell, "-NoProfile", "-File", str(Path(__file__).with_name("protocol_test.ps1")),
                        "-Manifest", str(manifest_path)], check=True)
    else:
        print("PowerShell non testé : relancer avec --powershell <chemin-vers-pwsh>.")


# Les seuls fichiers produits sont des cas synthétiques dans le dossier demandé.
if __name__ == "__main__":
    main()

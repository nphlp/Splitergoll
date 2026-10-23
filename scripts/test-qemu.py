#!/usr/bin/env python3
"""Attend le verdict Ztest et arrête seulement le groupe QEMU créé par ce test."""

import argparse
import os
import selectors
import signal
import subprocess
import sys
import time


# Le délai et la commande sont explicites, sans shell ni choix de cible implicite.
def arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--timeout", type=int, required=True)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    if args.command[:1] == ["--"]:
        args.command.pop(0)
    if not args.command or not 1 <= args.timeout <= 120:
        parser.error("Indique une commande et un délai compris entre 1 et 120 secondes.")
    return args


# Le groupe appartient à cette invocation et ne contient aucun service préexistant.
def stop(process):
    try:
        os.killpg(process.pid, signal.SIGTERM)
    except ProcessLookupError:
        pass
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        process.wait(timeout=3)


# Cortex-M3 reste inactif après Ztest : le verdict exact déclenche son arrêt propre au test.
def run(args):
    process = subprocess.Popen(args.command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               start_new_session=True)
    passed = False
    buffer = b""
    deadline = time.monotonic() + args.timeout
    try:
        with selectors.DefaultSelector() as reader:
            reader.register(process.stdout, selectors.EVENT_READ)
            while time.monotonic() < deadline:
                if not reader.select(timeout=min(0.2, max(0, deadline-time.monotonic()))):
                    if process.poll() is not None:
                        break
                    continue
                chunk = os.read(process.stdout.fileno(), 16384)
                if not chunk:
                    break
                sys.stdout.buffer.write(chunk)
                sys.stdout.buffer.flush()
                buffer += chunk
                if len(buffer) > 131072:
                    raise RuntimeError("Sortie QEMU sans lignes, résultat non interprétable.")
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    if line.strip() == b"PROJECT EXECUTION FAILED":
                        return 1
                    if line.strip() == b"PROJECT EXECUTION SUCCESSFUL":
                        passed = True
                        return 0
    finally:
        stop(process)
        process.stdout.close()
    if not passed:
        print("Aucun verdict Ztest positif complet avant la fin du test.", file=sys.stderr)
    return 1


# N’ouvre aucun périphérique et ne touche à aucun autre groupe de processus.
if __name__ == "__main__":
    sys.exit(run(arguments()))

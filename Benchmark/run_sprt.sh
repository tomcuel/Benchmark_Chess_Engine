#!/bin/sh
# Compile le moteur, vérifie la génération de coups, garde le binaire sous un nom, puis le compare à une référence par SPRT.
# usage : ./run_sprt.sh <nom de la nouvelle version> <nom de la référence> [parties] [cadence]
# exemple : ./run_sprt.sh hashmove v1.1 400 8+0.08
set -e
cd "$(dirname "$0")"
NEW="$1"; BASE="$2"; GAMES="${3:-400}"; TC="${4:-8+0.08}"
if [ -z "$NEW" ] || [ -z "$BASE" ]; then echo "usage : $0 <nouvelle version> <référence> [parties] [cadence]"; exit 2; fi
make -C ../Chess_Engine/UCI > /dev/null
../Chess_Engine/UCI/Perft.x | tail -1
../Chess_Engine/UCI/UCI.x bench 6 | tail -1
if [ ! -d "engines/$NEW" ]; then python3 benchmark.py --snapshot "$NEW"; fi
CONFIG="configs/experiments/sprt_${NEW}_vs_${BASE}.json"
cat > "$CONFIG" <<JSON
{
  "name": "sprt_${NEW}_vs_${BASE}",
  "mode": "sprt",
  "games": $GAMES,
  "time_control": "$TC",
  "concurrency": 4,
  "bench_depth": 6,
  "engine": {"name": "CleanedChess-$NEW", "version": "$NEW", "cmd": "engines/$NEW/UCI.x", "options": {"Hash": 64}},
  "baseline": {"name": "CleanedChess-$BASE", "version": "$BASE", "cmd": "engines/$BASE/UCI.x", "options": {"Hash": 64}},
  "sprt": {"elo0": 0, "elo1": 10, "alpha": 0.05, "beta": 0.05},
  "openings": {"plies": 10}
}
JSON
python3 benchmark.py --config "$CONFIG"

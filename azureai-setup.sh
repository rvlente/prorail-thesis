#!/bin/bash
sudo apt-get update

# pull submodules
git submodule init
git submodule update

# deactivate conda envs
for i in $(seq ${CONDA_SHLVL}); do
    conda deactivate
done

# setup venv
sudo apt-get install python3.8-venv
python -m venv .venv
source .venv/bin/activate

# install deps
pip install requests duckdb polars pyarrow

# run dataset script
mkdir -r data/nyc-taxi
python tools/generate-nyctaxi.py
mv output.bin data/nyc-taxi/nyc-taxi.bin

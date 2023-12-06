#!/bin/bash
sudo apt-get update
git config --global user.name "Robert van Lente"
git config --global user.email "robertvlente@gmail.com"

# pull submodules
git submodule init
git submodule update

# deactivate conda envs
for i in $(seq ${CONDA_SHLVL}); do
    conda deactivate
done

conda remove --name azureml_py38 --all
conda remove --name azureml_py38_PT_TF --all
conda remove --name azureml_py310_sdkv2 --all

# setup venv
sudo apt-get install python3.8-venv
/usr/bin/python3 -m venv .venv

# install deps
.venv/bin/pip install -r tools/requirements.txt

# # run dataset script
mkdir -p data/nyc-taxi
.venv/bin/python tools/generate-nyctaxi.py
mv output.bin data/nyc-taxi/nyc-taxi-30m.bin

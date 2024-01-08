#!/bin/bash
sudo apt-get update
git config --global user.name "Robert van Lente"
git config --global user.email "robertvlente@gmail.com"

# pull submodules
git submodule init
git submodule update

# deactivate conda envs
conda init bash

for i in $(seq ${CONDA_SHLVL}); do
    conda deactivate
done

conda remove --name azureml_py38 -y --all
conda remove --name azureml_py38_PT_TF -y --all
conda remove --name azureml_py310_sdkv2 -y --all

# setup venv
sudo apt-get install python3.8-venv
/usr/bin/python3 -m venv .venv

# install deps
.venv/bin/pip install -r tools/requirements.txt

# set env vars
export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

#!/bin/bash

# Encontra e remove todos os arquivos .exe no diretório atual e seus subdiretórios
find . -type f -name "*.exe" -exec rm -f {} \;

echo "Todos os arquivos .exe foram excluídos do diretório atual e de seus subdiretórios."

#!/bin/bash
# Télécharger les headers stb_image
wget -q https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -O stb_image.h
wget -q https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h -O stb_image_write.h
echo "Headers STB téléchargés"

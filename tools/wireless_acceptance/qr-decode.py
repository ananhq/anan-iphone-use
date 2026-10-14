#!/usr/bin/env python3
"""Predefined independent decoding scenario; synthetic image only, no camera."""
import sys
from PIL import Image
import zxingcpp
result=zxingcpp.read_barcode(Image.open(sys.argv[1]))
assert result and result.text=="WIFI:T:WPA;S:StopWatch-setup;P:DEMO23456789;;",result
print("Independent ZXing decoding recovered the exact synthetic Wi-Fi payload.")

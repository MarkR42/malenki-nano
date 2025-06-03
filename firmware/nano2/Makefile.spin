
# Makefile for Malenki-Spin
#
MCU=attiny1616
MCU_PYMCUPROG=attiny1616
# 
CFLAGS=-DENABLE_DIAG=1 -DPRODUCT_IS_PLUS=1 -DPRODUCT_IS_SPIN=1
PRODUCT_MODEL=spin

include common.mk

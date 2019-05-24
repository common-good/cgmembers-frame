#!/bin/bash

grep -rEn '[^a-zA-Z_](debug|print_r|die|echo)\(' cgmembers/rcredits | grep -v 'vendor/' | grep -v 'js:' | grep -v -E ': *(///|/\*\*/).*(debug|print_r|die|echo)'

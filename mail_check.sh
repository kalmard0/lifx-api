#!/bin/bash
while(true)
do
    ./lifx-api 1 1 1 1
    python imapidle.py
    ./lifx-api 0 0 1 1
    sleep 1
done
#!/bin/bash


# matches all (non-multiline) function prototypes
gawk "/^([a-z0-9\_\* ])*[a-z0-9\_\*]+\([a-z0-9\_\*]+ ([a-z0-9\_\* ])+(\,[a-z0-9\_\* ]+)*\)(;?\s*{?)$/" $1



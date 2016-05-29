#!/bin/sh
# Filter doxygen output for acceptable warnings

stdin=$(cat)
echo "$stdin" | grep -v "has become obsolete." \
              | grep -v "To avoid this warning please remove this line from your configuration file"

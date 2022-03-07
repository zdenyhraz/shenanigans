#!/bin/bash

#sudo apt install rename
find src -type f -name '*.h' -print0 | xargs -0 rename 's/.h$/.hpp/'

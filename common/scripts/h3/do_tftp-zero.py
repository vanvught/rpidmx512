#!/usr/bin/env python3
import sys
from do_tftp import main
raise SystemExit(main([sys.argv[0], *sys.argv[1:], "--board", "zero"]))


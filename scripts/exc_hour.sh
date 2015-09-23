#!/bin/bash
while [ true ]; do
sh /usr/local/vidy/scripts/clear.sh
sh /usr/local/vidy/scripts/run.sh
sh /usr/local/vidy/scripts/run_db.sh
/bin/sleep 3600
done

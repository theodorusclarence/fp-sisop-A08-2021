#!/bin/bash
# CRON 0 */1 * * *

# ./databasedump -u jack -p jack123 database1 > database1.backup

zip -r $(date +%m%d%Y) database1.backup

echo > database1.backup

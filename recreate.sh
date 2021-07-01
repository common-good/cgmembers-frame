#! /bin/bash
mysql "$@" <db/import-startup.sql && \
echo "Imported startup database" && \
vendor/robmorgan/phinx/bin/phinx migrate -e development -c config/phinx.json

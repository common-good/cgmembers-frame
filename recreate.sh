#! /bin/bash
sh ./importdb.sh startup && echo "Imported startup database" && \
sh ./migrate.sh $1 $2 $3 $4

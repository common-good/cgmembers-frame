#!/bin/bash
./jr-importdb.sh startup
./jr-migrate.sh $1 $2 $3 $4


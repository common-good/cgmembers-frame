#!/usr/bin/env bash

composer install && mysql -h db -u root < db/import-startup.sql && vendor/bin/phinx migrate -c config/phinx.json

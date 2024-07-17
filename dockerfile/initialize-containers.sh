#!/bin/bash
rm -rf ../cgmembers/gherkin
cp ../dockerfile/setup/config.json ../config/
cp ../dockerfile/setup/phinx.json ../config/
docker compose up
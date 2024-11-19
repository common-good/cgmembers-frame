#!/bin/bash
rm -rf ../cgmembers/gherkin
cp ../dockerfile/setup/config.json ../config/
cp ../dockerfile/setup/phinx.json ../config/
mkdir -p ../cgPhotoTemp/photoids
DOCKER_BUILDKIT=1 docker compose up
#!/bin/bash
rm -rf ../cgmembers/gherkin
cp ../dockerfile/setup/config.json ../config/
cp ../dockerfile/setup/phinx.json ../config/
mkdir -p ../cgPhotoTemp/photoids
export DOCKER_BUILDKIT=1
docker compose up
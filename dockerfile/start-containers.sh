#!/bin/bash
export DOCKER_BUILDKIT=1
docker container start cgmembers-mysql && docker container start cgmembers-apache

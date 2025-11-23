#!/usr/bin/env bash

./scripts/build_image.sh

podman push localhost/tempest:latest ryankelley20/tempest:latest

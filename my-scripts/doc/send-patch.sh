#!/bin/bash
set -x
git send-email --from robert.berger@ReliableEmbeddedSystems.com --cc richard.leitner@skidata.com --to openembedded-devel@lists.openembedded.org $1
set +x

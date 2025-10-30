#!/usr/bin/env zsh
set -euo pipefail
bundle exec jekyll build
rsync -avz --delete _site/ dennisppaul.de@ssh.strato.de:/mnt/web016/c1/09/5254209/htdocs/umfeld
